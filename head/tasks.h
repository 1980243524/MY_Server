// Created by xjc on 2024/6/11.
//
#ifndef MY_WEBSERVER_TASKS_H
#define MY_WEBSERVER_TASKS_H
#include <iostream>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unordered_set>
#include <sstream>
#include <unistd.h>


int const LOGIN_FLAG=1;     //登录功能标志
int const ENROLL_FLAG=2;    //注册功能标志

int const USER_INFO_MAX=40;
int const NOT_ERROR=-1;
int const ACCOUNT_NOT_EXITST=1;
int const PASSWD_ERROR=2;
int const ACCOUNT_EXIST=3;

struct message

{
    int *length;
    char* data;
    message()
    {
        length=new int(0);
        data=new char[USER_INFO_MAX];
    }
    ~message()
    {
        delete length;
        delete data;
    }
};


class tasks
{
public:
    static void make_connection(int listenfd,int epollfd,std::unordered_set<int> &clients);
private:
    tasks(){};
    static int check_login(std::string account,std::string passwd,int connfd);
    static int check_enroll(std::string account,std::string passwd,int connfd);
};

void tasks::make_connection(int listenfd,int epollfd,std::unordered_set<int>& clients)
{
    int error_flag=NOT_ERROR;
    socklen_t clilen=sizeof(sockaddr_in);
    sockaddr_in cliaddr;
    int connfd=accept(listenfd,(sockaddr *)&cliaddr,&clilen);
    clients.insert(connfd);
    std::cout<<"有客户访问"<<std::endl;


    int option=0;
    recv(connfd,&option,sizeof(option),0);
    std::string account,passwd;
    message m;

    recv(connfd,m.length,sizeof(int),0);

    int receved_len=0;
    while(*m.length>0) {
        receved_len = recv(connfd, m.data, *m.length, 0);
        *m.length -= receved_len;
    }
    account=std::string(m.data);

    strcpy(m.data, "");

    recv(connfd,m.length,sizeof(int),0);
    while(*m.length>0)
    {
        receved_len=recv(connfd,m.data,*m.length,0);
        *m.length-=receved_len;
    }
    passwd=std::string(m.data);

    std::cout<<std::format("账号：{}\n密码：{}\n",account,passwd);

    if(option==LOGIN_FLAG)
    {
        error_flag=check_login(account,passwd,connfd);
    }
    else if(option==ENROLL_FLAG)
    {
        error_flag=check_enroll(account,passwd,connfd);
    }
    std::cout<<error_flag<<std::endl;
    send(connfd,&error_flag,sizeof(error_flag),0);
    if(error_flag>=0)
    {
        close(connfd);
        return;
    }

    epoll_event tep;
    tep.events=EPOLLIN;
    tep.data.fd=connfd;
    int ret=epoll_ctl(epollfd,EPOLL_CTL_ADD,connfd,&tep);
    assert(ret>=0);
}

int tasks::check_login(std::string account,std::string passwd,int connfd)
{
    connection_pool *mysql_pool=connection_pool::getinstance();
    MYSQL * mysql_connection= nullptr;
    mysql_connection=mysql_pool->dispath_connection();
    MYSQL_RES * res= nullptr;


    std::string sql_operation=std::format("select count(*) from user where username='{}';",account);
    mysql_query(mysql_connection,sql_operation.data());
    res= mysql_store_result(mysql_connection);
    if(std::string(mysql_fetch_row(res)[0])=="0")        //判断账号是否存在
    {
        mysql_pool->free_connection(std::move(mysql_connection));
        std::cout<<"账号不存在"<<std::endl;
        return ACCOUNT_NOT_EXITST;
    }


    sql_operation=std::format("select passwd from user where username='{}';",account);
    mysql_query(mysql_connection,sql_operation.data());
    res= mysql_store_result(mysql_connection);
    if(mysql_fetch_row(res)[0]!=passwd)
    {
        mysql_pool->free_connection(std::move(mysql_connection));
        std::cout<<"密码错误"<<std::endl;
        return PASSWD_ERROR;
    }
    mysql_pool->free_connection(std::move(mysql_connection));
    return NOT_ERROR;
}

int tasks::check_enroll(std::string account, std::string passwd, int connfd)
{
    connection_pool *mysql_pool=connection_pool::getinstance();
    MYSQL * mysql_connection= nullptr;
    mysql_connection=mysql_pool->dispath_connection();
    MYSQL_RES * res= nullptr;

    std::string sql_operation=std::format("select count(*) from user where username='{}';",account);
    mysql_query(mysql_connection,sql_operation.data());
    res= mysql_store_result(mysql_connection);

    if(std::string(mysql_fetch_row(res)[0])!="0")        //判断账号是否存在
    {
        mysql_pool->free_connection(std::move(mysql_connection));
        std::cout<<"账号已存在"<<std::endl;
        return ACCOUNT_EXIST;
    }

    sql_operation=std::format("insert into user values({},{});",account,passwd);
    mysql_query(mysql_connection,sql_operation.data());

    return NOT_ERROR;
}

#endif
