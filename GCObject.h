//
// Created by v4kst1z.
//
#pragma once
#include "TinyGC.h"
#include "Common.h"

class GarbageCollectedBase {
public:
	GarbageCollectedBase(int obj_size, bool mark = false) :
		mark_(mark),
		obj_size_(obj_size)
	{}
	virtual void Trace(Visitor* visitor) const {}

	int GetObjSize() { return obj_size_; }

private:
	bool mark_;
	GarbageCollectedBase* next_;
	int obj_size_;
	friend class Visitor;
	friend class TinyGC;
};

template <typename T>
class GarbageCollected : public GarbageCollectedBase {
public:
	GarbageCollected() : GarbageCollectedBase(sizeof(T), false) {}
	virtual void Trace(Visitor* visitor) const {}

	DISALLOW_COPY_AND_ASSIGN(GarbageCollected);

};

template <typename T>
class Member {
public:
	explicit Member(T* raw): raw_(raw) {}
	T* GetRaw() const { return raw_; }
	
	DISALLOW_COPY_AND_ASSIGN(Member);
private:
	T* raw_;
};