#include "timer.h"

void Timer::Start(second passtime) {
  std::lock_guard lock(m_mutex);
  if (!m_stoped) {
    std::cout << "repeat start timer" << std::endl;
    return;
  }
  m_stoped = false;
  m_time = std::chrono::steady_clock::now() + std::chrono::seconds(passtime);
  m_thread = std::make_shared<std::thread>(&Timer::Waiting, this);
}

void Timer::Waiting() {
  while (!m_stoped.load()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::lock_guard lock(m_mutex);
    if (std::chrono::steady_clock::now() > m_time) {
      try {
        cb();
      } catch (...) {
        std::cout << "timer callback function error" << std::endl;
      }
      m_stoped = true;
    }
  }
}

void Timer::Stop() {
  {
    std::lock_guard lock(m_mutex);
    m_stoped = true;
  }
  if (m_thread->joinable()) m_thread->join();
}

void Timer::Reset(second passtime) {
  std::lock_guard lock(m_mutex);
  m_time = std::chrono::steady_clock::now() + std::chrono::seconds(passtime);
}

void Timer::Tick(millisecond interval) {
  std::lock_guard lock(m_mutex);
  if (!m_stoped) {
    std::cout << "repeat start timer" << std::endl;
    return;
  }
  m_stoped = false;
  m_interval = std::chrono::milliseconds(interval);
  m_time = std::chrono::steady_clock::now() + m_interval;
  m_thread = std::make_shared<std::thread>(&Timer::_Tick, this);
}

// 在timer析构时this指针失效,访问成员变量会报错
void Timer::_Tick() {
  while (!m_stoped) {
    if (std::chrono::steady_clock::now() > m_time) {
      std::lock_guard lock(m_mutex);
      try {
        cb();
      } catch (...) {
        std::cout << "timer callback function error" << std::endl;
        return;
      }
      m_time += m_interval;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
}