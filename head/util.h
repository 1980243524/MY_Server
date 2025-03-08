#ifndef UTIL_H
#define UTIL_H
#include<vector>
#include<cstring>
#include<iostream>
#include<cstdint>
struct Require{
    static const size_t HEADLEN;
    struct Head{
        size_t Length;
        uint32_t SourceId;
        uint32_t DestinationId;
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


struct Response{
    struct Head{
        size_t Length;
        uint32_t SourceId;
        uint32_t DestinationId;
        uint32_t Code;
        Head()=default;
        Head(size_t length,uint32_t sourceId,uint32_t destinationId,uint32_t code)
                            :Length(length),SourceId(sourceId),DestinationId(destinationId),Code(code) {}
    };
    Response()=default;
    Response(uint32_t sourceId,uint32_t destinationId,uint32_t code,std::string message)
                            :m_head(message.size(),sourceId,destinationId,code),m_data(message) {}
                            
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

std::vector<std::byte> Serialize(const Require& obj);

std::vector<std::byte> Serialize(const Response& obj);

#endif