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
    using milisecond=int;
    using second=int;
    Timer()=default;
    ~Timer(){};
    template<class F,class... Args>
    void SetEvent(F&& f,Args&&... args);
    void Tick(milisecond interval);
    void Start(second passtime);
    void Stop();
    void Reset(second passtime);


private:
    void _Tick();
    void Waiting();
private:

    std::function<void()> cb;
    std::shared_mutex m_cb_mutex;
    std::mutex m_time_mutex;
    std::chrono::steady_clock::time_point m_time;
    std::chrono::steady_clock::duration m_interval;
    std::atomic<bool> m_stoped=true;
};

template<class F,class... Args>
void Timer::SetEvent(F&& f,Args&&... args){
    std::unique_lock lock(m_cb_mutex);
    cb=std::bind(std::forward<F>(f),std::forward<Args>(args)...);
    m_stoped=true;
}

#endif