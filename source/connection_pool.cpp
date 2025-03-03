#include"../head/connection_pool.h"

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
        
        std::lock_guard lock(connections_mutex);
        connections.push(con);

        num_free_connection.release();
    }
}

MYSQL* connection_pool::dispath_connection()
{
   MYSQL *m;

   num_free_connection.acquire();   //空闲连接信号量P操作

   std::lock_guard lock(connections_mutex);
   m=std::move(connections.front());
   connections.pop();

   return m;
}

bool connection_pool::free_connection(MYSQL* (&&m))
{
    std::lock_guard lock(connections_mutex);
    connections.push(m);
    num_free_connection.release();

    return true;
}
connection_pool::~connection_pool()
{

}
