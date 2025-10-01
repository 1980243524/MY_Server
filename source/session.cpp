#include "session.h"
#include <boost/asio/strand.hpp>
#include <chrono>
#include <cstddef>
#include <format>
#include <vector>
#include "chatserver.h"
#include "msgqueue.h"
#include "timer.h"
#include "util.h"

using namespace boost::asio::ip;

auto MakeDaytimeString() -> std::string {
  time_t now = time(nullptr);
  return ctime(&now);
}

Session::Session(tcp::socket socket, ChatServer *server, boost::asio::io_context &io)
    : socket_(std::move(socket)),
      id_(-1),
      server_(server),
      strand_(boost::asio::make_strand(io)),
      timer_(io, std::chrono::minutes(10)) {
  timer_.async_wait(boost::asio::bind_executor(strand_, [this](const boost::system::error_code &ec) {
    if (!ec) {
      this->HandleTimeOut();
    }
  }));
}

void Session::DoRead() {
  auto self(shared_from_this());

  socket_.async_receive(
      boost::asio::buffer(recv_head_, Request::HEADLEN),
      boost::asio::bind_executor(strand_, [self](boost::system::error_code ec, size_t length) {
        if (!ec) {
          // 处理接收到的数据
          self->timer_.expires_after(std::chrono::minutes(10));
          self->timer_.async_wait(
              boost::asio::bind_executor(self->strand_, [self](const boost::system::error_code &ec) {
                if (!ec) {
                  self->HandleTimeOut();
                }
              }));
          auto head = reinterpret_cast<Request::Head *>(self->recv_head_.data());
          self->recv_body_.resize(head->length_);
          self->GetSocket().async_receive(
              boost::asio::buffer(self->recv_body_, head->length_),
              boost::asio::bind_executor(self->strand_, [self](boost::system::error_code ec, size_t length) {
                if (!ec) {
                  // 处理接收到的数据
                  self->HandleRead();
                }
              }));
        }
      }));
}

void Session::HandleRead() {
  auto head = reinterpret_cast<Request::Head *>(recv_head_.data());
  if (head->des_id_ < 100) {
    switch (head->des_id_) {
      case 1:
        Login();
        break;
      case 2:
        Signup();
        break;
      case 3:
        Echo();
        break;
      case 4:
        PublicChat();
        break;
      case 5:
        PrivateChat();
        break;
      case 6:
        MakeFriend();
        break;
    }
  } else {
    PrivateChat();
  }

  DoRead();
}

void Session::Login() {
  Request request;
  request.head_ = reinterpret_cast<Request::Head *>(recv_head_.data());
  request.data_ = std::string(reinterpret_cast<char *>(recv_body_.data()), recv_body_.size());

  auto mid = request.data_.find('\\');
  std::string account = request.data_.substr(0, mid);
  std::string passwd = request.data_.substr(mid + 1);

  MysqlRAII mysql_conn(DbPool::Getinstance());

  std::shared_ptr<SqlResult> result;

  std::string sql = std::format("select * from user where username='{}'", account);
  int ret = mysql_conn.Select(sql, result);

  if (ret < 0) {
    std::cout << "查询语句执行失败" << std::endl;
    Response response(1, 1, 500, "unknow error\n");
    SendMsg(MsgNode(Serialize(response)));
    // send(connfd, Serialize(response).data(), response.size(), 0);
    return;
  }
  if (result->GetNum() < 1) {
    Response response(1, 1, 500, "user not exist\n");
    SendMsg(MsgNode(Serialize(response)));
    // send(connfd, Serialize(response).data(), response.size(), 0);
    return;
  }

  auto userinfo = result->GetRows(0, 1)[0];

  if (userinfo["passwd"] != passwd) {
    Response response(1, 1, 500, "password is uncorrect\n");
    SendMsg(MsgNode(Serialize(response)));
    // send(connfd, Serialize(response).data(), response.size(), 0);
    return;
  }
  uint32_t curid = std::stoi(userinfo["id"]);

  Response response(1, curid, 200, "successfully login\n");
  SendMsg(MsgNode(Serialize(response)));
  // send(connfd, Serialize(response).data(), response.size(), 0);

  sql = std::format(
      "select relation.user2_id as id, user.username as name \
        from relation inner join user  on user.id=relation.user2_id where user1_id='{}';",
      curid);
  mysql_conn.Select(sql, result);
  auto users = result->GetRows();
  uint32_t n = result->GetNum();
  SendMsg(MsgNode(n));
  // send(connfd, &n, sizeof(uint32_t), 0);
  for (auto user : users) {
    Response response(std::stoi(user["id"]), curid, 200, user["name"]);
    SendMsg(MsgNode(Serialize(response)));
    // send(connfd, Serialize(response).data(), response.size(), 0);
  }

  id_ = curid;
  name_ = userinfo["username"];
  server_->AddUser(id_, shared_from_this());
}

