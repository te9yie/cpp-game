#pragma once
#include <SDL.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <deque>
#include <memory>
#include <utility>

#include "sync/mutex.h"
#include "t9/noncopyable.h"

namespace sai {

// HandleId.
struct HandleId {
  std::size_t revision = 0;
  std::size_t index = 0;

  bool is_valid() const { return revision > 0; }

  bool operator==(const HandleId& rhs) const {
    return revision == rhs.revision && index == rhs.index;
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
    rhs.id = HandleId{};
    rhs.observer = nullptr;
  }
  ~Handle() {
    if (observer) {
      observer->on_drop(id);
    }
  }

  explicit operator bool() const { return id.is_valid(); }

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
  SDL_atomic_t ref_count;
  std::size_t revision = 0;
  std::shared_ptr<T> x;
};

// HandleRemoveObserver.
template <typename T>
class HandleRemoveObserver {
 public:
  virtual ~HandleRemoveObserver() = default;
  virtual void on_remove(HandleId id, std::shared_ptr<T> p) = 0;
};

// HandleStorage
template <typename T>
class HandleStorage : private HandleObserver, private t9::NonCopyable {
 private:
  std::size_t peak_index_ = 0;
  std::deque<std::size_t> index_stock_;
  std::deque<HandleEntry<T>> entries_;
  std::deque<HandleId> remove_ids_;
  sync::MutexPtr remove_mutex_;

 public:
  Handle<T> add(std::shared_ptr<T> x) {
    auto index = find_index_();
    auto& entry = entries_[index];
    SDL_AtomicSet(&entry.ref_count, 1);
    entry.x = std::move(x);
    return Handle<T>(HandleId{entry.revision, index}, this);
  }

  void update(HandleRemoveObserver<T>* observer = nullptr) {
    sync::UniqueLock lock(remove_mutex_.get());
    for (auto id : remove_ids_) {
      assert(id.index < entries_.size());
      auto& entry = entries_[id.index];
      if (entry.revision != id.revision) continue;
      if (SDL_AtomicGet(&entry.ref_count) > 0) continue;
      if (observer) {
        observer->on_remove(id, entry.x);
      }
      entry.revision = std::max<std::size_t>(entry.revision + 1, 1);
      entry.x.reset();
      index_stock_.emplace_back(id.index);
    }
    remove_ids_.clear();
  }

  Handle<T> make_handle(HandleId id) {
    assert(id.index < entries_.size());
    auto& entry = entries_[id.index];
    if (id.revision != entry.revision) return Handle<T>{};
    SDL_AtomicSet(&entry.ref_count, 1);
    return Handle<T>(id, this);
  }

  const T* get(const HandleId& id) const {
    assert(id.index < entries_.size());
    auto& entry = entries_[id.index];
    if (id.revision != entry.revision) return nullptr;
    return entry.x.get();
  }
  T* get_mut(const HandleId& id) {
    assert(id.index < entries_.size());
    auto& entry = entries_[id.index];
    if (id.revision != entry.revision) return nullptr;
    return entry.x.get();
  }

  bool exists(const HandleId& id) const {
    assert(id.index < entries_.size());
    auto& entry = entries_[handle.id.index];
    return id.revision == entry.revision;
  }

  template <typename F>
  void each(F f) const {
    std::for_each(entries_.begin(), entries_.end(),
                  [f](const HandleEntry<T>& e) {
                    if (e.x) {
                      f(e.x.get());
                    }
                  });
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
    remove_ids_.emplace_back(id);
  }
};

}  // namespace sai
