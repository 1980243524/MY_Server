#include"../head/dbPool.h"

// [first,last)
std::vector<std::map<std::string,std::string>> SqlResult::getRows(int first,int last){
    if(first<0||last>m_rowNum) {
        std::cout<<"指定的区间错误,超出数据范围"<<std::endl;
    }
    if(last==-1) last=m_rowNum;
    

    int num_fields = mysql_num_fields(m_result);
    MYSQL_FIELD *fields = mysql_fetch_fields(m_result);

    mysql_data_seek(m_result, first);
    MYSQL_ROW row;
    std::vector<std::map<std::string,std::string>> ret;
    for(int i=first;i<last;i++){
        row = mysql_fetch_row(m_result);
        std::map<std::string,std::string> temp;
        for (int j = 0; j < num_fields; j++) {
            temp[fields[j].name]=row[j];
        }
        ret.push_back(temp);
    }
    return ret;
}

int Mysql::Select(std::string&& sql,std::shared_ptr<SqlResult>& result){
    if (mysql_query(m_conn, sql.c_str())) {
        std::cerr << "SELECT失败: " << mysql_error(m_conn) << std::endl;
        return -1;
    }

    // 获取结果集
    result=std::make_shared<SqlResult>(mysql_store_result(m_conn)) ;
    if (!result.get()) {
        std::cerr << "结果集为空或错误: " << mysql_error(m_conn) << std::endl;
        return -1;
    }

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

   return std::move(conn);
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
