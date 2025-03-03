#include<../head/chatserver.h>
#include<iostream>
int main(){
    ChatServer server;

    server.Init("127.0.0.1",8080,10);

    server.Start();
}