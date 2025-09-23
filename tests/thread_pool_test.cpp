#include "src/scanner_lib/thread_pool.h"

#include <chrono>

#include <atomic>
#include <future>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "gtest/gtest.h"

namespace scanner {
namespace {

TEST(ThreadPoolTest, ConstructionAndDestruction) {
  try {
    ThreadPool pool(4);
  } catch (...) {
    FAIL() << "ThreadPool constructor or destructor threw an exception.";
  }
  SUCCEED();
}

TEST(ThreadPoolTest, ExecutesVoidTask) {
  ThreadPool pool(2);
  std::atomic<int> counter{0};

  std::future<void> fut = pool.Enqueue([&counter]() { counter.store(1); });

  fut.get();
  EXPECT_EQ(counter.load(), 1);
}

TEST(ThreadPoolTest, ExecutesTaskWithReturnValue) {
  ThreadPool pool(1);
  std::future<std::string> fut =
      pool.Enqueue([]() { return std::string("hello world"); });

  EXPECT_EQ(fut.get(), "hello world");
}

TEST(ThreadPoolTest, ExecutesMultipleTasks) {
  const int kNumTasks = 100;
  ThreadPool pool(4);
  std::atomic<int> counter{0};
  std::vector<std::future<void>> futures;

  for (int i = 0; i < kNumTasks; ++i) {
    futures.push_back(pool.Enqueue([&counter]() {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      counter++;
    }));
  }

  for (auto& fut : futures) {
    fut.get();
  }

  EXPECT_EQ(counter.load(), kNumTasks);
}

TEST(ThreadPoolTest, DestructorWaitsForTasks) {
  std::atomic<int> counter{0};
  const int kNumTasks = 8;

  {
    ThreadPool pool(4);
    for (int i = 0; i < kNumTasks; ++i) {
      pool.Enqueue([&counter]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        counter++;
      });
    }
  }

  EXPECT_EQ(counter.load(), kNumTasks);
}

TEST(ThreadPoolTest, ThrowsWhenEnqueueingAfterStop) {
  ThreadPool pool(2);

  std::this_thread::sleep_for(std::chrono::milliseconds(20));

  pool.Stop();

  EXPECT_THROW(pool.Enqueue([]() {}), std::runtime_error);
}

}  // namespace
}  // namespace scanner