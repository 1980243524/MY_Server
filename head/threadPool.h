#ifndef MY_WEBSERVER_THREADPOOL_H
#define MY_WEBSERVER_THREADPOOL_H
#include<thread>
#include<iostream>
#include<mutex>
#include<condition_variable>
#include<vector>
#include<queue>
#include<functional>
#include<format>
#include <semaphore>
#include <future>
int const THREAD_SEMAPHOR_MAX=100;

class ThreadPool
{
public:
    using ptr=std::shared_ptr<ThreadPool>;
    ThreadPool(int numThreads);
    ~ThreadPool();
    template<class F,class... Args>
    auto addTask(F &&f,Args&&... args)->std::future<decltype(f(args...))>;
private:
    void run();


    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex m_mutex;        //互斥访问任务队列
    std::counting_semaphore<10> m_taskNum;
    int m_minTask;
    std::atomic<bool> stop;              //线程停止标志位
};


template<class F,class... Args>
auto ThreadPool::addTask(F &&f,Args&&... args)->std::future<decltype(f(args...))>
{
    using RetType=decltype(f(args...));

    auto task = std::make_shared<std::packaged_task<RetType()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<RetType> res = task->get_future();
    {
        std::lock_guard lock(m_mutex);
        if(stop)
            throw std::runtime_error("enqueue on stopped ThreadPool");
        tasks.emplace([task](){ (*task)(); });
    }
    m_taskNum.release();
    return res;
}


#endif
