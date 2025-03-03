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

int const THREAD_SEMAPHOR_MAX=100;

class ThreadPool
{
public:
    using ptr=std::shared_ptr<ThreadPool>;
    ThreadPool(int numThreads);
    ~ThreadPool();
    template<class F,class... Args>
    void addTask(F &&f,Args&&... args);
private:
    void run();


    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex m_mutex;        //互斥访问任务队列
    std::counting_semaphore<10> m_taskNum;
    int m_minTask;
    std::atomic<bool> stop;              //线程停止标志位
};


ThreadPool::ThreadPool(int numThreads):stop(false), m_taskNum(0),
                                        m_minTask(numThreads<THREAD_SEMAPHOR_MAX?numThreads:THREAD_SEMAPHOR_MAX)
{
    if(numThreads<=0){
        throw std::logic_error("线程池初始线程数不能为0");
    }
    for(int i=0;i<numThreads;i++)
    {
        threads.emplace_back([this](){
            run();
        });
        threads[i].detach();
    }
};
ThreadPool::~ThreadPool()
{
    stop=true;
    m_taskNum.release();

}

template<class F,class... Args>
void ThreadPool::addTask(F &&f,Args&&... args)
{
    std::function<void()>task = std::bind(std::forward<F>(f),std::forward<Args>(args)...);
    
    std::lock_guard lock(m_mutex);
    tasks.emplace(std::move(task));

    m_taskNum.release();
}

void ThreadPool::run(){
        while(true)
        {
            m_taskNum.acquire();                         //判断当前是否有任务或线程是否停止决定是否阻塞
            if(stop)
            {
                m_taskNum.release();
                return;
            }
            std::function<void()> task;
            {            
                std::lock_guard lock(m_mutex);
                task=std::move(tasks.front());    //从任务队列中取任务
                tasks.pop();
            }
            task();                                             //执行任务
        }
    
}
#endif
