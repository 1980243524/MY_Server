#include "util.h"

// auto Serialize(const Request &obj) -> std::vector<std::byte> {
//   std::vector<std::byte> result(obj.Size());
//   std::memcpy(result.data(), &obj.head_, Request::HEADLEN);
//   std::memcpy(result.data() + Request::HEADLEN, obj.m_data.data(), obj.head_.Length);
//   return result;
// }

auto Serialize(const Response &obj) -> std::vector<std::byte> {
  std::vector<std::byte> result(obj.Size());
  std::memcpy(result.data(), &obj.head_, Response::HEADLEN);
  std::memcpy(result.data() + Response::HEADLEN, obj.data_.data(), obj.head_.length_);
  return result;
}