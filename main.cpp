
#include <unistd.h>

#include <cstdlib>
#include <sys/epoll.h>
#include<iostream>
#include<format>
#include "head/ThreadPool.h"
#include "head/connection_pool.h"
#include <cassert>
#include <cstring>
#include "tasks.h"
#define MAX_EVENT_NUMBER 10000 //最大事件数
int const OPEN_MAX=1000;
static int epollfd = 0;



int main(int argc, char *argv[])
{
    if (argc <= 1)
    {
        std::cout<<std::format("请输入端口号\n");
        return 1;
    }
    int nready;
    std::unordered_set<int> clients;          //存储客户文件描述符

    int port = atoi(argv[1]);
    connection_pool *connPool = connection_pool::getinstance();
    connPool->init("localhost", "xjc", "593509663", "test", 3306, 8);
    ThreadPool* th_pool=new ThreadPool(8);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    int ret = 0;
    sockaddr_in address;
    memset(&address,0,sizeof(sockaddr_in));

    //定义套接字
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    int flag = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret = bind(listenfd, (struct sockaddr *)&address, sizeof(address));
    assert(ret >= 0);
    ret = listen(listenfd, 5);
    assert(ret >= 0);

    //定义事件
    epoll_event tep;
    tep.events=EPOLLIN;
    tep.data.fd=listenfd;
    epoll_event events[MAX_EVENT_NUMBER];
    epollfd = epoll_create(5);
    assert(epollfd != -1);

    ret=epoll_ctl(epollfd,EPOLL_CTL_ADD,listenfd,&tep);
    assert(ret >= 0);
    while(1)
    {
        nready= epoll_wait(epollfd,events,MAX_EVENT_NUMBER,-1);
        assert(nready>=0);
        for(int i=0;i<nready;i++)
        {
            if(events[i].data.fd==listenfd)
            {
                th_pool->addTask<void(int,int,std::unordered_set<int>&)>(tasks::make_connection,listenfd,epollfd,clients);       //为线程池添加建立连接的任务
            }

        }
    }


//    ThreadPool pool(4);
//    int x;
//    std::cout<<std::format("输入1分配线程执行累加n次任务")<<std::endl;
//    std::cout<<std::format("输入2分配线程执行求-1的n次方任务")<<std::endl;
//    std::cout<<std::format("输入3分配线程执行累减n次任务")<<std::endl;
//    std::cout<<std::format("输入0结束进程")<<std::endl;
//    while(1)
//    {
//        std::cin>>x;
//        if(x==1) pool.addTask<int(long)>(accumulation,1000000000);
//        else if(x==2) pool.addTask<int(long)>(multiply_n,1000000000);
//        else if(x==3) pool.addTask<int(long)>(minus_n,1000000000);
//        else if(x==0) break;
//    }

}

