#pragma once
#include <SDL.h>

#include <cstddef>
#include <memory>
#include <utility>

#include "sync/mutex.h"
#include "t9/noncopyable.h"

namespace sai {

// HandleId.
struct HandleId {
  std::size_t revision = 0;
  std::size_t index = 0;

  static inline const HandleId INVALID;

  bool operator==(const HandleId& rhs) const {
    return revision == rhs.revision && index && rhs.index;
  }
  bool operator!=(const HandleId& rhs) const { return !operator==(rhs); }
};

// HandleObserver.
class HandleObserver {
 public:
  virtual ~HandleObserver() = default;
  virtual void on_grab(HandleId id) = 0;
  virtual void on_drop(HandleId id) = 0;
};

// Handle.
template <typename T>
struct Handle final {
  HandleId id;
  HandleObserver* observer = nullptr;

  Handle() = default;
  Handle(HandleId id, HandleObserver* ob) : id(id), observer(ob) {}
  Handle(const Handle& rhs) : Handle(rhs.id, rhs.observer) {
    if (observer) {
      observer->on_grab(id);
    }
  }
  Handle(Handle&& rhs) : Handle(rhs.id, rhs.observer) {
    rhs.id = HandleId::INVALID;
    rhs.observer = nullptr;
  }
  ~Handle() {
    if (observer) {
      observer->on_drop(id);
    }
  }

  Handle& operator=(const Handle& rhs) {
    Handle(rhs).swap(*this);
    return *this;
  }
  Handle& operator=(Handle&& rhs) {
    Handle(std::move(rhs)).swap(*this);
    return *this;
  }

  void swap(Handle& rhs) {
    using std::swap;
    swap(id, rhs.id);
    swap(observer, rhs.observer);
  }
};

// HandleEntry.
template <typename T>
struct HandleEntry {
  SDL_Atomic_t ref_count;
  std::size_t revision = 0;
  std::unique_ptr<T> x;
};

// HandleStorage
template <typename T>
class HandleStorage : private HandleObserver, private t9::NonCopyable {
 private:
  std::size_t peak_index_ = 0;
  std::deque<std::size_t> index_stock_;
  std::deque<EntryHandle<T>> entries_;
  std::deque<HandleId> remove_ids_;
  sync::MutexPtr remove_mutex_;

 public:
  template <typename... Args>
  Handle<T> create(Args&&... args) {
    auto index = find_index_();
    auto& entry = entries_[index];
    SDL_AtomicSet(&entry.ref_count, 1);
    entry.x = std::make_unique<T>(std::forward<Args>(args)...);
    return Handle<T>(HandleId(entry.revision, index), this);
  }

 private:
  std::size_t find_index_() {
    if (index_stock_.empty()) {
      auto index = peak_index_++;
      auto& entry = entries_.emplace_back();
      entry.revision = 1;
      return index;
    } else {
      auto index = index_stock_.front();
      index_stock_.pop_front();
      return index;
    }
  }

 private:
  // HandleObserver.
  virtual void on_grab(HandleId id) override {
    if (id.revision == 0) return;
    if (entries_.size() <= id.index) return;
    auto& entry = entries_[id.index];
    if (id.revision != entry.revision) return;
    SDL_AtomicAdd(&entry.ref_count, 1);
  }
  virtual void on_drop(HandleId id) override {
    if (id.revision == 0) return;
    if (entries_.size() <= id.index) return;
    auto& entry = entries_[id.index];
    if (id.revision != entry.revision) return;
    assert(SDL_AtomicGet(&entry.ref_count) > 0);
    if (SDL_AtomicAdd(&entry.ref_count, -1) > 1) return;

    sync::UniqueLock lock(remove_mutex_.get());
    remove_ids_.emplac_back(id);
  }
};

}  // namespace sai