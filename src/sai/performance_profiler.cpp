#include "performance_profiler.h"

namespace sai {

void PerformanceProfiler::start_tag(const char* name) {
  auto it = timelines_.find(SDL_GetThreadID(nullptr));
  if (it == timelines_.end()) return;

  Tag tag;
  tag.type = Tag::Type::Start;
  tag.start.time = SDL_GetPerformanceCounter();
  tag.start.name = name;

  it->second->tags.emplace_back(tag);
}
void PerformanceProfiler::end_tag() {
  auto it = timelines_.find(SDL_GetThreadID(nullptr));
  if (it == timelines_.end()) return;

  Tag tag;
  tag.type = Tag::Type::End;
  tag.end.time = SDL_GetPerformanceCounter();

  it->second->tags.emplace_back(tag);
}

}  // namespace sai
