#ifndef USER_H
#define USER_H
#include<iostream>
#include"timer.h"

struct User{
    std::string m_name;
    int m_fd;
    std::shared_ptr<Timer> m_timer;
    User()=default;
    User(std::string name,int fd):m_name(name),m_fd(fd),m_timer(std::make_shared<Timer>()) {}
};


#endif