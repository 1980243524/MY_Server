#ifndef MY_WEBSERVER_THREADPOOL_H
#define MY_WEBSERVER_THREADPOOL_H
#include <condition_variable>
#include <format>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <semaphore>
#include <thread>
#include <vector>
int const THREAD_SEMAPHOR_MAX = 100;

class ThreadPool {
 public:
  using ptr = std::shared_ptr<ThreadPool>;
  explicit ThreadPool(int numThreads_);
  ~ThreadPool();
  template <class F, class... Args>
  auto AddTask(F &&f, Args &&...args) -> std::future<decltype(f(args...))>;

 private:
  void run();

  std::vector<std::thread> threads_;
  std::queue<std::function<void()>> tasks_;
  std::mutex mu_;  // 互斥访问任务队列
  std::counting_semaphore<10> task_num_;
  int min_task_;
  std::atomic<bool> stop_;  // 线程停止标志位
};

template <class F, class... Args>
auto ThreadPool::AddTask(F &&f, Args &&...args) -> std::future<decltype(f(args...))> {
  using RetType = decltype(f(args...));

  auto task =
      std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

  std::future<RetType> res = task->get_future();
  {
    std::lock_guard lock(mu_);
    if (stop_) {
      throw std::runtime_error("enqueue on stopped ThreadPool");
    }
    tasks_.emplace([task]() { (*task)(); });
  }
  task_num_.release();
  return res;
}

#endif
