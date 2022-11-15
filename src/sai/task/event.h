#pragma once

#include <array>
#include <cassert>
#include <memory>
#include <vector>

#include "args.h"
#include "context.h"

namespace sai::task {

// Event.
template <typename T>
class Event {
 public:
  // EventData.
  template <class T>
  struct EventData {
    size_t index;
    T x;

    template <class... Args>
    EventData(size_t index, Args&&... args)
        : index(index), x(std::forward<Args>(args)...) {}
  };

 private:
  static constexpr std::size_t FRAME_N = 2;
  std::array<std::vector<std::unique_ptr<EventData<T>>>, FRAME_N> events_;
  std::array<std::size_t, FRAME_N> start_index_ = {0};
  std::size_t count_ = 0;
  std::size_t frame_index_ = 0;

 public:
  virtual void update() {
    auto index = (frame_index_ + 1) % FRAME_N;
    events_[index].clear();
    start_index_[index] = count_;
    frame_index_ = index;
  }

 public:
  template <class... Args>
  void notify(Args&&... args) {
    auto e =
        std::make_unique<EventData<T>>(count_++, std::forward<Args>(args)...);
    events_[frame_index_].emplace_back(std::move(e));
  }

  const EventData<T>* get_event(std::size_t index) const {
    auto n = count_ - index;
    if (n == 0) return nullptr;
    auto f_i = frame_index_;
    do {
      if (n <= events_[f_i].size()) {
        auto i = index - start_index_[f_i];
        return events_[f_i][i].get();
      }
      n -= events_[f_i].size();
      f_i = (f_i + FRAME_N - 1) % FRAME_N;
    } while (f_i != frame_index_);
    return nullptr;
  }
};

// update_events.
template <typename T>
void update_events(Event<T>* e) {
  e->update();
}

// EventObserver.
template <typename T>
struct EventObserver {
  std::size_t index = 0;
};

// EventWriter.
template <typename T>
struct EventWriter {
  Event<T>* event = nullptr;

  EventWriter(const AppContext* ctx) : event(ctx->get<Event<T>>()) {}

  template <class... Args>
  void notify(Args&&... args) {
    assert(event);
    event->notify<Args...>(std::forward<Args>(args)...);
  }
};

// EventReader.
template <typename T>
struct EventReader {
  Event<T>* event = nullptr;
  std::size_t* index = nullptr;

  EventReader(const AppContext* ctx, std::size_t* i)
      : event(ctx->get<Event<T>>()), index(i) {}

  template <typename F>
  void each(F f) {
    assert(event);
    for (auto e = event->get_event(*index); e; e = event->get_event(*index)) {
      *index = e->index + 1;
      f(e->x);
    }
  }
};

// arg_traits.
template <typename T>
struct arg_traits<EventWriter<T>> {
  static void set_type_bits(ArgsTypeBits* bits) { bits->set_write<Event<T>>(); }
  static EventWriter<T> to(const AppContext* ctx, TaskWork*) {
    return EventWriter<T>(ctx);
  }
};

template <typename T>
struct arg_traits<EventReader<T>> {
  static void set_type_bits(ArgsTypeBits* bits) { bits->set_read<Event<T>>(); }
  static EventReader<T> to(const AppContext* ctx, TaskWork* work) {
    auto o = work->get<EventObserver<T>>();
    if (!o) {
      o = work->add<EventObserver<T>>();
    }
    return EventReader<T>(ctx, &o->index);
  }
};

}  // namespace sai::task