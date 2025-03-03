#ifndef UTIL_H
#define UTIL_H
#include<vector>
#include<cstring>
#include<iostream>
struct Require{
    int Length;
    int operation;
    std::string Data="";

    std::string operator=(std::string message){
        Data=message;
        Length=Data.size();
        return Data;
    }
    bool operator==(std::string message) const {
        return Data==message;
    }
    std::string operator+(std::string message)const {
        return Data+message;
    }
    int size()const  {
        return sizeof(int)*2+Length;
    }
};


std::vector<std::byte> Serialize(const Require& obj){
    std::vector<std::byte> result(obj.size());
    std::memcpy(result.data(), &obj, 8);
    std::memcpy(result.data()+8, obj.Data.data(), obj.Length);
    return result;
}

Require deserialize(const std::vector<std::byte>& buffer) {
    Require obj;
    size_t pos = 0;

    // 反序列化固定大小的成员
    std::memcpy(&obj.Length, &buffer[pos], sizeof(obj.Length));
    pos += sizeof(obj.Length);
    std::memcpy(&obj.operation, &buffer[pos], sizeof(obj.operation));
    pos += sizeof(obj.operation);

    // 反序列化 std::string

    obj.Data.assign(reinterpret_cast<const char*>(&buffer[pos]), obj.Length);

    return obj;
}

#endif