#pragma once
#include "phase.h"

#include <algorithm>
#include <map>

#include "../job/executor.h"
#include "imgui.h"

namespace sai::task {

void Phase::run(const Context* ctx) {
  if (auto executor = ctx->get<job::Executor>()) {
    std::for_each(tasks.begin(), tasks.end(), [executor, ctx](auto& task) {
      task->set_context(ctx);
      task->reset_state();
      executor->submit(task);
    });
    executor->kick();
    executor->join();
  }
}

void render_debug_gui(const PhaseReference* ref) {
  for (auto& phase : *ref->phases) {
    if (ImGui::CollapsingHeader(phase->name.c_str())) {
      std::map<Task*, int> task_depth;
      int max_depth = 0;
      for (auto& task : phase->tasks) {
        int depth = 0;
        if (!task->dependencies().empty()) {
          for (auto& d_task : task->dependencies()) {
            if (auto it = task_depth.find(d_task); it != task_depth.end()) {
              depth = std::max(it->second + 1, depth);
            }
          }
        }
        max_depth = std::max(depth, max_depth);
        task_depth.emplace(task.get(), depth);
      }

      std::multimap<int, Task*> task_order;
      task_order.clear();
      for (auto it : task_depth) {
        task_order.emplace(it.second, it.first);
      }

      for (int i = 0; i <= max_depth; ++i) {
        ImGui::Text("%d:", i);
        auto range = task_order.equal_range(i);
        for (auto it = range.first; it != range.second; ++it) {
          ImGui::SameLine();
          ImGui::Text("[%s]", it->second->name().c_str());
        }
      }
    }
  }
}

}  // namespace sai::task

/*
std::map<Task*, int> task_depth;
int max_depth = 0;
for (auto& task : tasks_) {
  int depth = 0;
  if (!task->dependencies().empty()) {
    for (auto& d_task : task->dependencies()) {
      if (auto it = task_depth.find(d_task); it != task_depth.end()) {
        depth = std::max(it->second + 1, depth);
      }
    }
  }
  max_depth = std::max(depth, max_depth);
  task_depth.emplace(task.get(), depth);
}

std::multimap<int, Task*> task_order;
int max_order = 0;
task_order.clear();
for (auto it : task_depth) {
  task_order_.emplace(it.second, it.first);
}
max_order = max_depth;

for (int i = 0; i <= max_order_; ++i) {
  ImGui::Text("%d:", i);
  auto range = task_order.equal_range(i);
  for (auto it = range.first; it != range.second; ++it) {
    ImGui::SameLine();
    ImGui::Text("[%s]", it->second->name().c_str());
  }
}
*/
