#ifndef SRC_SCANNER_LIB_THREAD_POOL_H_
#define SRC_SCANNER_LIB_THREAD_POOL_H_

#include <condition_variable>

#include <atomic>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <vector>

namespace scanner {

/**
 * @class ThreadPool
 * @brief Manages a pool of worker threads to execute tasks concurrently.
 *
 * This class creates a fixed number of threads upon construction and allows
 * tasks to be enqueued for execution. It provides a graceful shutdown mechanism
 * that can be initiated manually via Stop() or automatically in the destructor.
 * Once stopped, no new tasks can be enqueued.
 */
class ThreadPool {
public:
  /**
   * @brief Constructs a thread pool with a specified number of threads.
   * @param num_threads The number of worker threads to create. If 0, it
   * defaults to the number of hardware concurrency units, with a minimum of 1.
   */
  explicit ThreadPool(std::size_t num_threads = 0);

  /**
   * @brief Destructor. Initiates a graceful shutdown and joins all threads.
   *
   * The destructor calls Stop() to signal workers to finish and then waits
   * for all currently executing and queued tasks to complete.
   */
  ~ThreadPool();

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  /**
   * @brief Initiates the shutdown of the thread pool.
   *
   * Sets a flag that prevents new tasks from being enqueued and wakes up all
   * worker threads. The workers will complete any remaining tasks in the queue
   * and then exit. This method is idempotent and thread-safe.
   */
  void Stop();

  /**
   * @brief Enqueues a task for execution by a worker thread.
   *
   * This method is thread-safe.
   *
   * @tparam F The type of the callable object.
   * @tparam Args The types of the arguments to the callable.
   * @param f The callable object (e.g., function, lambda).
   * @param args The arguments to be passed to the callable.
   * @return A std::future that will hold the result of the task's execution.
   * @throws std::runtime_error if the pool has been stopped.
   */
  template <class F, class... Args>
  auto Enqueue(F&& f, Args&&... args)
      -> std::future<std::invoke_result_t<F, Args...>>;

private:
  void Worker();

  std::vector<std::thread> workers_;
  std::queue<std::function<void()>> tasks_;

  std::mutex queue_mutex_;
  std::condition_variable condition_;
  std::atomic<bool> stop_{false};
};

template <class F, class... Args>
auto ThreadPool::Enqueue(F&& f, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>> {
  using return_type = std::invoke_result_t<F, Args...>;

  auto task = std::make_shared<std::packaged_task<return_type()>>(
      std::bind(std::forward<F>(f), std::forward<Args>(args)...));

  std::future<return_type> res = task->get_future();
  {
    std::unique_lock<std::mutex> lock(queue_mutex_);

    // Don't allow enqueueing after stopping.
    if (stop_.load()) {
      throw std::runtime_error("Enqueue on stopped ThreadPool");
    }

    tasks_.emplace([task]() { (*task)(); });
  }
  condition_.notify_one();
  return res;
}

}  // namespace scanner

#endif  // SRC_SCANNER_LIB_THREAD_POOL_H_