#include"../head/timer.h"



void Timer::Start(second passtime){
    if(!m_stoped){
        throw std::logic_error("repeat start timer");
    }
    m_stoped=false;

    {
        std::lock_guard lock(m_time_mutex);
        m_time=std::chrono::steady_clock::now()+std::chrono::seconds(passtime);
    }
    std::thread th(&Timer::Waiting,this);
    th.detach();
}

void Timer::Waiting(){
    while(!m_stoped){
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::lock_guard lock(m_time_mutex);
        if(std::chrono::steady_clock::now()>m_time) m_stoped=true;
    }
    std::shared_lock lock(m_cb_mutex);
    cb();
}

void Timer::Stop(){
    m_stoped=true;
}

void Timer::Reset(second passtime){
    std::lock_guard lock(m_time_mutex);
    m_time=std::chrono::steady_clock::now()+std::chrono::seconds(passtime);
}


void Timer::Tick(milisecond interval){
    if(!m_stoped){
        throw std::logic_error("repeat start timer");
    }
    m_stoped=false;
    {    
        std::lock_guard lock(m_time_mutex);
        m_interval=std::chrono::seconds(interval);
        m_time=std::chrono::steady_clock::now()+m_interval;
    }
    
    std::thread th(&Timer::_Tick,this);
    th.detach();
}


void Timer::_Tick(){
    while(!m_stoped){
        if(std::chrono::steady_clock::now()>m_time) {
            {
                std::shared_lock lock(m_cb_mutex);
                cb();
            }
            std::lock_guard lock(m_time_mutex);
            m_time+=m_interval;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}