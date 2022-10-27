#pragma once

#include <array>
#include <cassert>
#include <memory>
#include <vector>

#include "args.h"
#include "context.h"

namespace sai::task {

// Event.
class Event {
 public:
  virtual ~Event() = default;
  virtual void update() = 0;
};

// EventSubject.
template <typename T>
class EventSubject : public Event {
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
  std::array<std::vector<std::unique_ptr<EventData<T>>>, 2> events_;
  std::array<std::size_t, 2> start_index_ = {0};
  std::size_t count_ = 0;
  std::size_t index_ = 0;

 public:
  virtual void update() override {
    auto index = 1 - index_;
    events_[index].clear();
    start_index_[index] = count_;
    index_ = index;
  }

 public:
  template <class... Args>
  void notify(Args&&... args) {
    auto e =
        std::make_unique<EventData<T>>(count_++, std::forward<Args>(args)...);
    events_[index_].emplace_back(std::move(e));
  }

  const EventData<T>* get_event(std::size_t index) const {
    const auto n = count_ - index;
    if (n == 0) return nullptr;
    if (n <= events_[index_].size()) {
      const auto i = index - start_index_[index_];
      return events_[index_][i].get();
    } else {
      auto last_index = 1 - index_;
      auto i = index - start_index_[last_index];
      const auto& data = events_[last_index];
      if (i < data.size()) {
        return data[i].get();
      } else if (!data.empty()) {
        return data[0].get();
      }
    }
    return nullptr;
  }
};

// EventObserver.
template <typename T>
struct EventObserver {
  std::size_t index = 0;
};

// EventWriter.
template <typename T>
struct EventWriter {
  EventSubject<T>* subject = nullptr;

  EventWriter(const Context* ctx) : subject(ctx->get<EventSubject<T>>()) {}

  template <class... Args>
  void notify(Args&&... args) {
    assert(subject);
    subject->notify<Args...>(std::forward<Args>(args)...);
  }
};

// EventReader.
template <typename T>
struct EventReader {
 private:
  EventSubject<T>* subject_ = nullptr;
  std::size_t* index_ = nullptr;

 public:
  EventReader(const Context* ctx, std::size_t* i)
      : subject_(ctx->get<EventSubject<T>>()), index_(i) {}

  template <typename F>
  void each(F f) {
    assert(subject_);
    for (auto e = subject_->get_event(*index_); e;
         e = subject_->get_event(*index_)) {
      *index_ = e->index + 1;
      f(e->x);
    }
  }
};

// arg_traits.
template <typename T>
struct arg_traits<EventWriter<T>> {
  static void set_type_bits(ArgsTypeBits* bits) {
    bits->set_write<EventSubject<T>>();
  }
  static EventWriter<T> to(const Context* ctx, TaskWork*) {
    return EventWriter<T>(ctx);
  }
};

template <typename T>
struct arg_traits<EventReader<T>> {
  static void set_type_bits(ArgsTypeBits* bits) {
    bits->set_read<EventSubject<T>>();
  }
  static EventReader<T> to(const Context* ctx, TaskWork* work) {
    auto o = work->get<EventObserver<T>>();
    if (!o) {
      o = work->add<EventObserver<T>>();
    }
    return EventReader<T>(ctx, &o->index);
  }
};

}  // namespace sai::task