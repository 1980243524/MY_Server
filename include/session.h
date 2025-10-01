
#ifndef SESSION_H
#define SESSION_H
#include "msgqueue.h"

#include <array>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <cstddef>
#include <ctime>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "util.h"
auto MakeDaytimeString() -> std::string;

const int MAXLENGTH = 1024;
class ChatServer;
class Session : public std::enable_shared_from_this<Session> {
 public:
  explicit Session(boost::asio::ip::tcp::socket socket, ChatServer *server, boost::asio::io_context &io);
  auto GetSocket() -> boost::asio::ip::tcp::socket & { return socket_; }
  void Start() { DoRead(); }
  void HandleRead();
  void DoRead();
  // void SendMsg(const MsgNode &&msg) { mq_.PushMsg(msg, shared_from_this()); }
  void SendMsg(const MsgNode &msg) { mq_.PushMsg(msg, shared_from_this()); }
  void Login();
  void Signup();
  void Echo();
  void PublicChat();
  void PrivateChat();
  void MakeFriend();
  void HandleTimeOut();
  ~Session();

 private:
  boost::asio::ip::tcp::socket socket_;
  boost::asio::steady_timer timer_;
  boost::asio::strand<boost::asio::io_context::executor_type> strand_;
  std::array<std::byte, Request::HEADLEN> recv_head_;
  std::vector<std::byte> recv_body_;
  ChatServer *server_;
  MessageQueue mq_;
  int id_;
  std::string name_;
};

#endif