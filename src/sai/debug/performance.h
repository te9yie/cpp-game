#pragma once

#include <SDL.h>

#include <array>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "t9/singleton.h"

namespace sai::debug {

// PerformanceProfiler.
class PerformanceProfiler : public t9::Singleton<PerformanceProfiler> {
 public:
  // Tag.
  struct Tag {
    enum class Type {
      None,
      Start,
      End,
    } type = Type::None;
    union {
      struct {
        Uint64 count;
        const char* name;
      } start;
      struct {
        Uint64 count;
        std::size_t frame;
      } end;
    };
  };

  // Timeline.
  struct Timeline {
    std::string name;
    std::array<std::vector<Tag>, 2> tags;
  };

 private:
  std::map<SDL_threadID, std::unique_ptr<Timeline>> timelines_;
  std::array<Uint64, 2> start_count_{0};
  std::array<std::size_t, 2> frame_count_{0};
  SDL_threadID main_thread_id_ = 0;
  bool use_back_buffer_ = false;
  bool ticked_ = false;

 public:
  PerformanceProfiler() = default;

  SDL_threadID setup_thread(std::string_view name);

  void tick();

  std::size_t start_tag(std::string_view name);
  void end_tag(std::size_t start_id);

 public:
  void render_debug_gui() const;

 private:
  std::size_t read_index_() const { return use_back_buffer_ ? 0u : 1u; }
  std::size_t write_index_() const { return use_back_buffer_ ? 1u : 0u; }
};

struct PerformanceTag {
  std::size_t start_id = 0;
  PerformanceTag(std::string_view name)
      : start_id(PerformanceProfiler::instance()->start_tag(name)) {}
  ~PerformanceTag() { PerformanceProfiler::instance()->end_tag(start_id); }
};

#define PERF_TAG_NAME_(name, line) tag##line(name)
#define PERF_TAG_NAME(name, line) PERF_TAG_NAME_(name, line)
#define PERF_TAG(name) sai::debug::PerformanceTag PERF_TAG_NAME(name, __LINE__)

}  // namespace sai::debug
