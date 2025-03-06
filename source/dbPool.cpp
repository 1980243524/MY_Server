#include"../head/dbPool.h"

int Mysql::Select(std::string&& sql,MYSQL_RES* result){
    if (mysql_query(m_conn, sql.c_str())) {
        std::cerr << "SELECT失败: " << mysql_error(m_conn) << std::endl;
        return -1;
    }

    // 获取结果集
    result = mysql_store_result(m_conn);
    if (!result) {
        std::cerr << "结果集为空或错误: " << mysql_error(m_conn) << std::endl;
        return -1;
    }

    // // 获取字段数量
    // int num_fields = mysql_num_fields(result);
    // std::cout << "字段数量: " << num_fields << std::endl;

    // // 遍历结果行
    // MYSQL_ROW row;
    // while ((row = mysql_fetch_row(result))) {
    //     // 按字段索引获取数据（0-based）
    //     std::cout << "ID: " << row[0]
    //               << ", Name: " << row[1]
    //               << ", Age: " << row[2] << std::endl;
    // }

    // // 释放结果集
    // mysql_free_result(result);
    return 0;
}

int Mysql::Insert(std::string&& sql){
    // 直接执行SQL（不防注入，仅演示）
    if (mysql_query(m_conn, sql.c_str())) {
        std::cerr << "INSERT失败: " << mysql_error(m_conn) << std::endl;
        return -1;
    }
    // 获取受影响的行数
    std::cout << "插入成功，影响行数: " << mysql_affected_rows(m_conn) << std::endl;
    return 0;
}

int Mysql::Delete(std::string&& sql){
    if (mysql_query(m_conn, sql.c_str())) {
        std::cerr << "DELETE失败: " << mysql_error(m_conn) << std::endl;
        return -1;
    }
    std::cout << "删除成功，影响行数: " << mysql_affected_rows(m_conn) << std::endl;
    return 0;
}

int Mysql::Update(std::string&& sql){
    if (mysql_query(m_conn, sql.c_str())) {
        std::cerr << "UPDATE失败: " << mysql_error(m_conn) << std::endl;
        return -1;
    }
    std::cout << "更新成功，影响行数: " << mysql_affected_rows(m_conn) << std::endl;
    return 0;
}
std::shared_ptr<DbPool> DbPool::getinstance(){
    static std::shared_ptr<DbPool> I(new DbPool);
    return I;
}

void DbPool::init(std::string url, std::string User, std::string PassWord, std::string DBName, int Port, unsigned int MaxConn)
{
    m_url = url;
    m_port = Port;
    m_user= User;
    m_passWord= PassWord;
    m_databaseName = DBName;
    this->MaxConn=MaxConn;

    for(int i=0;i<MaxConn;i++)
    {
        
        
        std::lock_guard lock(connections_mutex);
        connections.push(Mysql(url,User,PassWord,DBName,Port));

        num_free_connection.release();
    }
}

Mysql DbPool::DispathConnection()
{

   
   num_free_connection.acquire();   //空闲连接信号量P操作

   std::lock_guard lock(connections_mutex);
   Mysql conn(std::move(connections.front()));
   connections.pop();

   return conn;
}

bool DbPool::FreeConnection(Mysql&& conn)
{
    std::lock_guard lock(connections_mutex);
    connections.push(std::forward<Mysql>(conn));

    num_free_connection.release();
    return true;
}
DbPool::~DbPool()
{

}
