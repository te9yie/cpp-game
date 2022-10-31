#pragma once
#include <cstddef>
#include <set>

#include "context.h"
#include "t9/type_list.h"

namespace sai::task {

// arg_type_index.
namespace args_ {

struct arg_type_index {
  static inline std::size_t index = 0;
};

}  // namespace args_

template <typename T>
struct arg_type_index {
  static std::size_t index() {
    static std::size_t i = args_::arg_type_index::index++;
    return i;
  }
};

// ArgsTypeBits.
struct ArgsTypeBits {
  std::set<std::size_t> writes;
  std::set<std::size_t> reads;

  template <typename T>
  void set_write() {
    writes.emplace(arg_type_index<T>::index());
  }
  template <typename T>
  void set_read() {
    reads.emplace(arg_type_index<T>::index());
  }

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

// TaskWork.
using TaskWork = ContextBase<struct TaskTag>;

// arg_traits.
template <typename T>
struct arg_traits {
  static void set_type_bits(ArgsTypeBits* bits) { bits->set_read<T>(); }
  static T to(const Context* ctx, TaskWork*) { return *ctx->get<T>(); }
};
template <typename T>
struct arg_traits<T*> {
  static void set_type_bits(ArgsTypeBits* bits) { bits->set_write<T>(); }
  static T* to(const Context* ctx, TaskWork*) { return ctx->get<T>(); }
};
template <typename T>
struct arg_traits<const T*> {
  static void set_type_bits(ArgsTypeBits* bits) { bits->set_read<T>(); }
  static const T* to(const Context* ctx, TaskWork*) { return ctx->get<T>(); }
};
template <typename T>
struct arg_traits<T&> {
  static void set_type_bits(ArgsTypeBits* bits) { bits->set_write<T>(); }
  static T& to(const Context* ctx, TaskWork*) { return *ctx->get<T>(); }
};
template <typename T>
struct arg_traits<const T&> {
  static void set_type_bits(ArgsTypeBits* bits) { bits->set_read<T>(); }
  static const T& to(const Context* ctx, TaskWork*) { return *ctx->get<T>(); }
};

namespace args_ {

inline void set_args_type_bits(ArgsTypeBits*, t9::type_list<>) {}

template <typename T, typename... Ts>
inline void set_args_type_bits(ArgsTypeBits* bits, t9::type_list<T, Ts...>) {
  arg_traits<T>::set_type_bits(bits);
  set_args_type_bits(bits, t9::type_list<Ts...>{});
}

}  // namespace args_

template <typename... Ts>
inline ArgsTypeBits make_args_type_bits() {
  ArgsTypeBits bits;
  args_::set_args_type_bits(&bits, t9::type_list<Ts...>{});
  return bits;
}

}  // namespace sai::task
