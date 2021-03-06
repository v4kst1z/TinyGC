//
// Created by v4kst1z.
//
#pragma once
#include <type_traits>

#include "GCObject.h"

class GarbageCollectedBase;

// Forward declarations
template <typename T>
class Member;

class Visitor {
 public:
  Visitor() : mark_(false){};

  template <typename T>
  void Trace(const Member<T> &mem) {
    if (mem.GetRaw() == nullptr) return;
    if (std::is_polymorphic<T>::value) {
      LOG("find object at " << mem.GetRaw());
      ObjTrace(mem.GetRaw());
    }
  }

  void SetMark(bool val) { mark_ = val; }

  void ObjTrace(GarbageCollectedBase *gc_obj) {
    if (gc_obj == nullptr) return;
    if (gc_obj->mark_ == this->mark_) return;
    gc_obj->mark_ = this->mark_;
    gc_obj->Trace(this);
  }

 private:
  bool mark_;
};
