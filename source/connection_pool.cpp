#include"../head/connection_pool.h"

#include <format>

connection_pool *connection_pool::getinstance()
{
    static connection_pool connPool;
    return &connPool;
}
void connection_pool::init(std::string url, std::string User, std::string PassWord, std::string DBName, int Port, unsigned int MaxConn)
{
    this->url = url;
    this->Port = Port;
    this->User = User;
    this->PassWord = PassWord;
    this->DatabaseName = DBName;
    this->MaxConn=MaxConn;

    for(int i=0;i<MaxConn;i++)
    {
        MYSQL *con = nullptr;
        con = mysql_init(con);
        if (con == nullptr)
        {
            std::cout<<std::format("Error:{}\n",mysql_error(con));
            exit(1);
        }
        con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, nullptr, 0);
        if (con == nullptr)
        {
            std::cout<<std::format("Error:{}\n",mysql_error(con));
            exit(1);
        }
        connections_mutex.acquire();
        connections.push(con);
        connections_mutex.release();

        num_free_connection.release();
    }
}

MYSQL* connection_pool::dispath_connection()
{
   MYSQL* m= nullptr;

   num_free_connection.acquire();   //空闲连接信号量P操作

   connections_mutex.acquire();     //连接队列互斥
   m=connections.front();
   connections.pop();
   connections_mutex.release();     //连接队列互斥结束

   return m;

}
connection_pool::~connection_pool()
{

}