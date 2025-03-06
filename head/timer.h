#ifndef TIMER_H
#define TIMER_H

#include<functional>
#include<time.h>
#include<mutex>
#include<chrono>
#include<thread>
#include<iostream>
#include<shared_mutex>
class Timer{
public:
    using ptr=std::shared_ptr<Timer>;
    using millisecond=int;
    using second=int;
    Timer()=default;
    ~Timer(){
        Stop();
    };
    template<class F,class... Args>
    void SetEvent(F&& f,Args&&... args);
    void Tick(millisecond interval);
    void Start(second passtime);
    void Stop();
    void Reset(second passtime);


private:
    void _Tick();
    void Waiting();
private:

    std::function<void()> cb;
    std::mutex m_mutex;
    std::chrono::steady_clock::time_point m_time;
    std::chrono::steady_clock::duration m_interval;
    std::atomic<bool> m_stoped=true;
    std::shared_ptr<std::thread> m_thread;
};

template<class F,class... Args>
void Timer::SetEvent(F&& f,Args&&... args){
    std::lock_guard lock(m_mutex);
    cb=std::bind(std::forward<F>(f),std::forward<Args>(args)...);
    m_stoped=true;
}

#endif