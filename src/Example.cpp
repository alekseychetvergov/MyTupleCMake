#include "Example.hpp"
#include <utility>


Example::Example() {
#ifdef PRINT_ALLOCATE_INFO
    std::cout << "[DefCtor {}]...[" << *this << "]\n";
#endif //PRINT_ALLOCATE_INFO
}

Example::Example(int ii, char chch, double dd) : i(ii), ch(chch), d(dd)
{
#ifdef PRINT_ALLOCATE_INFO
    std::cout << "[IniCtor {}]...[" << *this << "]\n";
#endif //PRINT_ALLOCATE_INFO
}

Example::Example(const Example& l) : i(l.i), ch(l.ch), d(l.d) {
#ifdef PRINT_ALLOCATE_INFO
    std::cout << "[CopyCtor {}]...[" << *this << "]\n";
#endif //PRINT_ALLOCATE_INFO
}

Example::Example(Example&& r) noexcept : i(std::move(r.i)), ch(std::move(r.ch)), d(std::move(r.d)) {
#ifdef PRINT_ALLOCATE_INFO
    std::cout << "[MoveCtor {}]...[" << *this << "]\n";
#endif //PRINT_ALLOCATE_INFO
}
Example::~Example() {
#ifdef PRINT_ALLOCATE_INFO
    std::cout << "[Dtor]...[" << *this << "]\n";
#endif //PRINT_ALLOCATE_INFO
}

std::ostream& operator << (std::ostream& os, const Example& ex) {
    return (os << "Ex: (" << ex.i << ", " << ex.ch << ", " << ex.d << ") ");
}