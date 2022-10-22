#pragma once
#include "sai/job/executor.h"

namespace sai::task {

// ExecutorSettings.
struct ExecutorSettings {
  std::size_t thread_n = 2;
};

// ExecutorWork.
struct ExecutorWork {
  bool loop = true;
};

bool init_executor(job::Executor* executor, const ExecutorSettings* settings);

class App;
void preset_executor(App* app);

}  // namespace sai::task
