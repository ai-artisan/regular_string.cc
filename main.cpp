#include <iostream>

#include "Regular.hh"

#include <memory>

class A {
public:
    virtual void f() = 0;
};

class B : public A {
public:
    virtual void f() {
        std::cout << 1234 << '\n';
    }
};

int main() {
    std::shared_ptr<A> a = std::make_shared<B>();
    ((B*)(&*a))->f();

    std::cout << "Hello, World!" << std::endl;
    return 0;
}