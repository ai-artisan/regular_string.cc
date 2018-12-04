#include <iostream>

#include "Regular.hh"

#include <memory>

class A {
public:
    A(int) {}

    A(int, int) {}
};

int main() {
    regular_string::regular::linear::Union u({});

    std::cout << "Hello, World!" << std::endl;
    return 0;
}