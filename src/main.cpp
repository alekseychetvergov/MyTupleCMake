#include <iostream>
#include <string>

#define PRINT_ALLOCATE_INFO

#include "TupleVector.hpp"
#include "AllocatorX.hpp"
#include "Example.hpp"


int main() {
    using allocatorXCh = AllocatorX::allocatorX<char>;
    myTuple::tupleVector<char, allocatorXCh, int, double, std::string, Example> tplVec1(
        42,
        3.14,
        std::string{ "Hello Tuple" },
        Example{ 7, 'b', 9.9 }
    );

    auto tplVec2(std::move(tplVec1));

    std::cout << "Int: " << tplVec2.get<0>() << "\n";
    std::cout << "Double: " << tplVec2.get<1>() << "\n";
    std::cout << "Value by type int: " << tplVec2.get<std::string>() << "\n";
    std::cout << "Value by type std::string: " << tplVec2.get<Example>() << "\n";

    std::cout << "\nTotal byte size in memory (with alignment): " << tplVec2.sizeByte_ << "\n";
    std::cout << "\nTotal byte size in memory (with alignment): " << tplVec1.sizeByte_ << "\n";
    return 0;
}