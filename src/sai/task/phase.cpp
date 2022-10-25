#pragma once
#include "phase.h"

#include <algorithm>
#include <iterator>
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

void Phase::setup_task_dependencies() {
  for (auto it = tasks.begin(), end = tasks.end(); it != end; ++it) {
    using rev_iter = std::reverse_iterator<decltype(it)>;
    auto task = it->get();
    if (task->is_fence()) {
      for (auto r_it = rev_iter(it), r_end = tasks.rend(); r_it != r_end;
           ++r_it) {
        task->add_dependency(r_it->get());
        if ((*r_it)->is_fence()) break;
      }
    } else {
      const auto& bits = task->type_bits();
      for (auto i : bits.writes) {
        for (auto r_it = rev_iter(it), r_end = tasks.rend(); r_it != r_end;
             ++r_it) {
          if ((*r_it)->is_fence()) {
            task->add_dependency(r_it->get());
            break;
          }
          if ((*r_it)->type_bits().is_conflict_write(i)) {
            task->add_dependency(r_it->get());
          }
        }
      }
      for (auto i : bits.reads) {
        for (auto r_it = rev_iter(it), r_end = tasks.rend(); r_it != r_end;
             ++r_it) {
          if ((*r_it)->is_fence()) {
            task->add_dependency(r_it->get());
            break;
          }
          if ((*r_it)->type_bits().is_conflict_read(i)) {
            task->add_dependency(r_it->get());
          }
        }
      }
    }
  }
}

void Phase::render_debug_gui() {
  if (ImGui::CollapsingHeader(name.c_str())) {
    std::map<Task*, int> task_depth;
    int max_depth = 0;
    for (auto& task : tasks) {
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

}  // namespace sai::task