#pragma once
#include <cstddef>
#include <set>

#include "t9/dynamic_tuple.h"
#include "t9/type_list.h"

namespace sai {

// TaskContext.
using TaskContext = t9::DynamicTuple;

namespace task_arg_ {

struct task_arg_type_index_ {
  static inline std::size_t index = 0;
};

}  // namespace task_arg_

// task_arg_type_index.
template <typename T>
struct task_arg_type_index {
  static std::size_t index() {
    static std::size_t i = task_arg_::task_arg_type_index_::index++;
    return i;
  }
};

// TaskArgsPermission.
struct TaskArgsPermission {
  std::set<std::size_t> writes;
  std::set<std::size_t> reads;

  bool is_conflict_write(std::size_t i) const {
    if (writes.find(i) != writes.end()) return true;
    if (reads.find(i) != reads.end()) return true;
    return false;
  }
  bool is_conflict_read(std::size_t i) const {
    if (writes.find(i) != writes.end()) return true;
    return false;
  }
};

// task_arg_traits.
template <typename T>
struct task_arg_traits;

template <typename T>
struct task_arg_traits<T*> {
  static void set_permission(TaskArgsPermission* p) {
    p->writes.emplace(task_arg_type_index<T>::index());
  }
  static T* from_context(const TaskContext& ctx) { return ctx.get<T>(); }
};

template <typename T>
struct task_arg_traits<const T*> {
  static void set_permission(TaskArgsPermission* p) {
    p->reads.emplace(task_arg_type_index<T>::index());
  }
  static const T* from_context(const TaskContext& ctx) { return ctx.get<T>(); }
};

template <typename T>
struct task_arg_traits<T&> {
  static void set_permission(TaskArgsPermission* p) {
    p->writes.emplace(task_arg_type_index<T>::index());
  }
  static T* from_context(const TaskContext& ctx) { return ctx.get<T>(); }
};

template <typename T>
struct task_arg_traits<const T&> {
  static void set_permission(TaskArgsPermission* p) {
    p->reads.emplace(task_arg_type_index<T>::index());
  }
  static const T* from_context(const TaskContext& ctx) { return ctx.get<T>(); }
};

// make_task_args_permission.
namespace task_arg_ {

inline void set_task_args_permission_(TaskArgsPermission*, t9::type_list<>) {}

template <typename T, typename... Ts>
inline void set_task_args_permission_(TaskArgsPermission* p,
                                      t9::type_list<T, Ts...>) {
  task_arg_traits<T>::set_permission(p);
  set_task_args_permission_(p, t9::type_list<Ts...>{});
}

}  // namespace task_arg_

template <typename... Ts>
inline TaskArgsPermission make_task_args_permission() {
  TaskArgsPermission p;
  task_arg_::set_task_args_permission_(&p, t9::type_list<Ts...>{});
  return p;
}

}  // namespace sai
