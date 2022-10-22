#pragma once
#include <algorithm>
#include <cstddef>
#include <deque>
#include <memory>

#include "noncopyable.h"

namespace t9 {

// Handle.
template <typename T>
struct Handle {
  std::size_t revision = 0;
  std::size_t index = 0;

  bool operator==(const Handle& rhs) const {
    return revision == rhs.revision && index == rhs.index;
  }
  bool operator!=(const Handle& rhs) const { return !operator==(rhs); }
};

// HandleEntry.
template <typename T>
struct HandleEntry {
  std::size_t revision = 0;
  std::unique_ptr<T> x;
};

// HandleStorage.
template <typename T>
class HandleStorage : private NonCopyable {
 private:
  std::size_t peak_index_ = 0;
  std::deque<std::size_t> index_stock_;
  std::deque<HandleEntry<T>> entries_;

 public:
  template <typename... Args>
  Handle<T> create(Args&&... args) {
    auto index = find_index_();
    auto& entry = entries_[index];
    entry.x = std::make_unique<T>(std::forward<Args>(args)...);
    return Handle<T>{entry.revision, index};
  }

  bool destroy(Handle<T> handle) {
    auto& entry = entries_[handle.index];
    if (handle.revision != entry.revision) return false;

    entry.x.reset();
    ++entry.revision;
    index_stock_.emplace_back(handle.index);
    return true;
  }

  T* get(Handle<T> handle) const {
    auto& entry = entries_[handle.index];
    if (handle.revision != entry.revision) return nullptr;
    return entry.x.get();
  }

  bool exists(Handle<T> handle) const {
    auto& entry = entries_[handle.index];
    return handle.revision == entry.revision;
  }

  void clear() {
    peak_index_ = 0;
    index_stock_.clear();
    entries_.clear();
  }

  template <typename F>
  void each(F f) {
    std::for_each(entries_.begin(), entries_.end(),
                  [f](const HandleEntry<T>& handle) { f(handle.x.get()); });
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
};

}  // namespace t9