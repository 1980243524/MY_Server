#ifndef CHATSERVER_H
#define CHATSERVER_H
#include<iostream>
#include"ThreadPool.h"
#include<sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<memory>
#include <unistd.h>
#include"util.h"
#include<unordered_map>
#include"user.h"
#define MAX_EVENT_NUMBER 10000 //最大事件数
int const OPEN_MAX=1000;
static int epollfd = 0;

const int LOGIN=1;



struct Response{
    int Length;
    int Code;
    std::vector<std::byte> Data;
};

class ChatServer{
public: 
    ChatServer()=default;
    ~ChatServer()=default;
    void Start();
    void Init(const std::string& ip,const int& port,const int& threadNum);
    void Stop();
    void Listen();

private:
    void ParseRequire(Require &require,int connfd);
    static void ProcessRequire(void* arg,int connfd);
    void addfd(int fd, bool oneshot );
    void reSetfd(int fd, bool oneshot );
    int ReceiveRequire(int sockfd, Require& require);
private:
    ThreadPool::ptr m_threadPool;
    std::string m_ip;
    int m_eopllfd;
    int m_port;
    int m_listenfd;
    std::unordered_map<int,User> m_users;
    
};

void ChatServer::Init(const std::string& ip, const int& port, const int& threadNum){
    m_ip=ip;
    m_port=port;
    m_threadPool=std::make_shared<ThreadPool>(threadNum);
}

void ChatServer::addfd(int fd, bool oneshot ) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if ( oneshot ) {
        event.events |= EPOLLONESHOT;
    }
    int ret=epoll_ctl( m_eopllfd, EPOLL_CTL_ADD, fd, &event );
}

void ChatServer::reSetfd(int fd, bool oneshot ) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if ( oneshot ) {
        event.events |= EPOLLONESHOT;
    }
    int ret=epoll_ctl( m_eopllfd, EPOLL_CTL_MOD, fd, &event );
}

void ChatServer::Start(){
    Listen();

    //定义事件

    epoll_event tep;
    tep.events = EPOLLIN;
    tep.data.fd = m_listenfd;

    epoll_event events[MAX_EVENT_NUMBER];
    m_eopllfd = epoll_create(10);
    if(m_eopllfd<0) throw std::logic_error("failed to create epoll");

    int ret = epoll_ctl(m_eopllfd, EPOLL_CTL_ADD, m_listenfd, &tep);
    if(ret<0) throw std::logic_error("failed to add socket to epoll");

    while (1) {
        int nready = epoll_wait(m_eopllfd, events, MAX_EVENT_NUMBER, -1);
        //assert(nready >= 0);
        if(nready<0) std::cout<<errno<<std::endl;
        for (int i = 0; i < nready; i++) {
            int sockfd=events[i].data.fd;
            if (sockfd == m_listenfd) {
                struct sockaddr_in client;
                socklen_t client_length = sizeof( client );
                int conn_fd = accept( m_listenfd, ( struct sockaddr * )&client,
                                        &client_length );

                //对每个非监听文件描述符都注册 EPOLLONESHOT 事件
                //添加的是刚accept的fd
                addfd(conn_fd, true );
            }else if(events[i].events & EPOLLIN){
                m_threadPool->addTask(ProcessRequire,this,sockfd);
            }

        }
    }
}

void ChatServer::Listen(){
    m_listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if(m_listenfd<0) throw std::logic_error("failed to get socket");

    int ret = 0;
    sockaddr_in address;

    //定义套接字
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_port);

    int flag = 1;
    setsockopt(m_listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret = bind(m_listenfd, (struct sockaddr *) &address, sizeof(address));
    if(ret<0) throw std::logic_error(std::format("failed to bind socket port:{}",m_port));
    ret = listen(m_listenfd, 5);
    if(ret<0) throw std::logic_error("failed to listen");
}


//解析请求对应的操作
 void ChatServer::ParseRequire(Require &require,int connfd){
    switch (require.operation){
        case 1:         //echo server
            write(connfd,require.Data.data(),require.Data.size());
        break;
        case 2:
        break;
    }
}


//处理客户端发出的请求
void ChatServer::ProcessRequire(void* arg,int connfd){
    ChatServer* server=(ChatServer*)arg;
    Require require;
    int ret=server->ReceiveRequire(connfd,require);
    if(ret<0) return ;
    server->ParseRequire(require,connfd);
    server->reSetfd(connfd,true);
}

//接收请求数据
int ChatServer::ReceiveRequire(int connfd, Require& require) {

    recv(connfd,&require.Length,sizeof(int),0);
    std::vector<std::byte> buffer(require.size());
    memcpy(buffer.data(),&require.Length,sizeof(int));
    recv(connfd,&require.operation,sizeof(int),0);
    memcpy(buffer.data()+sizeof(int),&require.operation,sizeof(int));

    size_t received = 0;
    while (received < require.Length) {
        ssize_t ret = recv(connfd, buffer.data()+8, require.Length - received, 0);
        if (ret <= 0) {
            // 处理错误（ret == 0：连接关闭；ret < 0：错误）
            return -1;
        }
        received += ret;
    }


    // 反序列化 std::string

    
    require.Data.assign(reinterpret_cast<const char*>(&buffer[8]), require.Length);
    return received;
}

#endif