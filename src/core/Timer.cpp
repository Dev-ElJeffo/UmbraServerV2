#include "Timer.hpp"
#include "Logger.hpp"
#include <algorithm>

namespace Umbra {
namespace Core {

Timer::Timer() : running_(false), nextTaskId_(1) {}

Timer::~Timer() {
  stop();
}

uint32_t Timer::scheduleRepeating(Callback callback, Duration interval) {
  std::lock_guard<std::mutex> lock(tasksMutex_);
  
  auto task = std::make_shared<Task>();
  task->id = nextTaskId_++;
  task->callback = callback;
  task->interval = interval;
  task->nextRun = std::chrono::steady_clock::now() + interval;
  task->repeating = true;
  task->cancelled = false;
  
  tasks_.push_back(task);
  
  Logger::getInstance().debug("Scheduled repeating task {} (interval: {}ms)", 
                              task->id, interval.count());
  
  return task->id;
}

uint32_t Timer::scheduleOnce(Callback callback, Duration delay) {
  std::lock_guard<std::mutex> lock(tasksMutex_);
  
  auto task = std::make_shared<Task>();
  task->id = nextTaskId_++;
  task->callback = callback;
  task->interval = delay;
  task->nextRun = std::chrono::steady_clock::now() + delay;
  task->repeating = false;
  task->cancelled = false;
  
  tasks_.push_back(task);
  
  Logger::getInstance().debug("Scheduled one-time task {} (delay: {}ms)", 
                              task->id, delay.count());
  
  return task->id;
}

bool Timer::cancel(uint32_t taskId) {
  std::lock_guard<std::mutex> lock(tasksMutex_);
  
  auto it = std::find_if(tasks_.begin(), tasks_.end(),
    [taskId](const std::shared_ptr<Task>& task) {
      return task->id == taskId;
    });
  
  if (it != tasks_.end()) {
    (*it)->cancelled = true;
    Logger::getInstance().debug("Cancelled task {}", taskId);
    return true;
  }
  
  return false;
}

void Timer::start() {
  if (running_) {
    Logger::getInstance().warn("Timer already running");
    return;
  }
  
  running_ = true;
  workerThread_ = std::make_unique<std::thread>(&Timer::workerLoop, this);
  
  Logger::getInstance().info("Timer started");
}

void Timer::stop() {
  if (!running_) {
    return;
  }
  
  running_ = false;
  
  if (workerThread_ && workerThread_->joinable()) {
    workerThread_->join();
  }
  
  {
    std::lock_guard<std::mutex> lock(tasksMutex_);
    tasks_.clear();
  }
  
  Logger::getInstance().info("Timer stopped");
}

bool Timer::isRunning() const {
  return running_;
}

void Timer::workerLoop() {
  while (running_) {
    executeTasks();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void Timer::executeTasks() {
  std::lock_guard<std::mutex> lock(tasksMutex_);
  
  auto now = std::chrono::steady_clock::now();
  
  for (auto it = tasks_.begin(); it != tasks_.end();) {
    auto& task = *it;
    
    if (task->cancelled) {
      it = tasks_.erase(it);
      continue;
    }
    
    if (now >= task->nextRun) {
      try {
        task->callback();
      } catch (const std::exception& e) {
        Logger::getInstance().error("Task {} threw exception: {}", 
                                    task->id, e.what());
      }
      
      if (task->repeating) {
        task->nextRun = now + task->interval;
        ++it;
      } else {
        it = tasks_.erase(it);
      }
    } else {
      ++it;
    }
  }
}

}  // namespace Core
}  // namespace Umbra

