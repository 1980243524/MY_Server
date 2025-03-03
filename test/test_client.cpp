#include <iostream>
#include <string>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include<vector>
#include"../head/util.h"
const int BUFFER_SIZE = 1024;

std::istream& operator>>(std::istream& in,Require& r){

    in>>r.Data;
    r.Length=r.Data.size();
    return in;
}
int main() {
    // 1. 创建套接字
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        std::cerr << "创建套接字失败: " << strerror(errno) << std::endl;
        return 1;
    }

    // 2. 配置服务器地址
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080); // 服务器端口
    
    // 将IP地址从字符串转换为二进制格式
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        std::cerr << "无效的地址/地址不支持: " << strerror(errno) << std::endl;
        close(client_socket);
        return 1;
    }

    // 3. 连接服务器
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "连接失败: " << strerror(errno) << std::endl;
        close(client_socket);
        return 1;
    }

    std::cout << "成功连接到服务器!" << std::endl;

    
    Require input;
    input.operation=1;
    std::vector<std::byte> buffer;
    
    // 4. 通信循环
    while (true) {
        
        // 获取用户输入
        std::cout << "输入要发送的消息 (输入 'exit' 退出): ";
        std::cin>>input;


        if (input == "exit") {
            break;
        }
        buffer=Serialize(input);
        // 发送数据到服务器
        ssize_t bytes_sent = send(client_socket, buffer.data(), input.size(), 0);
        if (bytes_sent == -1) {
            std::cerr << "发送失败: " << strerror(errno) << std::endl;
            break;
        }
        // 接收服务器响应
        std::vector<std::byte> get(1024);
        ssize_t bytes_received = recv(client_socket, get.data(), BUFFER_SIZE - 1, 0);
        if (bytes_received == -1) {
            std::cerr << "接收失败: " << strerror(errno) << std::endl;
            break;
        } else if (bytes_received == 0) {
            std::cout << "服务器关闭了连接" << std::endl;
            break;
        }

        // 处理接收数据
        std::cout << "服务器响应: " << reinterpret_cast<const char*>(get.data()) << std::endl;
    }

    // 5. 关闭套接字
    close(client_socket);
    std::cout << "连接已关闭" << std::endl;
    return 0;
}