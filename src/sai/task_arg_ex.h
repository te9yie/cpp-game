#pragma once
#include "task_arg.h"

namespace sai {

// MutexRes.
template <typename T>
struct MutexRes {
  T* p = nullptr;
  T* operator->() const { return p; }
  T& operator*() const { return *p; }
};

template <typename T>
struct task_arg_traits<MutexRes<T>> {
  static void set_permission(TaskArgsPermission* p) {
    p->writes.emplace(task_arg_type_index<T>::index());
  }
  static MutexRes<T> from_context(const TaskContext& ctx) {
    return MutexRes<T>{ctx.get<T>()};
  }
};

// ReadRes.
template <typename T>
struct ReadRes {
  T* p = nullptr;
  T* operator->() const { return p; }
  T& operator*() const { return *p; }
};

template <typename T>
struct task_arg_traits<ReadRes<T>> {
  static void set_permission(TaskArgsPermission* p) {
    p->reads.emplace(task_arg_type_index<T>::index());
  }
  static ReadRes<T> from_context(const TaskContext& ctx) {
    return ReadRes<T>{ctx.get<T>()};
  }
};

// WriteRes.
template <typename T>
struct WriteRes {
  T* p = nullptr;
  T* operator->() const { return p; }
  T& operator*() const { return *p; }
};

template <typename T>
struct task_arg_traits<WriteRes<T>> {
  static void set_permission(TaskArgsPermission* p) {
    p->writes.emplace(task_arg_type_index<T>::index());
  }
  static WriteRes<T> from_context(const TaskContext& ctx) {
    return WriteRes<T>{ctx.get<T>()};
  }
};

}  // namespace sai
