#include"../head/util.h"

const size_t Require::HEADLEN=16;
const size_t Response::HEADLEN=20;
std::vector<std::byte> Serialize(const Require& obj){
    std::vector<std::byte> result(obj.size());
    std::memcpy(result.data(), &obj.m_head, Require::HEADLEN);
    std::memcpy(result.data()+Require::HEADLEN, obj.m_data.data(), obj.m_head.Length);
    return result;
}

std::vector<std::byte> Serialize(const Response& obj){
    std::vector<std::byte> result(obj.size());
    std::memcpy(result.data(), &obj, Response::HEADLEN);
    std::memcpy(result.data()+Response::HEADLEN, obj.m_data.data(), obj.m_head.Length);
    return result;
}