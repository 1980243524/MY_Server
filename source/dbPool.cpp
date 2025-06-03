#include "dbPool.h"
#include <memory>

// [first,last)
auto SqlResult::GetRows(int first, int last) -> std::vector<std::map<std::string, std::string>> {
  if (first < 0 || last > row_num_) {
    std::cout << "指定的区间错误,超出数据范围" << std::endl;
  }
  if (last == -1) {
    last = row_num_;
  }

  int num_fields = mysql_num_fields(result_);
  MYSQL_FIELD *fields = mysql_fetch_fields(result_);

  mysql_data_seek(result_, first);
  MYSQL_ROW row;
  std::vector<std::map<std::string, std::string>> ret;
  for (int i = first; i < last; i++) {
    row = mysql_fetch_row(result_);
    std::map<std::string, std::string> temp;
    for (int j = 0; j < num_fields; j++) {
      temp[fields[j].name] = row[j];
    }
    ret.push_back(temp);
  }
  return ret;
}

auto Mysql::Select(std::string &sql, std::shared_ptr<SqlResult> &result) -> int {
  if (mysql_query(conn_, sql.c_str()) != 0) {
    std::cerr << "SELECT失败: " << mysql_error(conn_) << std::endl;
    return -1;
  }

  // 获取结果集
  result = std::make_shared<SqlResult>(mysql_store_result(conn_));
  if (result == nullptr) {
    std::cerr << "结果集为空或错误: " << mysql_error(conn_) << std::endl;
    return -1;
  }

  return 0;
}

auto Mysql::Insert(std::string &sql) -> int {
  // 直接执行SQL（不防注入，仅演示）
  if (mysql_query(conn_, sql.c_str()) != 0) {
    std::cerr << "INSERT失败: " << mysql_error(conn_) << std::endl;
    return -1;
  }
  // 获取受影响的行数
  std::cout << "插入成功，影响行数: " << mysql_affected_rows(conn_) << std::endl;

  return 0;
}

auto Mysql::Delete(std::string &sql) -> int {
  if (mysql_query(conn_, sql.c_str()) != 0) {
    std::cerr << "DELETE失败: " << mysql_error(conn_) << std::endl;
    return -1;
  }
  std::cout << "删除成功，影响行数: " << mysql_affected_rows(conn_) << std::endl;
  return 0;
}

auto Mysql::Update(std::string &sql) -> int {
  if (mysql_query(conn_, sql.c_str()) != 0) {
    std::cerr << "UPDATE失败: " << mysql_error(conn_) << std::endl;
    return -1;
  }
  std::cout << "更新成功，影响行数: " << mysql_affected_rows(conn_) << std::endl;
  return 0;
}
auto DbPool::Getinstance() -> std::shared_ptr<DbPool> {
  static std::shared_ptr<DbPool> i(new DbPool);
  return i;
}

void DbPool::Init(const std::string &url, const std::string &User, const std::string &PassWord,
                  const std::string &DBName, const int Port, const unsigned int MaxConn) {
  url_ = url;
  port_ = Port;
  user_ = User;
  password_ = PassWord;
  database_name_ = DBName;
  this->max_conn_ = MaxConn;

  for (int i = 0; i < MaxConn; i++) {
    std::lock_guard lock(connections_mutex_);
    connections_.push(std::make_unique<Mysql>(url, User, PassWord, DBName, Port));
    num_free_connection_.release();
  }
}

auto DbPool::DispathConnection() -> std::unique_ptr<Mysql> {
  num_free_connection_.acquire();  // 空闲连接信号量P操作

  std::lock_guard lock(connections_mutex_);

  std::unique_ptr<Mysql> conn(std::move(connections_.front()));
  connections_.pop();

  return conn;
}

void DbPool::FreeConnection(std::unique_ptr<Mysql> conn) {
  std::lock_guard lock(connections_mutex_);
  connections_.emplace(std::move(conn));

  num_free_connection_.release();
}
DbPool::~DbPool() { DestroyPool(); }

void DbPool::DestroyPool() {
  std::lock_guard lock(connections_mutex_);
  while (!connections_.empty()) {
    connections_.pop();
  }
}