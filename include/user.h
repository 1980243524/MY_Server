#ifndef USER_H
#define USER_H
#include <chrono>
#include <cstdint>
#include <iostream>
#include <thread>
#include <unordered_map>
#include "timer.h"

struct User {
  std::string m_name;
  int m_fd;
  uint32_t id = 0;
  std::shared_ptr<Timer> m_timer;
  User() = default;
  User(std::string name, int fd) : m_name(name), m_fd(fd), m_timer(std::make_shared<Timer>()) {}
};

class UserManager {
 public:
  UserManager() = default;
  ~UserManager() = default;

  void UserOnline(int32_t fd);
  void UserDownline(int32_t fd);

 private:
  void CheckOnline();
  void UserAccess(int32_t fd);
  std::unordered_map<int32_t, User> users_;
  std::thread th_check_online_;
};
#endif