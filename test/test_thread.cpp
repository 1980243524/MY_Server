#include"../head/ThreadPool.h"

int count=0;
void func(){
    for(int i=0;i<100000;i++)count++;
}
int main(){
    ThreadPool pool(5);
    for(int i=0;i<10;i++)
        pool.addTask(func);
    
    std::cout<<count;
}