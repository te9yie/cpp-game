#pragma once
#include <deque>
#include <memory>

#include "task.h"

namespace sai::task {

// Runner.
class Runner {
 public:
  virtual ~Runner() = default;
  virtual bool run(const TaskList* tasks) = 0;
};

// DefaultRunner.
class DefaultRunner : public Runner {};
}  // namespace sai::task