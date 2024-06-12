#include<thread>
#include<iostream>
#include<mutex>
#include<condition_variable>
#include<vector>
#include<queue>
#include<functional>
#include<format>
#include <semaphore>
class ThreadPool
{
public:
    ThreadPool(int numThreads):stop(false), task_number(0), mutex_task(1), mutex_stop(1)
    {
        for(int i=0;i<numThreads;i++)
        {
            threads.emplace_back([this]
                                 {
                                     while(1)
                                     {
                                         task_number.acquire();                         //判断当前是否有任务或线程是否停止决定是否阻塞
                                         mutex_stop.acquire();
                                         if(stop)
                                         {
                                             task_number.release();
                                             return;
                                         }
                                         mutex_stop.release();
                                         mutex_task.acquire();
                                         std::function<void()> task(move(tasks.front()));    //从任务队列中取任务
                                         tasks.pop();
                                         mutex_task.release();
                                         task();                                             //执行任务
                                     }
                                 });
            threads[i].detach();
        }
    };
    ~ThreadPool()
    {
        mutex_stop.acquire();
        stop=true;
        mutex_stop.release();
        task_number.release();

    }
    template<class F,class... Args>
    void addTask(F &&f,Args&&... args)
    {
        std::function<void()>task = std::bind(std::forward<F>(f),std::forward<Args>(args)...);

        mutex_task.acquire();
        tasks.emplace(std::move(task));
        mutex_task.release();

        task_number.release();
    }
private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::binary_semaphore mutex_task;        //互斥访问任务队列
    std::binary_semaphore mutex_stop;                 //互斥访问stop标记
    std::counting_semaphore<1> task_number;
    bool stop;              //线程停止标志位
};