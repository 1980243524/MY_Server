#include "msgqueue.h"
#include <cstddef>
#include <mutex>
#include "session.h"

void MessageQueue::PushMsg(MsgNode msg, std::shared_ptr<Session> &&session) {
  std::lock_guard lock(mu_);
  if (!messages_.empty()) {
    messages_.push(std::move(msg));
    return;
  }
  messages_.push(std::move(msg));

  session->GetSocket().async_send(boost::asio::buffer(messages_.front().Data(), messages_.front().total_len_),
                                  [session, this](boost::system::error_code ec, size_t send_len) {
                                    if (!ec) {
                                      this->HandleSend(session, send_len);
                                    }
                                  });
}

void MessageQueue::HandleSend(const std::shared_ptr<Session> &session, size_t size) {
  std::lock_guard lock(this->mu_);
  this->messages_.front().Send(size);
  if (this->messages_.front().RecvSize() == 0) {
    this->messages_.pop();
  }
  if (!messages_.empty()) {
    session->GetSocket().async_send(
        boost::asio::buffer(messages_.front().Data(), messages_.front().total_len_),
        [session, this](boost::system::error_code ec, size_t send_len) { this->HandleSend(session, send_len); });
  }
}