#ifndef DBPOOL_H
#define DBPOOL_H
#include <mysql/mysql.h>
#include <format>
#include <iostream>
#include <map>
#include <memory>
#include <queue>
#include <semaphore>
#include <vector>
int const CONNECT_SEMAPHOR_MAX = 100;

class SqlResult {
 public:
  explicit SqlResult(MYSQL_RES *result) : result_(result), row_num_(mysql_num_rows(result_)) {}

  ~SqlResult() { mysql_free_result(result_); }
  auto operator=(MYSQL_RES *res) -> SqlResult & {
    result_ = res;
    return *this;
  }
  auto operator==(void *other) -> bool { return result_ == other; }
  auto operator!() -> bool { return result_ == nullptr; }
  auto GetNum() const -> int { return row_num_; }
  auto Get() -> MYSQL_RES * { return result_; }

  auto GetRows(int first = 0, int last = -1) -> std::vector<std::map<std::string, std::string>>;

 private:
  MYSQL_RES *result_;
  const int row_num_;
};

class Mysql {
 public:
  Mysql() = default;
  Mysql(const std::string &url, const std::string &User, const std::string &PassWord, const std::string &DBName,
        const int Port) {
    conn_ = mysql_init(nullptr);
    if (conn_ == nullptr) {
      std::cout << std::format("Error:{}\n", mysql_error(conn_));
      exit(1);
    }
    conn_ = mysql_real_connect(conn_, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, nullptr, 0);
    if (conn_ == nullptr) {
      std::cout << std::format("Error:{}\n", mysql_error(conn_));
      exit(1);
    }
  }
  Mysql(Mysql &&other) noexcept {
    conn_ = other.conn_;
    other.conn_ = nullptr;
  }

  ~Mysql() {
    if (conn_ != nullptr) {
      mysql_close(conn_);
    }
  }
  auto Select(std::string &sql, std::shared_ptr<SqlResult> &result) -> int;
  auto Insert(std::string &sql) -> int;
  auto Delete(std::string &sql) -> int;
  auto Update(std::string &sql) -> int;

  auto Get() -> MYSQL * { return conn_; }
  auto operator==(void *other) -> bool { return conn_ == other; }

 private:
  MYSQL *conn_;
};

class DbPool {
 public:
  using ptr = std::shared_ptr<DbPool>;
  static auto Getinstance() -> std::shared_ptr<DbPool>;  // 单例模式
  auto DispathConnection() -> std::unique_ptr<Mysql>;    // 为请求分配连接
  void FreeConnection(std::unique_ptr<Mysql> conn);      // 释放连接
  void DestroyPool();                                    // 销毁线程池
  void Init(const std::string &url, const std::string &User, const std::string &PassWord, const std::string &DBName,
            int Port, unsigned int MaxConn);
  ~DbPool();

 private:
  DbPool() : num_free_connection_(0){};

 private:
  std::queue<std::unique_ptr<Mysql>> connections_;  // 连接池队列
  std::mutex connections_mutex_;                    // 定义互斥信号量
  std::counting_semaphore<CONNECT_SEMAPHOR_MAX> num_free_connection_;
  unsigned int max_conn_;      // 最大连接数
  std::string url_;            // 主机地址
  int port_;                   // 数据库端口号
  std::string user_;           // 登陆数据库用户名
  std::string password_;       // 登陆数据库密码
  std::string database_name_;  // 使用数据库名
};

class MysqlRAII {
 public:
  explicit MysqlRAII(std::shared_ptr<DbPool> &pool) : pool_(pool), conn_(pool->DispathConnection()) {}
  explicit MysqlRAII(std::shared_ptr<DbPool> &&pool) : pool_(std::move(pool)), conn_(pool_->DispathConnection()) {}
  ~MysqlRAII() { pool_->FreeConnection(std::move(conn_)); }
  auto Select(std::string &sql, std::shared_ptr<SqlResult> &result) -> int { return conn_->Select(sql, result); }
  auto Insert(std::string &sql) -> int { return conn_->Insert(sql); }
  auto Delete(std::string &sql) -> int { return conn_->Delete(sql); }
  auto Update(std::string &sql) -> int { return conn_->Update(sql); }

 private:
  std::shared_ptr<DbPool> pool_;
  std::unique_ptr<Mysql> conn_;
};
#endif
