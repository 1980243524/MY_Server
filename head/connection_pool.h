#include<mysql/mysql.h>
#include<queue>
#include<semaphore>
#include <iostream>
int const CONNECT_SEMAPHOR_MAX=100;
class connection_pool
{
public:
    static connection_pool *getinstance();      //单例模式
    MYSQL* dispath_connection();                   //为请求分配连接
    bool free_connection(MYSQL* (&&m));                     //释放连接
    void destroy_pool();                         //销毁线程池
    void init(std::string url, std::string User, std::string PassWord, std::string DBName, int Port, unsigned int MaxConn);

private:
    unsigned int MaxConn;  //最大连接数

    std::queue<MYSQL *> connections;            //连接池队列
    std::binary_semaphore connections_mutex;       //定义互斥信号量
    std::counting_semaphore<CONNECT_SEMAPHOR_MAX> num_free_connection;
    connection_pool(): connections_mutex(1),num_free_connection(0){};
	~connection_pool();

    std::string url;			 //主机地址
    std::string Port;		 //数据库端口号
    std::string User;		 //登陆数据库用户名
    std::string PassWord;	 //登陆数据库密码
    std::string DatabaseName; //使用数据库名


};