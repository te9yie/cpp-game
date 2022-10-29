#pragma once
#include "args.h"
#include "context.h"

namespace sai::task {

// LocalApp.
template <typename... Ts>
class LocalApp final {
 private:
  const Context* ctx_ = nullptr;
  TaskWork* work_ = nullptr;

 public:
  explicit LocalApp(const Context* ctx, TaskWork* work)
      : ctx_(ctx), work_(work) {}

  template <typename U>
  U* get_context() const {
    return arg_traits<U*>::to(ctx_, work_);
  }

  template <typename U>
  U get_event() const {
    return arg_traits<U>::to(ctx_, work_);
  }
};

// arg_traits.
template <typename... Ts>
struct arg_traits<LocalApp<Ts...>> {
  static void set_type_bits_(ArgsTypeBits*, t9::type_list<>) {}
  template <typename U, typename... Us>
  static void set_type_bits_(ArgsTypeBits* bits, t9::type_list<U, Us...>) {
    arg_traits<U>::set_type_bits(bits);
    set_type_bits_(bits, t9::type_list<Us...>{});
  }
  static void set_type_bits(ArgsTypeBits* bits) {
    set_type_bits_(bits, t9::type_list<Ts...>{});
  }
  static LocalApp<Ts...> to(const Context* ctx, TaskWork* work) {
    return LocalApp<Ts...>(ctx, work);
  }
};

template <typename... Ts>
struct arg_traits<const LocalApp<Ts...>&> {
  static void set_type_bits(ArgsTypeBits* bits) {
    arg_traits<LocalApp<Ts...>>::set_type_bits(bits);
  }
  static LocalApp<Ts...> to(const Context* ctx, TaskWork* work) {
    return arg_traits<LocalApp<Ts...>>::to(ctx, work);
  }
};

}  // namespace sai::task
