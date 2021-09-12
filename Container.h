//
// Created by v4kst1z.
//
#pragma once
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "GCObject.h"

class GarbageCollectedBase;

template <typename T>
class Member;

class Visitor;

template <typename T>
class HeapVector final : public GarbageCollected<HeapVector<T>> {
 public:
  using Container = std::vector<Member<T> *>;
  HeapVector() = default;

  void Trace(Visitor *visitor) const override {
    for (auto &elem : vec_) visitor->Trace(*elem);
  }

  GarbageCollectedBase *operator[](std::size_t nIndex) {
    if (size() == 0) return nullptr;
    if (nIndex > vec_.size()) {
      LOG("Index too larger~");
      return vec_[vec_.size() - 1]->GetRaw();
    }
    return vec_[nIndex]->GetRaw();
  }

  void erase(T *t) {
    for (auto it = vec_.begin(); it != vec_.end();) {
      if ((*it)->GetRaw() == t) {
        it = vec_.erase(it);
      } else {
        ++it;
      }
    }
  }

  size_t size() { return vec_.size(); }

  void clear() { vec_.clear(); }

  void push_back(T *t) { vec_.push_back(new Member<T>(t)); }

 private:
  Container vec_;
};

template <typename T>
class HeapDeque final : public GarbageCollected<HeapDeque<T>> {
 public:
  using Container = std::deque<Member<T> *>;
  HeapDeque() = default;

  void Trace(Visitor *visitor) const override {
    for (size_t id = 0; id < dque_.size(); id++) visitor->Trace(*dque_[id]);
  }

  GarbageCollectedBase *operator[](std::size_t nIndex) {
    if (size() == 0) return nullptr;
    if (nIndex > dque_.size()) {
      LOG("Index too larger~");
      return dque_[dque_.size() - 1]->GetRaw();
    }
    return dque_[nIndex]->GetRaw();
  }

  GarbageCollectedBase *front() {
    if (size()) return dque_.front()->GetRaw();
  }

  GarbageCollectedBase *back() {
    if (size()) return dque_.back()->GetRaw();
  }

  void pop_back() {
    if (size()) dque_.pop_back();
  }

  void pop_front() {
    if (size()) return dque_.pop_front();
  }

  size_t size() { return dque_.size(); }

  void clear() { dque_.clear(); }

  void push_back(T *t) { dque_.push_back(new Member<T>(t)); }

  void push_front(T *t) { dque_.push_back(new Member<T>(t)); }

 private:
  Container dque_;
};

template <typename T>
class HeapSet final : public GarbageCollected<HeapSet<T>> {
 public:
  using Container = std::unordered_set<Member<T> *>;
  HeapSet() = default;

  void Trace(Visitor *visitor) const override {
    for (auto &elem : set_) visitor->Trace(*elem);
  }

  void erase(T *t) {
    for (auto it = set_.begin(); it != set_.end();) {
      if ((*it)->GetRaw() == t) {
        it = set_.erase(it);
      } else {
        ++it;
      }
    }
  }

  size_t count(T *t) { return set_.count(new Member<T>(t)); }

  size_t size() { return set_.size(); }

  void clear() { set_.clear(); }

  void insert(T *t) { set_.insert(new Member<T>(t)); }

 private:
  Container set_;
};

template <typename Key, typename Value, typename KeyBool = std::true_type,
          typename ValueBool = std::true_type>
class HeapMap final : public GarbageCollected<HeapMap<Key, Value>> {
 public:
  using Container = std::unordered_map<Member<Key> *, Member<Value> *>;
  HeapMap() = default;

  void Trace(Visitor *visitor) const override {
    for (auto &pr : map_) {
      visitor->Trace(*pr.first);
      visitor->Trace(*pr.second);
    }
  }

  void insert(Key *k, Value *val) {
    map_.insert({new Member<Key>(k), new Member<Value>(val)});
  }

  void erase(Key *k) {
    for (auto it = map_.begin(); it != map_.end();) {
      if (it->first->GetRaw() == k) {
        it = map_.erase(it);
      } else {
        ++it;
      }
    }
  }

  GarbageCollectedBase *at(Key *k) {
    for (auto it = map_.begin(); it != map_.end(); it++) {
      if (it->first->GetRaw() == k) {
        return it->second->GetRaw();
      }
    }
    return nullptr;
  }

  size_t size() { return map_.size(); }

  void clear() { map_.clear(); }

 private:
  Container map_;
};

template <typename Key, typename Value>
class HeapMap<Key, Value,
              std::integral_constant<
                  bool, !std::is_base_of<GarbageCollectedBase, Key>::value>,
              std::integral_constant<
                  bool, !std::is_base_of<GarbageCollectedBase, Value>::value>>
    final : public GarbageCollected<HeapMap<Key, Value>> {
 public:
  using Container = std::unordered_map<Key, Value>;
  HeapMap() = default;

  void insert(Key k, Value val) { map_.insert({k, val}); }

  void insert(Key &k, Value &val) { map_.insert({k, val}); }

  void erase(Key &k) { map_.erase(k); }

  Value at(Key &k) { return map_.at(k); }

  size_t size() { return map_.size(); }

  void clear() { map_.clear(); }

 private:
  Container map_;
};

template <typename Key, typename Value>
class HeapMap<Key, Value,
              std::integral_constant<
                  bool, std::is_base_of<GarbageCollectedBase, Key>::value>,
              std::integral_constant<
                  bool, !std::is_base_of<GarbageCollectedBase, Value>::value>>
    final : public GarbageCollected<HeapMap<Key, Value>> {
 public:
  using Container = std::unordered_map<Member<Key> *, Value>;
  HeapMap() = default;

  void Trace(Visitor *visitor) const override {
    for (auto &pr : map_) {
      visitor->Trace(*pr.first);
    }
  }

  void insert(Key *k, const Value &val) {
    map_.insert({new Member<Key>(k), val});
  }

  void erase(Key *k) {
    for (auto it = map_.begin(); it != map_.end();) {
      if (it->first->GetRaw() == k) {
        it = map_.erase(it);
      } else {
        ++it;
      }
    }
  }

  Value at(Key *k) {
    for (auto it = map_.begin(); it != map_.end(); it++) {
      if (it->first->GetRaw() == k) {
        return it->second;
      }
    }
  }

  size_t size() { return map_.size(); }

  void clear() { map_.clear(); }

 private:
  Container map_;
};

template <typename Key, typename Value>
class HeapMap<Key, Value,
              std::integral_constant<
                  bool, !std::is_base_of<GarbageCollectedBase, Key>::value>,
              std::integral_constant<
                  bool, std::is_base_of<GarbageCollectedBase, Value>::value>>
    final : public GarbageCollected<HeapMap<Key, Value>> {
 public:
  using Container = std::unordered_map<Key, Member<Value> *>;
  HeapMap() = default;

  void Trace(Visitor *visitor) const override {
    for (auto &pr : map_) {
      visitor->Trace(*pr.second);
    }
  }

  void insert(const Key &k, Value *val) {
    map_.insert({k, new Member<Value>(val)});
  }

  void erase(Key &k) { map_.erase(k); }

  GarbageCollectedBase *at(Key &k) { return map_.at(k)->GetRaw(); }

  size_t size() { return map_.size(); }

  void clear() { map_.clear(); }

 private:
  Container map_;
};
