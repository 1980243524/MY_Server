//
// Created by xjc on 2024/6/11.
//
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unordered_set>
#ifndef MY_WEBSERVER_TASKS_H
#define MY_WEBSERVER_TASKS_H
int const MAXLINE=80;

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
    char buf[MAXLINE];
    int ret=recv(connfd,buf,MAXLINE,0);
    account=std::string(buf);
    ret=recv(connfd,buf,MAXLINE,0);
    passwd=std::string(buf);

    if(ret==-1)
    {
        close(connfd);
        std::cout<<std::format("client {} closed connection\n",connfd);
    }
    else
    {
        std::cout<<std::format("账号：{}\n密码：{}\n",account,passwd);
    }

    //send(connfd,"连接成功",sizeof("连接成功"),0);
    epoll_event tep;
    tep.events=EPOLLIN;
    tep.data.fd=connfd;
    ret=epoll_ctl(epollfd,EPOLL_CTL_ADD,connfd,&tep);
    assert(ret>=0);
}