#pragma once

#include <utility>
#include <memory>

#include "TupleMeta.hpp"

namespace myTuple
{
    template <typename T = char, typename allocator = std::allocator<T>>
    class tupleVectorBuff
    {
    public:
        using size_type = size_t;
        using value_type = T;
        using traits_type = std::allocator_traits<allocator>;
        using allocator_type = allocator;

    public:
        value_type *m_buffer_ = nullptr;
        size_type capacityByte_ = 0;
        allocator_type alloc_;

    public:
        tupleVectorBuff(const allocator_type &alloc = allocator_type()) : m_buffer_(nullptr), capacityByte_(0), alloc_(alloc) {}
        tupleVectorBuff(size_type size, const allocator_type &alloc = allocator_type())
            : capacityByte_(0), alloc_(alloc)
        {
            if (size > 0)
            {
                allocate(size);
            }
        }
        tupleVectorBuff(const tupleVectorBuff &) = delete;
        tupleVectorBuff &operator=(const tupleVectorBuff &) = delete;

        tupleVectorBuff(tupleVectorBuff &&other) noexcept
            : m_buffer_(other.m_buffer_), capacityByte_(other.capacityByte_), alloc_(std::move(other.alloc_))
        {
            other.m_buffer_ = nullptr;
            other.capacityByte_ = 0;
        }

        tupleVectorBuff &operator=(tupleVectorBuff &&other) noexcept
        {
            if (this != &other)
            {
                deallocate(capacityByte_); // Освобождаем свою старую память
                m_buffer_ = other.m_buffer_;
                capacityByte_ = other.capacityByte_;
                alloc_ = std::move(other.alloc_);

                other.m_buffer_ = nullptr;
                other.capacityByte_ = 0;
            }
            return *this;
        }

        void allocate(size_type size)
        {
            if (m_buffer_ != nullptr)
            {
                deallocate(capacityByte_);
            }
            m_buffer_ = traits_type::allocate(alloc_, size);
            capacityByte_ = size;
        }
        void deallocate(size_type size) noexcept
        {
            if (m_buffer_ != nullptr)
            {
                traits_type::deallocate(alloc_, m_buffer_, size);
                m_buffer_ = nullptr;
                capacityByte_ = 0;
            }
        }
        ~tupleVectorBuff() noexcept {}
    };
    //****************************************************************************************************************
    template <typename T = char, typename allocator = std::allocator<T>, typename... Types>
    class tupleVector : public tupleVectorBuff<char, allocator>
    {
    private:
        // Реверс последовательности для уничтожения элементов в обратном порядке
        template <std::size_t... Is>
        static constexpr auto reverse_sequence(std::index_sequence<Is...>)
            -> std::index_sequence<(sizeof...(Is) - 1 - Is)...>
        {
            return {};
        }
        template <std::size_t N>
        using make_index_sequence_reverse = decltype(reverse_sequence(std::make_index_sequence<N>{}));

    public:
        using Base = tupleVectorBuff<char, allocator>;
        using size_type = typename Base::size_type;
        using traits_type = typename Base::traits_type;
        using allocator_type = typename Base::allocator_type;

        using Base::alloc_;
        using Base::capacityByte_;
        using Base::m_buffer_;

        using Aligner = TupleAligner<Types...>;

        static constexpr size_type countArgs_ = sizeof...(Types);
        size_type sizeByte_ = 0;

        tupleVector(Types... args)
        {
            if constexpr (countArgs_ > 0)
            {
                sizeByte_ = Aligner::get_total_size();
                Base::allocate(sizeByte_);
                constructElem(std::index_sequence_for<Types...>{}, std::forward<Types>(args)...);
            }
        }

        tupleVector(tupleVector &&other) noexcept
            : Base(std::move(other)),
              sizeByte_(other.sizeByte_)
        {
            other.sizeByte_ = 0;
        }

        ~tupleVector()
        {
            if (m_buffer_ != nullptr)
            {
                if constexpr (countArgs_ > 0)
                {
                    destructElem(make_index_sequence_reverse<countArgs_>{});
                }
                Base::deallocate(capacityByte_);
            }
        }

        template <size_t Index>
        auto &operator[](std::integral_constant<size_t, Index>)
        {
            using TargetType = MyTypeAt_t<Index, Types...>;
            constexpr size_t offset = Aligner::template get_offset<Index>();
            return *reinterpret_cast<TargetType *>(m_buffer_ + offset);
        }

        template <size_t Index>
        auto &get()
        {
            using TargetType = MyTypeAt_t<Index, Types...>;
            constexpr size_t offset = Aligner::template get_offset<Index>();
            return *reinterpret_cast<TargetType *>(m_buffer_ + offset);
        }
        template <typename TargetType>
        auto &get()
        {
            constexpr size_t idx = MyIndexOf_v<TargetType, Types...>;

            // Проверки на этапе компиляции для безопасности
            static_assert(idx != static_cast<size_t>(-1), "Error: Type not found in this tupleVector!");
            static_assert(CountType_v<TargetType, Types...> == 1, "Error: Type is ambiguous! Appears multiple times.");

            // Вызываем уже существующий проверенный метод по индексу
            return get<idx>();
        }

    private:
        template <size_t... Is, typename... Args>
        void constructElem(std::index_sequence<Is...>, Args &&...args)
        {
            (traits_type::construct(alloc_,
                                    reinterpret_cast<Types *>(m_buffer_ + Aligner::template get_offset<Is>()),
                                    std::forward<Args>(args)),
             ...);
        }

        template <size_t... ReverseIs>
        void destructElem(std::index_sequence<ReverseIs...>)
        {
            (traits_type::destroy(alloc_,
                                  reinterpret_cast<MyTypeAt_t<ReverseIs, Types...> *>(m_buffer_ + Aligner::template get_offset<ReverseIs>())),
             ...);
        }
    };
}

template <typename T, size_t... Is>
constexpr void printTupleVectorHelp(T &tVec, std::index_sequence<Is...>)
{
    ((std::cout << tVec[std::integral_constant<size_t, Is>{}] << " "), ...);
}
template <typename T>
constexpr void printTupleVector(T &tVec)
{
    printTupleVectorHelp(tVec, std::make_index_sequence<T::countArgs_>{});
}