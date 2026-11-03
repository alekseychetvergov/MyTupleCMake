#pragma once

#include <iostream>
#include <new>
#include <stdexcept>

namespace AllocatorX {
    template <typename T> class allocatorX {
    public:
        using value_type = T;
        using size_type = size_t;
        allocatorX() noexcept = default;

        // Шаблонный конструктор копирования для приведения аллокатора к другому типу
        template <typename U>
        allocatorX(const allocatorX<U>&) noexcept {}

        T* allocate(size_type size) {
            if (size == 0) return nullptr;
            // Проверка на переполнение при умножении size * sizeof(T)
            if (size > size_type(-1) / sizeof(T)) { throw std::bad_array_new_length(); }

            void* ptr = nullptr;
            size_t total_bytes = size * sizeof(T);
            if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
                ptr = ::operator new(total_bytes, std::align_val_t{ alignof(T) });
            }
            else { ptr = ::operator new(total_bytes); }
#ifdef PRINT_ALLOCATE_INFO
            std::cout << "[allocateX] " << size << " elem!\n";
#endif // PRINT_ALLOCATE_INFO
            return static_cast<T*> (ptr);
        }
        void deallocate(T* res, size_type size) noexcept {
            if (res == nullptr) return;

            size_t total_bytes = size * sizeof(T);
            if constexpr (alignof(T) > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
                ::operator delete(res, total_bytes, std::align_val_t{ alignof(T) });
            }
            else { ::operator delete(res, total_bytes); }
#ifdef PRINT_ALLOCATE_INFO
            std::cout << "[deallocateX] " << size << " elem!\n";
#endif // PRINT_ALLOCATE_INFO
        }

        template <typename U>
        bool operator == (const allocatorX<U>& allocX) const noexcept { return true; }
        template <typename U>
        bool operator != (const allocatorX<U>& allocX) const noexcept { return false; }
    };
}