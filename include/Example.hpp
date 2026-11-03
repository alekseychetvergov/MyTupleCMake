#pragma once

#include <iostream>    

struct Example {
    int i = 1;
    char ch = 'a';
    double d = 7.0;
    Example();
    Example(int ii, char chch, double dd);
    Example(const Example& l);
    Example(Example&& r) noexcept;
    ~Example();
};
std::ostream& operator << (std::ostream& os, const Example& ex);
