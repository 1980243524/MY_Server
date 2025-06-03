#ifndef CHATSERVER_H
#define CHATSERVER_H
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include "dbPool.h"
#include "msgqueue.h"
#include "session.h"
#include "threadPool.h"
#include "user.h"
#include "util.h"
#define MAX_EVENT_NUMBER 10000  // 最大事件数
int const OPEN_MAX = 1000;

class ChatServer {
 public:
  explicit ChatServer(boost::asio::io_context &io, int port);
  ~ChatServer() = default;
  void Start();
  void Init(const std::string &DBUser, const std::string &DBPassWord, const std::string &DBName, int DBPort,
            unsigned int DBConnNum);
  void Stop();
  void Listen();
  void DoAccept();
  void DeleteUser(int id);
  void AddUser(int id, std::shared_ptr<Session> &&session);
  void Breadcast(MsgNode msg);
  void SendById(int id, MsgNode msg);

 private:
  DbPool::ptr db_pool_;
  int port_;
  boost::asio::io_context &io_;
  boost::asio::ip::tcp::acceptor acceptor_;
  std::unordered_map<int, std::weak_ptr<Session>> users_;
  std::mutex mu_;
};

#endif