#include <gtest/gtest.h>
#include <string>
#include "TupleVector.hpp"
#include "AllocatorX.hpp"
#include "Example.hpp"

// Тест на проверку базовой логики перемещения и безопасности деструктора
TEST(TupleVectorTest, MoveConstructorSafety) {
    using allocatorXCh = AllocatorX::allocatorX<char>;

    // Создаем область видимости для tplVec1, чтобы контролировать момент вызова его деструктора
    alignas(Example) char raw_buffer[sizeof(myTuple::tupleVector<char, allocatorXCh, int, double, std::string, Example>)];
    auto* tplVec1 = new (raw_buffer) myTuple::tupleVector<char, allocatorXCh, int, double, std::string, Example>(
        42,
        3.14,
        std::string{ "Hello Tuple" },
        Example{ 7, 'b', 9.9 }
    );

    // Фиксируем исходный адрес буфера
    void* original_buffer_address = tplVec1->m_buffer_;
    ASSERT_NE(original_buffer_address, nullptr);

    // Вызываем конструктор перемещения
    myTuple::tupleVector<char, allocatorXCh, int, double, std::string, Example> tplVec2(std::move(*tplVec1));

    // ПРОВЕРКА 1: Живой объект (tplVec2) должен забрать старый адрес буфера
    EXPECT_EQ(tplVec2.m_buffer_, original_buffer_address);
    EXPECT_GT(tplVec2.sizeByte_, 0);

    // ПРОВЕРКА 2: Перемещенный объект (tplVec1) должен быть корректно сброшен
    EXPECT_EQ(tplVec1->m_buffer_, nullptr);
    EXPECT_EQ(tplVec1->sizeByte_, 0);

    // ПРОВЕРКА 3: Доступ к данным в новом объекте должен работать корректно
    EXPECT_EQ(tplVec2.get<0>(), 42);
    EXPECT_DOUBLE_EQ(tplVec2.get<1>(), 3.14);
    EXPECT_EQ(tplVec2.get<std::string>(), "Hello Tuple");

    // ПРОВЕРКА 4: Безопасность деструктора перемещенного объекта.
    // Принудительно вызываем деструктор у tplVec1. 
    // Если ваш деструктор имеет рантайм-проверку (if (m_buffer_ != nullptr)), этот шаг пройдет успешно.
    // Если проверки нет, здесь произойдет Access Violation и тест упадет.
    EXPECT_NO_THROW({
        tplVec1->~tupleVector();
        });
}

// Тест на корректность выравнивания (размера буфера)
TEST(TupleVectorTest, TotalSizeAlignment) {
    using allocatorXCh = AllocatorX::allocatorX<char>;
    myTuple::tupleVector<char, allocatorXCh, char, int, char, double> tplAlignCheck(
        'a', 10, 'b', 20.0
    );

    // Проверяем, что общий размер учитывает padding (выравнивание) элементов
    size_t expected_min_size = sizeof(char) + sizeof(int) + sizeof(char) + sizeof(double);
    EXPECT_GE(tplAlignCheck.sizeByte_, expected_min_size);
}