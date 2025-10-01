#ifndef UTIL_H
#define UTIL_H
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>
struct Request {
  static const size_t HEADLEN = 16;
  struct Head {
    size_t length_;
    uint32_t source_id_;
    uint32_t des_id_;
  };
  Head *head_;
  std::string data_;

  //   std::string operator=(std::string message) {
  //     m_data = message;
  //     m_head.Length = m_data.size();
  //     return m_data;
  //   }
  //   bool operator==(std::string message) const { return m_data == message; }
  //   std::string operator+(std::string message) const { return m_data + message; }
  //   auto Size() const -> size_t { return HEADLEN + m_head.Length; }
};

struct Response {
  struct Head {
    size_t length_;
    uint32_t source_id_;
    uint32_t des_id_;
    uint32_t code_;
    Head() = default;
    Head(size_t length, uint32_t sourceId, uint32_t destinationId, uint32_t code)
        : length_(length), source_id_(sourceId), des_id_(destinationId), code_(code) {}
  };
  Response() = default;
  Response(uint32_t sourceId, uint32_t destinationId, uint32_t code, const std::string &message)
      : head_(message.size(), sourceId, destinationId, code), data_(message) {}

  static const size_t HEADLEN = 20;
  Head head_;
  std::string data_;
  //   std::string operator=(std::string message) {
  //     m_data = message;
  //     m_head.Length = m_data.size();
  //     return m_data;
  //   }
  //   bool operator==(std::string message) const { return m_data == message; }
  //   std::string operator+(std::string message) const { return m_data + message; }
  auto Size() const -> int { return HEADLEN + head_.length_; }
};

auto Serialize(const Response &obj) -> std::vector<std::byte>;

#endif