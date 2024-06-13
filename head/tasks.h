//
// Created by xjc on 2024/6/11.
//
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unordered_set>
#include <sstream>
#ifndef MY_WEBSERVER_TASKS_H
#define MY_WEBSERVER_TASKS_H

int const USER_INFO_MAX=40;
class tasks
{
public:
    static void make_connection(int listenfd,int epollfd,std::unordered_set<int> &clients);
private:
    tasks();
};
#endif //MY_WEBSERVER_TASKS_H

void tasks::make_connection(int listenfd,int epollfd,std::unordered_set<int>& clients)
{

    socklen_t clilen=sizeof(sockaddr_in);
    sockaddr_in cliaddr;
    int connfd=accept(listenfd,(sockaddr *)&cliaddr,&clilen);
    clients.insert(connfd);
    std::cout<<"有客户访问"<<std::endl;
    //write(connfd,"连接到服务器",sizeof("连接到服务器"));
    std::string account,passwd;
    char buf[USER_INFO_MAX];
    int ret=recv(connfd,buf,USER_INFO_MAX,0);
    if(ret==-1)
    {
        close(connfd);
        std::cout<<std::format("client {} closed connection\n",connfd);
    }
    std::string user_info=std::string(buf);

    int acc_len=0;
    std::istringstream ss(user_info.substr(0,user_info.find('#'))); //获取账号字段长度
    ss >> acc_len;
    user_info=user_info.substr(user_info.find('#')+1);

    //将账号密码分离
    account=user_info.substr(0,acc_len);
    passwd=user_info.substr(acc_len);
    std::cout<<std::format("账号：{}\n密码：{}\n",account,passwd);
    connection_pool *mysql_pool=connection_pool::getinstance();
    MYSQL * mysql_connection= nullptr;
    mysql_connection=mysql_pool->dispath_connection();

    mysql_pool->free_connection(std::move(mysql_connection));

    //send(connfd,"连接成功",sizeof("连接成功"),0);
    epoll_event tep;
    tep.events=EPOLLIN;
    tep.data.fd=connfd;
    ret=epoll_ctl(epollfd,EPOLL_CTL_ADD,connfd,&tep);
    assert(ret>=0);
}