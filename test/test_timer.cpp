#include"../head/timer.h"
#include<iostream>
#include<format>
int count=0;
void func(){
    std::cout<<std::format("定时{}",count++)<<std::endl;
}
int main(){
    std::cout<<"开始";
    Timer t;
    t.SetEvent(func);
    t.Tick(1);
    sleep(2);
    t.Reset(2);
    while(1);
}