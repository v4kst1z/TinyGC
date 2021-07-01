//
// Created by v4kst1z.
//
// 
// TinyGC.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once

#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <mutex>

#if defined(_WIN32)
#include <stddef.h>
#include <windows.h>
#include <winnt.h>
#include <intrin.h>
#elif defined(__GLIBC__)
extern "C" void* __libc_stack_end; 
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#endif

#include <vector>
#include "Common.h"

// Forward declarations
class Visitor;
class GarbageCollectedBase;
class ThreadState;

class TinyGC {
public:
	static TinyGC& GetInstance() {
		static TinyGC tg;
		return tg;
	}

	~TinyGC();

	enum class GCPhase {
		kNone,
		kMarking,
		kSweeping,
	};

	void SetGCPhase(GCPhase);

	void AttachCurrentThread();

	void AttachMainThread();

	void DetachMainThread();

	void DetachCurrentThread();

	void MarkSweepGC();

	template <typename T, typename ...Args>
	T* MakeGarbageCollected(Args&& ...args);

	DISALLOW_COPY_AND_ASSIGN(TinyGC);
private:
	class ThreadState {
	public:
		ThreadState(bool main = false) : stack_end_(nullptr),
			thread_id_(std::this_thread::get_id()),
			main_thread_ (main){
			stack_start_ = GetStackStart(main_thread_);
		}
		void GetCurrentStackPosition();

		void* GetStackStartAddr() { return  stack_start_; }

		void* GetStackEndAddr() { return stack_end_; }
		std::thread::id GetThreadId() { return thread_id_;  }
	private:
		void* GetStackStart(bool main);

		void* GetStackEnd();

		void* stack_start_;
		void* stack_end_;
		bool main_thread_;
		std::thread::id thread_id_;
	};

	TinyGC();

	void Mark();

	void Sweep();

	void CheckThreshold();

	void SwitchTrace(GarbageCollectedBase* ptr);

	std::unordered_set<GarbageCollectedBase*> objs_addr_;
	std::unordered_map<std::thread::id, ThreadState*> thread_to_stack_;
	std::mutex ts_mxt_;
	std::mutex alloc_mxt_;
	Visitor* visitor_;
	GCPhase gc_phase_;
	int bytes_allocated_;
	int size_of_objects_;
	int gc_count_threshold_;
	int gc_bytes_threshold_;
};

template<typename T, typename ...Args>
T* TinyGC::MakeGarbageCollected(Args && ...args) {
	std::unique_lock<std::mutex> lck(alloc_mxt_);
	CheckThreshold();
	T* new_obj = ::new(malloc(sizeof(T))) T(std::forward<Args>(args)...);
	LOG("Object is allocated at " << new_obj);
	objs_addr_.insert(new_obj);
	this->bytes_allocated_ += new_obj->GetObjSize();
	this->size_of_objects_++;
	return new_obj;
}

static TinyGC& tg = TinyGC::GetInstance();
#define MakeGarbageCollected tg.MakeGarbageCollected 

