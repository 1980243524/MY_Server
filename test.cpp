//
// Created by xjc on 2024/6/12.
//

#include <iostream>
using namespace std;
template <typename T>
void wrapper(T &&u)
{ // 万能引用
    func(std::forward<T>(u)); // 完美转发
}

class MyClass {};

void func(MyClass& a) { std::cout << "in func(MyClass&)\n"; }
void func(const MyClass& a) { std::cout << "in func(const MyClass&)\n"; }
void func(MyClass&& a) { std::cout << "in func(MyClass &&)\n"; }

int main(void) {
    MyClass a;
    const MyClass b;

    func(a);
    func(b);
    func(MyClass());

    std::cout << "----- Wrapper ------\n";
    wrapper(a);
    wrapper(b);
    wrapper(MyClass());

    return 0;
}
