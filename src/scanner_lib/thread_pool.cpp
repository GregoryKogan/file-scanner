#include "src/scanner_lib/thread_pool.h"

#include <stdexcept>
#include <thread>

namespace scanner {

ThreadPool::ThreadPool(std::size_t num_threads) {
  if (num_threads == 0) {
    num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) {
      num_threads = 1;
    }
  }

  workers_.reserve(num_threads);
  for (std::size_t i = 0; i < num_threads; ++i) {
    workers_.emplace_back(&ThreadPool::Worker, this);
  }
}

ThreadPool::~ThreadPool() {
  Stop();
  for (std::thread& worker : workers_) {
    if (worker.joinable()) {
      worker.join();
    }
  }
}

void ThreadPool::Stop() {
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    if (stop_.load()) {
      return;
    }
    stop_.store(true);
  }
  condition_.notify_all();
}

void ThreadPool::Worker() {
  for (;;) {
    std::function<void()> task;
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      condition_.wait(lock, [this] { return stop_.load() || !tasks_.empty(); });
      if (stop_.load() && tasks_.empty()) {
        return;
      }
      task = std::move(tasks_.front());
      tasks_.pop();
    }
    task();
  }
}

}  // namespace scanner