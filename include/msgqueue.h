#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include <algorithm>
#include <boost/asio/ip/tcp.hpp>
#include <cstddef>
#include <cstring>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <utility>
#include <vector>

class Session;
struct MsgNode {
  explicit MsgNode(std::vector<std::byte> &&data) : total_len_(data.size()), data_(std::move(data)) {}
  explicit MsgNode(const std::vector<std::byte> &data) : total_len_(data.size()), data_(data) {}
  // MsgNode(char *data, size_t size) : total_len_(size), offset_(0), data_(size) {
  //   std::copy(data, data + size, reinterpret_cast<char *>(data_.data()));
  // }

  explicit MsgNode(uint32_t data) : total_len_(sizeof(uint32_t)), data_(sizeof(uint32_t)) {
    std::copy(&data, &data + sizeof(uint32_t), reinterpret_cast<uint32_t *>(data_.data()));
  }
  MsgNode(const MsgNode &other) = default;

  MsgNode(MsgNode &&other) noexcept
      : total_len_(other.total_len_), offset_(other.offset_), data_(std::move(other.data_)) {}

  ~MsgNode() = default;
  auto Data() -> std::byte * { return data_.data() + offset_; }
  void Send(size_t size) { offset_ += size; }
  auto RecvSize() -> size_t { return total_len_ - offset_; }
  size_t total_len_;
  size_t offset_ = 0;
  std::vector<std::byte> data_;
};

class MessageQueue {
 public:
  MessageQueue() = default;
  void PushMsg(MsgNode msg, std::shared_ptr<Session> &&session);

 private:
  void HandleSend(const std::shared_ptr<Session> &session, size_t size);
  std::queue<MsgNode> messages_;
  std::mutex mu_;
};
#endif