#pragma once
#include <cstdint>
#include <memory>
#include <vector>

#include "task.h"

namespace sai::task {

struct FirstPhase {};
struct PreUpdatePhase {};
struct UpdatePhase {};
struct PostUpdatePhase {};
struct PreRenderPhase {};
struct RenderPhase {};
struct PostRenderPhase {};
struct LastPhase {};

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
  phase_index_type index;
  std::vector<std::shared_ptr<Task>> tasks;
};

// make_phase.
template <typename T>
inline std::unique_ptr<Phase> make_phase() {
  return std::make_unique<Phase>(Phase{phase_index<T>::index()});
}

}  // namespace sai::task