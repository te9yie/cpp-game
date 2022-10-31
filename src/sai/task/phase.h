#pragma once
#include <cstdint>
#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "context.h"
#include "task.h"

namespace sai::task {

struct FirstPhase;
struct PreUpdatePhase;
struct UpdatePhase;
struct PostUpdatePhase;
struct PreRenderPhase;
struct RenderPhase;
struct PostRenderPhase;
struct LastPhase;

// phase_index_type.
using phase_index_type = std::uintptr_t;

// phase_index.
template <typename T>
struct phase_index {
  static phase_index_type index() {
    static int i = 0;
    return reinterpret_cast<phase_index_type>(&i);
  }
};

// Phase.
struct Phase {
  std::string name;
  phase_index_type index;
  std::vector<std::shared_ptr<Task>> tasks;

  void run(const Context* ctx);
  void setup_task_dependencies();
  void render_debug_gui();
};

// make_phase.
template <typename T>
inline std::unique_ptr<Phase> make_phase(std::string_view name) {
  std::string phase_name(name);
  return std::make_unique<Phase>(
      Phase{std::move(phase_name), phase_index<T>::index()});
}

}  // namespace sai::task
