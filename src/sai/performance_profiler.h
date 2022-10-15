#pragma once

#include <t9/singleton.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "config.h"

namespace sai {

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
        Uint64 time;
        const char* name;
      } start;
      struct {
        Uint64 time;
      } end;
    };
  };

  // Timeline.
  struct Timeline {
    std::string name;
    std::vector<Tag> tags;
  };

 private:
  std::map<SDL_threadID, std::unique_ptr<Timeline>> timelines_;

 public:
  PerformanceProfiler() = default;

  void start_tag(const char* name);
  void end_tag();
};

}  // namespace sai
