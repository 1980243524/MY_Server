#ifndef UTIL_H
#define UTIL_H
#include<vector>
#include<cstring>
#include<iostream>
struct Require{
    static const size_t HEADLEN;
    struct Head{
        size_t Length;
        int operation;
    };
    Head m_head;
    std::string m_data="";

    std::string operator=(std::string message){
        m_data=message;
        m_head.Length=m_data.size();
        return m_data;
    }
    bool operator==(std::string message) const {
        return m_data==message;
    }
    std::string operator+(std::string message)const {
        return m_data+message;
    }
    int size()const  {
        return HEADLEN+m_head.Length;
    }
};
const size_t Require::HEADLEN=16;

struct Response{
    struct Head{
        size_t Length;
        int Code;
        int type;
    };

    static const size_t HEADLEN;
    Head m_head;
    std::string m_data;
    std::string operator=(std::string message){
        m_data=message;
        m_head.Length=m_data.size();
        return m_data;
    }
    bool operator==(std::string message) const {
        return m_data==message;
    }
    std::string operator+(std::string message)const {
        return m_data+message;
    }
    int size()const  {
        return HEADLEN+m_head.Length;
    }
};

const size_t Response::HEADLEN=16;

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

// Require deserialize(const std::vector<std::byte>& buffer) {
//     Require obj;
//     size_t pos = 0;

//     // 反序列化固定大小的成员
//     std::memcpy(&obj.Length, &buffer[pos], sizeof(obj.Length));
//     pos += sizeof(obj.Length);
//     std::memcpy(&obj.operation, &buffer[pos], sizeof(obj.operation));
//     pos += sizeof(obj.operation);

//     // 反序列化 std::string

//     obj.Data.assign(reinterpret_cast<const char*>(&buffer[pos]), obj.Length);

//     return obj;
// }

#endif