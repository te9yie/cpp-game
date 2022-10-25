#pragma once
#include "../job/executor.h"
#include "fwd.h"

namespace sai::task {

// ExecutorSettings.
struct ExecutorSettings {
  std::size_t thread_n = 2;
};

bool init_executor(job::Executor* executor, const ExecutorSettings* settings);
void preset_executor(App* app);

}  // namespace sai::task
