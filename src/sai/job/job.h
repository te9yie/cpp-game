#pragma once
#include <functional>
#include <string>
#include <string_view>

namespace sai::job {

class Job;

// JobObserver.
class JobObserver {
 public:
  virtual ~JobObserver() = default;
  virtual void on_pre_exec_job(Job* job) = 0;
  virtual void on_post_exec_job(Job* job) = 0;
};

// Job.
class Job {
 public:
  enum class State {
    None,
    Submit,
    Exec,
    WaitDone,
    Done,
  };

 private:
  std::string name_;
  State state_ = State::None;
  JobObserver* observer_ = nullptr;

 public:
  explicit Job(std::string_view name) : name_(name) {}
  virtual ~Job() = default;

  bool can_exec() const { return on_can_exec(); }
  void exec();

  const std::string& name() const { return name_; }
  bool change_state(State state);
  bool reset_state() { return change_state(State::None); }
  bool is_state(State state) const { return state_ == state; }

  void set_observer(JobObserver* observer) { observer_ = observer; }

 protected:
  virtual bool on_can_exec() const { return true; }
  virtual void on_exec() = 0;
};

// FuncJob.
class FuncJob : public Job {
 private:
  using FuncType = std::function<void(void)>;

 private:
  FuncType func_;

 public:
  FuncJob(std::string_view name, const FuncType& f) : Job(name), func_(f) {}

 protected:
  virtual void on_exec() override { func_(); }
};

}  // namespace sai::job
