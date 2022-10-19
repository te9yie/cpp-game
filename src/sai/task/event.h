#pragma once

#include "args.h"

namespace sai::task {

template <typename T>
struct Event {};

template <typename T>
struct EventReaderData {};

template <typename T>
struct EventWriter {};

template <typename T>
struct EventReader {
  Event<T>* e;
  EventReaderData<T>* data;
};

template <typename T>
struct arg_traits<EventReader<T>> {
  static void set_type_bits(ArgsTypeBits* bits) { bits->set_read<Event<T>>(); }
  static EventReader<T> to(const Context* ctx, TaskWork* work) {
    auto e = ctx->get<Event<T>>();
    auto d = work->get<EventReaderData<T>>();
    if (!d) {
      d = work->add<EventReaderData<T>>();
    }
    return EventReader<T>{e, d};
  }
};

}  // namespace sai::task