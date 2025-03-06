#include<../head/chatserver.h>
#include<iostream>
int main(){
    ChatServer server;

    server.Init("localhost",8080,10,"root","593509663","serverdb",3306,5);

    server.Start();
}