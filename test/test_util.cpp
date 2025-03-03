#include"../head/util.h"
#include<iostream>
int main(){
    Require x;
    x.operation=1;
    x="hello world";
    auto result=deserialize(Serialize(x));
    std::cout<<result.Data<<std::endl;
}