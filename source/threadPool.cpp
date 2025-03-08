#include"../head/threadPool.h"

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
}

ThreadPool::~ThreadPool()
{
    stop=true;
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
