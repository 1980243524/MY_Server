#include<mysql/mysql.h>
#include<queue>
#include<semaphore>
#include <iostream>
class connection_pool
{
public:
    static connection_pool *getinstance();      //单例模式
    int dispath_connection();                   //为请求分配连接
    bool free_connection();                     //释放连接
    void destroy_pool();                         //销毁线程池
    void init(std::string url, std::string User, std::string PassWord, std::string DBName, int Port, unsigned int MaxConn);

private:
    unsigned int MaxConn;  //最大连接数
	unsigned int CurConn;  //当前已使用的连接数
	unsigned int FreeConn; //当前空闲的连接数

    std::queue<MYSQL *> connections;            //连接池队列
    std::binary_semaphore mutex1;       //定义互斥信号量
    std::counting_semaphore<1> num_conn;       //连接数量信号量
    connection_pool():CurConn(0),FreeConn(0), mutex1(1), num_conn(0){};
	~connection_pool();

    std::string url;			 //主机地址
    std::string Port;		 //数据库端口号
    std::string User;		 //登陆数据库用户名
    std::string PassWord;	 //登陆数据库密码
    std::string DatabaseName; //使用数据库名

};