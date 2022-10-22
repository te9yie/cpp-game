#include "executor.h"

#include "app.h"

namespace sai::task {

bool init_executor(job::Executor* executor, const ExecutorSettings* settings) {
  ExecutorSettings default_settings;
  if (!settings) {
    settings = &default_settings;
  }
  return executor->start(settings->thread_n);
}

void preset_executor(App* app) {
  app->add_context<job::Executor>("TaskExecutor");
  app->add_context<ExecutorWork>();
  app->add_setup_task(init_executor);
}

}  // namespace sai::task
