#include"../head/timer.h"
#include<iostream>
#include<format>
int count=0;
void func(){
    std::cout<<std::format("定时{}",count++)<<std::endl;
}
int main(){
    std::cout<<"开始"<<std::endl;
    Timer t;
    t.SetEvent(func);
    t.Start(4);
    sleep(6);
    //t.Stop();
    t.Tick(1000);

    sleep(5);
}