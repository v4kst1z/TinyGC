//
// Created by v4kst1z.
//
#pragma once
#include "Common.h"

class Visitor;

class GarbageCollectedBase {
 public:
  explicit GarbageCollectedBase(int obj_size, bool mark = false)
      : mark_(mark), obj_size_(obj_size) {}
  virtual void Trace(Visitor *visitor) const {}

  int GetObjSize() { return obj_size_; }
  virtual ~GarbageCollectedBase() = default;

 private:
  bool mark_;
  int obj_size_;
  friend class Visitor;
  friend class TinyGC;
};

template <typename T>
class GarbageCollected : public GarbageCollectedBase {
 public:
  GarbageCollected() : GarbageCollectedBase(sizeof(T), false) {}
  void Trace(Visitor *visitor) const override {}

  DISALLOW_COPY_AND_ASSIGN(GarbageCollected);
};

template <typename T>
class Member {
 public:
  explicit Member(T *raw) : raw_(raw) {}
  T *GetRaw() const { return raw_; }

  DISALLOW_COPY_AND_ASSIGN(Member);

 private:
  T *raw_;
};