void Session::Signup() {
  Request request;
  request.head_ = reinterpret_cast<Request::Head *>(recv_head_.data());
  request.data_ = std::string(reinterpret_cast<char *>(recv_body_.data()), recv_body_.size());

  auto mid = request.data_.find('\\');
  std::string account = request.data_.substr(0, mid);
  std::string passwd = request.data_.substr(mid + 1);

  MysqlRAII mysql_conn(DbPool::Getinstance());

  std::shared_ptr<SqlResult> result;

  std::string sql = std::format("select * from user where username='{}';", account);
  int ret = mysql_conn.Select(sql, result);

  if (ret < 0) {
    std::cout << "查询语句执行失败" << std::endl;
    Response response(2, 2, 500, "unknow error\n");
    SendMsg(MsgNode(Serialize(response)));
    return;
  }
  if (result->GetNum() > 0) {
    Response response(2, 2, 500, "user exists\n");
    SendMsg(MsgNode(Serialize(response)));
    return;
  }
  sql = std::format("insert into user (username,passwd) values('{}','{}');", account, passwd);
  ret = mysql_conn.Insert(sql);
  if (ret < 0) {
    std::cout << "插入用户信息失败" << std::endl;
    Response response(2, 2, 500, "unknow error\n");
    SendMsg(MsgNode(Serialize(response)));
    return;
  }
  Response response(2, 2, 200, "user successfully sign up\n");
  SendMsg(MsgNode(Serialize(response)));
}

void Session::Echo() {
  auto head = reinterpret_cast<Request::Head *>(recv_head_.data());
  std::string body(reinterpret_cast<char *>(recv_body_.data()), recv_body_.size());
  Response response(3, head->source_id_, 200, std::format("回声| {}: {}\n", name_, body));
  SendMsg(MsgNode(Serialize(response)));
  // int ret = write(connfd, Serialize(response).data(), response.size());
}

Session::~Session() {
  server_->DeleteUser(id_);
  std::cout << id_ << "会话关闭" << std::endl;
}

void Session::HandleTimeOut() {
  socket_.close();
  std::cout << std::format("{} client : No operation has been conducted for a long time ", id_) << std::endl;
}

void Session::MakeFriend() {
  auto head = reinterpret_cast<Request::Head *>(recv_head_.data());
  std::string name(reinterpret_cast<char *>(recv_body_.data()), recv_body_.size());
  MysqlRAII mysql_conn(DbPool::Getinstance());

  std::shared_ptr<SqlResult> result;
  std::string sql = std::format("select * from user where username='{}';", name);
  int ret = mysql_conn.Select(sql, result);

  if (ret < 0) {
    std::cout << "查询语句执行失败" << std::endl;
    Response response(6, id_, 500, "unknow error\n");
    SendMsg(MsgNode(Serialize(response)));
    // send(connfd, Serialize(response).data(), response.size(), 0);
    return;
  }
  if (result->GetNum() < 1) {
    Response response(6, id_, 500, "user not exist\n");
    SendMsg(MsgNode(Serialize(response)));
    // send(connfd, Serialize(response).data(), response.size(), 0);
    return;
  }

  auto friend_id = result->GetRows(0, 1)[0]["id"];

  sql = std::format("select * from relation where user1_id={} and user2_id='{}';", id_, friend_id);

  ret = mysql_conn.Select(sql, result);

  if (ret < 0) {
    std::cout << "查询语句执行失败" << std::endl;
    Response response(6, id_, 500, "unknow error\n");
    SendMsg(MsgNode(Serialize(response)));
    // send(connfd, Serialize(response).data(), response.size(), 0);
    return;
  }
  if (result->GetNum() > 0) {
    Response response(6, id_, 500, "already existed friend\n");
    SendMsg(MsgNode(Serialize(response)));
    // send(connfd, Serialize(response).data(), response.size(), 0);
    return;
  }

  sql = std::format("insert into relation values('{}','{}'),('{}','{}');", id_, friend_id, friend_id, id_);
  ret = mysql_conn.Insert(sql);

  Response response(6, id_, 200, friend_id);
  SendMsg(MsgNode(Serialize(response)));
  // send(connfd, Serialize(response).data(), response.size(), 0);
}

void Session::PublicChat() {
  std::string data(reinterpret_cast<char *>(recv_body_.data()), recv_body_.size());

  Response response(4, id_, 200, std::format("公共频道| {}: {}\n", name_, data));
  server_->Breadcast(MsgNode(Serialize(response)));
  // send(user.second, Serialize(response).data(), response.size(), 0);
}

void Session::PrivateChat() {
  auto head = reinterpret_cast<Request::Head *>(recv_head_.data());
  std::string data(reinterpret_cast<char *>(recv_body_.data()), recv_body_.size());
  MysqlRAII mysql_conn(DbPool::Getinstance());

  auto user1 = head->source_id_;
  auto user2 = head->source_id_;

  std::shared_ptr<SqlResult> result;

  std::string sql = std::format("select * from relation where user1_id='{}' and user2_id='{}' ;", user1, user2);

  int ret = mysql_conn.Select(sql, result);

  if (ret < 0) {
    std::cout << "查询语句执行失败" << std::endl;
    Response response(5, user1, 500, "unknow error\n");
    SendMsg(MsgNode(Serialize(response)));
    // send(connfd, Serialize(response).data(), response.size(), 0);
    return;
  }
  if (result->GetNum() < 1) {
    Response response(5, user1, 500, "the user is not your friend\n");
    SendMsg(MsgNode(Serialize(response)));
    // send(connfd, Serialize(response).data(), response.size(), 0);
    return;
  }

  Response response(user1, user2, 200, std::format("私聊| {}: {}", name_, data));
  server_->SendById(user1, MsgNode(Serialize(response)));
  server_->SendById(user2, MsgNode(Serialize(response)));
  //     send(m_id_fdMap[user2], Serialize(response).data(), response.size(), 0);
  // send(connfd, Serialize(response).data(), response.size(), 0);
}