//
// Created by v4kst1z.
//
#pragma once
#include "TinyGC.h"
#include <thread>
#include <future>
#include <functional>

class WorkThread {
 public:
  WorkThread() = default;

  template<typename Func, typename... Args>
  auto Run(Func &&f, Args &&...args) -> std::future<decltype(std::forward<Func>(f)(std::forward<Args>(args)...))> {
    using ret_type = decltype(std::forward<Func>(f)(std::forward<Args>(args)...));
    std::function<ret_type()> func = std::bind(std::forward<Func>(f), std::forward<Args>(args)...);
    auto *tsk = new std::packaged_task<ret_type()>(func);
    std::future<ret_type> ret_future = tsk->get_future();
    thread_ = std::move(std::thread([tsk]() {
      TinyGC::GetInstance().AttachCurrentThread();
      (*tsk)();
      delete tsk;
      TinyGC::GetInstance().DetachCurrentThread();
    }));
    return ret_future;
  }

  WorkThread(WorkThread &&other) noexcept {
    thread_ = std::move(other.thread_);
  }

  WorkThread &operator=(WorkThread &&other) noexcept {
    thread_ = std::move(other.thread_);
    return *this;
  }

  std::thread &operator*() {
    return thread_;
  }

  std::thread const &operator*() const {
    return thread_;
  }

  std::thread *operator->() {
    return &operator*();
  }

  std::thread const *operator->() const {
    return &operator*();
  }

  auto get_id() const {
    return thread_.get_id();
  }

  auto join() {
    if (thread_.joinable())
      thread_.join();
  }

  ~WorkThread() {
    if (thread_.joinable())
      join();
  }

 private:
  std::thread thread_;
};