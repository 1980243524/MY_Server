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
#include"dbPool.h"
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
    int ParseRequire(int connfd);
    int ParseHead(int connfd,Require::Head& head);

    int UserOnline();
    int UserDownline(int connfd);
    int LogIn();
    int SignUp();
    int Echo(int connfd,std::string message);
    int MakeFriend();
    int PublicChat();
    int PrivateChat();
private:
    ThreadPool::ptr m_threadPool;
    DbPool::ptr m_dbPool;
    std::string m_ip;
    int m_eopllfd;
    int m_port;
    int m_listenfd;
    std::unordered_map<int,User> m_users;
    std::mutex m_mutex;
};

void ChatServer::Init(const std::string& Severip,const int& Serverport,const int& threadNum,
    const std::string DBUser, const std::string DBPassWord,const std::string DBName,const int DBPort, const unsigned int DBConnNum){
    m_ip=Severip;
    m_port=Serverport;
    m_threadPool=std::make_shared<ThreadPool>(threadNum);
    m_dbPool=DbPool::getinstance();
    m_dbPool->init(Severip,DBUser,DBPassWord,DBName,DBPort,DBConnNum);
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
                UserOnline();
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


//处理客户端发出的请求
void ChatServer::ProcessRequire(void* arg,int connfd){
    ChatServer* server=(ChatServer*)arg;
    
    int ret=server->ParseRequire(connfd);
    if(ret<0) return ;
    server->reSetfd(connfd,true);
}

//解析请求对应的操作
int ChatServer::ParseRequire(int connfd) {
    Require require;
    int ret=ParseHead(connfd,require.m_head);
    //require.m_data.resize()
    if(ret<0) return -1;
    std::vector<std::byte> buffer(require.m_head.Length);

    size_t received = 0;
    while (received < require.m_head.Length) {
        ssize_t ret = recv(connfd, buffer.data(), require.m_head.Length - received, 0);
        if (ret <= 0) {
            // 处理错误（ret == 0：连接关闭；ret < 0：错误）
            return -1;
        }
        received += ret;
    }
    // 反序列化 std::string
    
    require.m_data.assign(reinterpret_cast<const char*>(buffer.data()), require.m_head.Length);
    if(received<0) return -1;

    switch (require.m_head.DestinationId){
        case 0:
            LogIn();
            break;
        case 1:         
            SignUp();
            break;
        case 2:
            Echo(connfd,require.m_data);
            break;
        case 3:
            PublicChat();
            break;
        case 4:
            PrivateChat();
            break;
        case 5:
            MakeFriend();
            break;
        default:
        

    }
    return 0;
}
int ChatServer::ParseHead(int connfd,Require::Head& head){
    int ret=recv(connfd,&head,Require::HEADLEN,0);
    if(ret<0) {
        std::cout<<errno<<std::endl;
        return -1;
    }
    return 0;
}

int ChatServer::Echo(int connfd,std::string message){
    Response response;
    response=message;
    response.m_head.Code=200;
    response.m_head.type=1;
    int ret=write(connfd,Serialize(response).data(),response.size());
    if(ret<0) return -1;
    return 0;
}

int ChatServer::UserOnline(){
    struct sockaddr_in client;
    socklen_t client_length = sizeof( client );
    int conn_fd = accept( m_listenfd, ( struct sockaddr * )&client,&client_length );
    if(conn_fd<0){
        std::cout<<errno<<std::endl;
        return -1;
    }
    m_users[conn_fd]=User(std::string("user")+std::to_string(conn_fd),conn_fd);
    std::cout<<std::string("user")+std::to_string(conn_fd)+"online"<<std::endl;
    m_users[conn_fd].m_timer->SetEvent(&ChatServer::UserDownline,this,conn_fd);
    m_users[conn_fd].m_timer->Start(300);
    //对每个非监听文件描述符都注册 EPOLLONESHOT 事件
    addfd(conn_fd, true );
    return 0;
}

int ChatServer::UserDownline(int connfd){

    int ret=close(connfd);
    if(ret<0) {
        std::cout<<errno<<std::endl;
        return -1;
    }
    std::cout<<m_users[connfd].m_name+"downline"<<std::endl;
    m_users.erase(connfd);
    ret=epoll_ctl(m_eopllfd,EPOLL_CTL_DEL,connfd,nullptr);

    if(ret<0) {
        std::cout<<errno<<std::endl;
        return -1;
    }
    return 0;
}
#endif