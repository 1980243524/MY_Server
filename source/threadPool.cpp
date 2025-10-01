#include "threadPool.h"

ThreadPool::ThreadPool(int numThreads)
    : stop_(false), task_num_(0), min_task_(numThreads < THREAD_SEMAPHOR_MAX ? numThreads : THREAD_SEMAPHOR_MAX) {
  if (numThreads <= 0) {
    throw std::logic_error("线程池初始线程数不能为0");
  }
  for (int i = 0; i < numThreads; i++) {
    threads_.emplace_back([this]() { run(); });
    threads_[i].detach();
  }
}

ThreadPool::~ThreadPool() {
  stop_ = true;
  task_num_.release();
}

void ThreadPool::run() {
  while (true) {
    task_num_.acquire();  // 判断当前是否有任务或线程是否停止决定是否阻塞
    if (stop_) {
      task_num_.release();
      return;
    }
    std::function<void()> task;
    {
      std::lock_guard lock(mu_);
      task = std::move(tasks_.front());  // 从任务队列中取任务
      tasks_.pop();
    }
    task();  // 执行任务
  }
}
