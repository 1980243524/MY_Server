#ifndef CHATSERVER_H
#define CHATSERVER_H
#include<iostream>
#include"threadPool.h"
#include<sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<memory>
#include <unistd.h>
#include"util.h"
#include<unordered_map>
#include"user.h"
#include"dbPool.h"
#include<string>
#define MAX_EVENT_NUMBER 10000 //最大事件数
int const OPEN_MAX=1000;


class ChatServer{
public: 
    ChatServer()=default;
    ~ChatServer()=default;
    void Start();
    void Init(const std::string& Severip,const int& Serverport,const int& threadNum,
        const std::string DBUser, const std::string DBPassWord,const std::string DBName,const int DBPort, const unsigned int DBConnNum);
    void Stop();
    void Listen();

private:
    void addfd(int fd, bool oneshot );
    void reSetfd(int fd, bool oneshot );
    static void ProcessRequire(void* arg,int connfd);
    int ParseRequire(int connfd,Require& require);
    int ParseHead(int connfd,Require::Head& head);
    int Service(int connfd,const Require& require);
    int UserOnline();
    int UserDownline(int connfd);
    int LogIn(int connfd,const Require& require);
    int SignUp(int connfd,const Require& require);
    int Echo(int connfd,const Require& require);
    int MakeFriend(int connfd,const Require& requir);
    int PublicChat(int connfd,const Require& require);
    int PrivateChat(int connfd,const Require& requir);
private:
    ThreadPool::ptr m_threadPool;
    DbPool::ptr m_dbPool;
    std::string m_ip;
    int m_eopllfd;
    int m_port;
    int m_listenfd;
    std::unordered_map<int,User> m_users;
    std::unordered_map<int,int> m_id_fdMap;
    std::mutex m_mutex;
};


#endif