#ifndef DBPOOL_H
#define DBPOOL_H
#include<mysql/mysql.h>
#include<queue>
#include<semaphore>
#include <iostream>
#include<format>
#include<memory>
#include<map>
#include<vector>
int const CONNECT_SEMAPHOR_MAX=100;

class SqlResult{
public:

    SqlResult(MYSQL_RES* result):m_result(result),m_rowNum(mysql_num_rows(m_result)) {}

    ~SqlResult(){mysql_free_result(m_result);}
    SqlResult& operator=(MYSQL_RES* res){
        m_result=res;
        return *this;
    }
    bool operator==(void* other){return m_result==other;}
    bool operator!(){return !m_result;}
    int getNum()const {return m_rowNum;}
    MYSQL_RES* get(){return m_result;}

    std::vector<std::map<std::string,std::string>> getRows(int first=0,int last=-1 );
private:
    MYSQL_RES* m_result;
    const int m_rowNum;
};


class Mysql{
public:
    Mysql()=default;
    Mysql(std::string url, std::string User, std::string PassWord, std::string DBName, int Port){
        m_conn=mysql_init(nullptr);
        if (m_conn == nullptr){
            std::cout<<std::format("Error:{}\n",mysql_error(m_conn));
            exit(1);
        }
        m_conn=mysql_real_connect(m_conn, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, nullptr, 0);
        if (m_conn == nullptr){
            std::cout<<std::format("Error:{}\n",mysql_error(m_conn));
            exit(1);
        }
    }
    Mysql(Mysql&& other){
        m_conn=other.m_conn;
        other.m_conn=nullptr;
    }

    ~Mysql(){
        if(m_conn){
            mysql_close(m_conn);
        }
    }
    int Select(std::string&& sql,std::shared_ptr<SqlResult>& result);
    int Insert(std::string&& sql);
    int Delete(std::string&& sql);
    int Update(std::string&& sql);

    MYSQL* get(){return m_conn;}
    bool operator==(void * other){
        return m_conn==other;
    }
private:
    MYSQL* m_conn;
};


class DbPool
{
public:
    using ptr=std::shared_ptr<DbPool>;
    static std::shared_ptr<DbPool> getinstance();      //单例模式
    Mysql DispathConnection();                   //为请求分配连接
    bool FreeConnection(Mysql&& conn);                     //释放连接
    void destroy_pool();                         //销毁线程池
    void init(std::string url, std::string User, std::string PassWord, std::string DBName, int Port, unsigned int MaxConn);
    ~DbPool();

private:

    DbPool(): num_free_connection(0){};


private:
    std::queue<Mysql> connections;            //连接池队列
    std::mutex connections_mutex;       //定义互斥信号量
    std::counting_semaphore<CONNECT_SEMAPHOR_MAX> num_free_connection;
    unsigned int MaxConn;  //最大连接数
    std::string m_url;			 //主机地址
    std::string m_port;		 //数据库端口号
    std::string m_user;		 //登陆数据库用户名
    std::string m_passWord;	 //登陆数据库密码
    std::string m_databaseName; //使用数据库名
};

class MysqlRAII{
public:
    MysqlRAII(std::shared_ptr<DbPool> pool):m_pool(pool),m_conn(pool->DispathConnection()) {
    }
    ~MysqlRAII(){
        m_pool->FreeConnection(std::move(m_conn));
    }
    Mysql& getConn(){return m_conn;}
private:
    std::shared_ptr<DbPool> m_pool;
    Mysql m_conn;
};
#endif
