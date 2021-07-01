//
// Created by v4kst1z.
//

#include <cassert>

#include "Visitor.h"
#include "TinyGC.h"

TinyGC::TinyGC() :
    visitor_(new Visitor()),
    gc_phase_(GCPhase::kNone),
    bytes_allocated_(0),
    size_of_objects_(0),
    gc_count_threshold_(120),
    gc_bytes_threshold_(0x1000) {}


void TinyGC::Mark() {
    std::unique_lock<std::mutex> lck(ts_mxt_);
    LOG("\nMark Start~");
    SetGCPhase(GCPhase::kMarking);
    visitor_->SetMark(true);

    for (auto& ts_ : thread_to_stack_) {
        ThreadState* ts = ts_.second;
        ts->GetCurrentStackPosition();
        intptr_t** start = reinterpret_cast<intptr_t**>(ts->GetStackEndAddr());
        intptr_t** end = reinterpret_cast<intptr_t**>(ts->GetStackStartAddr());
        for (; start < end; start++) {
            if (*start > (intptr_t*)0x100000) {
                GarbageCollectedBase* ptr = reinterpret_cast<GarbageCollectedBase*>(*start);
                if (objs_addr_.find(ptr) != objs_addr_.end()) {
                    visitor_->ObjTrace((GarbageCollectedBase*)ptr);
                    LOG("find object at " << std::hex << ptr);
                }
            }
        }

    }
}


void TinyGC::Sweep() {
    std::unique_lock<std::mutex> lck(ts_mxt_);
    LOG("\nSweep Start~");
    SetGCPhase(GCPhase::kSweeping);
    visitor_->SetMark(false);

    std::unordered_set<GarbageCollectedBase*> roots = objs_addr_;

    for(auto &root: roots) {
        if(root->mark_ != true) {
            objs_addr_.erase(root);
            bytes_allocated_ -= root->obj_size_;
            size_of_objects_--;
            LOG("Object " << root << " is deleted!");
            root->~GarbageCollectedBase();
            delete root;
        }
    }
    SetGCPhase(GCPhase::kNone);
}

TinyGC::~TinyGC() {
    delete this->visitor_;
}

void TinyGC::MarkSweepGC() {
    LOG("Mark Sweep GC Start~");
    Mark();
    Sweep();
}


void TinyGC::SetGCPhase(GCPhase gc_phase) {
    switch (gc_phase) {
        case GCPhase::kNone:
            assert(gc_phase_ == GCPhase::kSweeping);
            break;
        case GCPhase::kMarking:
            assert(gc_phase_ == GCPhase::kNone);
            break;
        case GCPhase::kSweeping:
            assert(gc_phase_ == GCPhase::kMarking);
            break;
    }
    gc_phase_ = gc_phase;
}

void TinyGC::AttachCurrentThread() {
    std::unique_lock<std::mutex> lck(ts_mxt_);
    auto thread_state = new ThreadState();
    thread_to_stack_.insert( \
        std::pair<std::thread::id, ThreadState*>(thread_state->GetThreadId(), thread_state) \
    );
    //LOG("thread id is " << thread_state->GetThreadId());
    //LOG("stack start address " << thread_state->GetStackStartAddr());
    //LOG("stack end address " << thread_state->GetStackEndAddr());
}

void TinyGC::AttachMainThread() {
    std::unique_lock<std::mutex> lck(ts_mxt_);
    auto thread_state = new ThreadState(true);
    thread_to_stack_.insert(\
        std::pair<std::thread::id, ThreadState*>(thread_state->GetThreadId(), thread_state) \
    );
    //LOG("main thread~");
    //LOG("stack start address " << thread_state->GetStackStartAddr());
    //LOG("stack end address " << thread_state->GetStackEndAddr());
}

void TinyGC::DetachMainThread() {
    LOG("Detach Main Thread");
    std::unique_lock<std::mutex> lck(ts_mxt_);
    assert(thread_to_stack_.size() == 1);
    ThreadState* ts = thread_to_stack_[std::this_thread::get_id()];
    thread_to_stack_.erase(std::this_thread::get_id());
    delete ts;
    lck.unlock();
    MarkSweepGC();
}

void TinyGC::DetachCurrentThread() {
    std::unique_lock<std::mutex> lck(ts_mxt_);
    ThreadState* ts = thread_to_stack_[std::this_thread::get_id()];
    thread_to_stack_.erase(std::this_thread::get_id());
    delete ts;
}


void TinyGC::CheckThreshold() {
    if (size_of_objects_ >= gc_count_threshold_) {
        LOG("Object amout > gc amount threshold~");
        MarkSweepGC();
    }
    if (size_of_objects_ >= gc_count_threshold_) gc_count_threshold_ *= 2;

    if (bytes_allocated_ >= gc_bytes_threshold_) {
        LOG("bytes allocated > gc bytes threshold~");
        MarkSweepGC();
    }
    if (bytes_allocated_ >= gc_bytes_threshold_) gc_bytes_threshold_ *= 2;
}


void TinyGC::ThreadState::GetCurrentStackPosition() {
    stack_end_ = GetStackEnd();
}

void* TinyGC::ThreadState::GetStackStart(bool main) {
#if defined(__GLIBC__)
    if(main)
        return __libc_stack_end;
    else {
        pthread_attr_t attr;
        int ret = pthread_attr_init(&attr);
        assert(!ret);
        int error = pthread_getattr_np(pthread_self(), &attr);
        if (!error) {
            void* base;
            size_t size;
            error = pthread_attr_getstack(&attr, &base, &size);
            assert(!error);
            pthread_attr_destroy(&attr);
            return reinterpret_cast<uint8_t*>(base) + size;
        } else {
            perror("pthread_getattr_np");
            exit(EXIT_FAILURE);
        }
    }
#elif defined(_M_X64) || defined(__x86_64__)
    return reinterpret_cast<void*>(
        reinterpret_cast<NT_TIB64*>(NtCurrentTeb())->StackBase);
#elif defined(_M_IX86) || defined(__i386__)
    return reinterpret_cast<void*>(
        reinterpret_cast<NT_TIB*>(NtCurrentTeb())->StackBase);
#endif
}

void* TinyGC::ThreadState::GetStackEnd() {
#if defined(_MSC_VER)
    return reinterpret_cast<void*>(_AddressOfReturnAddress());
#else
    return reinterpret_cast<void*>(__builtin_frame_address(0));
#endif
}
