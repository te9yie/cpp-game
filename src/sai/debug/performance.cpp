#include "performance.h"

#include <cassert>
#include <cmath>
#include <numeric>
#include <stack>

#include "imgui.h"
#include "t9/color.h"
#include "t9/hash.h"

namespace sai::debug {

SDL_threadID PerformanceProfiler::setup_thread(std::string_view name) {
  assert(!ticked_);
  auto id = SDL_ThreadID();
  auto timeline = std::make_unique<Timeline>();
  timeline->name = name;
  timelines_.emplace(id, std::move(timeline));
  return id;
}

void PerformanceProfiler::tick() {
  if (!ticked_) {
    ticked_ = true;
  }
  auto read_index = read_index_();
  auto write_index = write_index_();
  start_count_[read_index] = SDL_GetPerformanceCounter();
  frame_count_[read_index] = frame_count_[write_index] + 1;
  for (auto& it : timelines_) {
    it.second->tags[read_index].clear();
  }
  use_back_buffer_ = !use_back_buffer_;
}

std::size_t PerformanceProfiler::start_tag(std::string_view name) {
  if (!ticked_) return 0;
  auto index = write_index_();

  auto it = timelines_.find(SDL_ThreadID());
  if (it == timelines_.end()) return 0;

  Tag tag;
  tag.type = Tag::Type::Start;
  tag.start.count = SDL_GetPerformanceCounter();
  tag.start.name = name.data();

  it->second->tags[index].emplace_back(tag);

  return frame_count_[index];
}
void PerformanceProfiler::end_tag(std::size_t start_id) {
  if (!ticked_) return;
  auto index = write_index_();

  auto it = timelines_.find(SDL_ThreadID());
  if (it == timelines_.end()) return;

  Tag tag;
  tag.type = Tag::Type::End;
  tag.end.count = SDL_GetPerformanceCounter();
  tag.end.frame = start_id;

  it->second->tags[index].emplace_back(tag);
}

void PerformanceProfiler::render_debug_gui() const {
  auto freq = SDL_GetPerformanceFrequency();
  auto index = read_index_();
  auto frame_count = frame_count_[index];
  auto count2us = [freq](Uint64 count) { return count * 1000 * 1000 / freq; };

  if (auto it = timelines_.find(main_thread_id_); it != timelines_.end()) {
    auto timeline = it->second.get();
    if (ImGui::CollapsingHeader(timeline->name.c_str())) {
      std::stack<Tag> tag_stack;
      auto& tags = timeline->tags[index];
      for (auto& tag : tags) {
        if (tag.type == Tag::Type::Start) {
          tag_stack.push(tag);
        } else {
          if (tag.end.frame == frame_count) {
            auto start_tag = tag_stack.top();
            tag_stack.pop();
            auto delta_us = count2us(tag.end.count - start_tag.start.count);
            ImGui::Text("%s: %zu", start_tag.start.name, delta_us);
          }
        }
      }
    }
  }

  auto canvas_p0 = ImGui::GetCursorScreenPos();
  auto canvas_s = ImGui::GetContentRegionAvail();
  if (canvas_s.x < 100.0f) canvas_s.x = 100.0f;
  if (canvas_s.y < 100.0f) canvas_s.y = 100.0f;
  auto canvas_p1 = ImVec2(canvas_p0.x + canvas_s.x, canvas_p0.y + canvas_s.y);

  const auto us30f = 1000 * 1000 / 30;
  const auto us60f = 1000 * 1000 / 60;
  const auto us2x = [us30f, &canvas_s](Uint64 us) {
    return canvas_s.x * us / us30f;
  };
  const auto start_count = start_count_[index];
  const auto line_h = 10.0f;
  const auto timeline_h = line_h * 3;

  auto draw = ImGui::GetWindowDrawList();
  draw->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(0x30, 0x30, 0x30, 0xff));

  int timeline_i = 0;
  for (auto& it : timelines_) {
    auto timeline = it.second.get();
    auto offset = ImVec2(canvas_p0.x, canvas_p0.y + timeline_i++ * timeline_h);

    std::stack<Tag> tag_stack;
    auto& tags = timeline->tags[index];
    for (auto& tag : tags) {
      if (tag.type == Tag::Type::Start) {
        tag_stack.push(tag);
      } else {
        if (tag.end.frame == frame_count) {
          auto start_tag = tag_stack.top();
          tag_stack.pop();
          auto start_us = count2us(start_tag.start.count - start_count);
          auto end_us = count2us(tag.end.count - start_count);
          auto depth = tag_stack.size();

          auto name_hash = t9::fnv1a(start_tag.start.name,
                                     std::strlen(start_tag.start.name));
          auto h = 1.0f * name_hash / std::numeric_limits<std::uint32_t>::max();
          auto rgb = t9::hsv2rgb(h, 0.95f, 0.95f);
          auto p0 =
              ImVec2(us2x(start_us) + offset.x, depth * line_h + offset.y);
          auto p1 =
              ImVec2(us2x(end_us) + offset.x, (depth + 1) * line_h + offset.y);
          draw->AddRectFilled(p0, p1, ImColor(rgb.r, rgb.g, rgb.b));
        }
      }
    }
  }

  {  // fps line.
    auto x30 = us2x(us30f);
    draw->AddLine(ImVec2(x30 + canvas_p0.x, canvas_p0.y),
                  ImVec2(x30 + canvas_p0.x, canvas_p1.y),
                  IM_COL32(0xff, 0x00, 0x00, 0x88));
    auto x60 = us2x(us60f);
    draw->AddLine(ImVec2(x60 + canvas_p0.x, canvas_p0.y),
                  ImVec2(x60 + canvas_p0.x, canvas_p1.y),
                  IM_COL32(0xff, 0x00, 0x00, 0x88));
  }
  draw->AddRect(canvas_p0, canvas_p1, IM_COL32(0xff, 0xff, 0xff, 0xff));
}

}  // namespace sai::debug
