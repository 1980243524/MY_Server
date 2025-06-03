#include "chatserver.h"
#include <memory>
#include <mutex>
using namespace boost::asio::ip;
ChatServer::ChatServer(boost::asio::io_context &io, int port)
    : io_(io), acceptor_(io, tcp::endpoint(tcp::v4(), port)) {}

void ChatServer::Init(const std::string &DBUser, const std::string &DBPassWord, const std::string &DBName, int DBPort,
                      unsigned int DBConnNum) {
  db_pool_ = DbPool::Getinstance();
  db_pool_->Init("localhost", DBUser, DBPassWord, DBName, DBPort, DBConnNum);
}

void ChatServer::DoAccept() {
  // acceptor_.set_option(tcp::acceptor::reuse_address(true));
  acceptor_.async_accept([this](const boost::system::error_code &err, tcp::socket socket) {
    if (!err) {
      std::cout << "client connect" << std::endl;
      auto session = std::make_shared<Session>(std::move(socket), this, io_);
      session->Start();
    }
    DoAccept();
  });
}

void ChatServer::DeleteUser(int id) {
  std::lock_guard lock(mu_);
  users_.erase(id);
}

void ChatServer::AddUser(int id, std::shared_ptr<Session> &&session) {
  std::lock_guard lock(mu_);
  users_[id] = session;
}

void ChatServer::Start() { DoAccept(); }

void ChatServer::Breadcast(MsgNode msg) {}

void ChatServer::SendById(int id, MsgNode msg) {}