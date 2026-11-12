#include <gtest/gtest.h>
#include <string>
#include <array>
#include <map>
#include "TupleVector.hpp"
#include "AllocatorX.hpp"
#include "Example.hpp"
#include <type_traits>
#include <new>

TEST(TupleVectorBuffTest, DefaultConstructorSafety)
{
    using allocatorXCh = AllocatorX::allocatorX<char>;
    myTuple::tupleVectorBuff<char, allocatorXCh> buff;

    EXPECT_EQ(buff.m_buffer_, nullptr);
    EXPECT_EQ(buff.capacityByte_, 0);

    EXPECT_NO_THROW({ buff.~tupleVectorBuff(); });
}

TEST(TupleVectorBuffTest, SizeConstructorSafety)
{
    using allocatorXCh = AllocatorX::allocatorX<char>;
    myTuple::tupleVectorBuff<char, allocatorXCh> buff(1024);

    EXPECT_NE(buff.m_buffer_, nullptr);
    EXPECT_EQ(buff.capacityByte_, 1024);

    EXPECT_NO_THROW({ buff.~tupleVectorBuff(); });
}

TEST(TupleVectorBuffTest, AllocateDeallocateCreatesWritableBufferAndStoresCapacity)
{
    using allocator = TrackingAllocator::TrackingAllocator<char>;
    allocator::reset();
    myTuple::tupleVectorBuff<char, allocator> buff;

    buff.allocate(32);

    ASSERT_NE(buff.m_buffer_, nullptr);
    EXPECT_EQ(buff.capacityByte_, 32);
    EXPECT_EQ(allocator::allocationCalls, 1);
    buff.m_buffer_[0] = 'a';
    buff.m_buffer_[31] = 'z';
    EXPECT_EQ(buff.m_buffer_[0], 'a');
    EXPECT_EQ(buff.m_buffer_[31], 'z');

    buff.deallocate(32);

    EXPECT_EQ(buff.m_buffer_, nullptr);
    EXPECT_EQ(buff.capacityByte_, 0);
    EXPECT_EQ(allocator::deallocationCalls, 1);
    EXPECT_EQ(allocator::lastDeallocatedSize, 32);
}

TEST(TupleVectorBuffTest, AllocateReleasesPreviousBuffer)
{
    using allocator = TrackingAllocator::TrackingAllocator<char>;
    allocator::reset();
    myTuple::tupleVectorBuff<char, allocator> buff;
    buff.allocate(16);

    buff.allocate(64);

    ASSERT_NE(buff.m_buffer_, nullptr);
    EXPECT_EQ(buff.capacityByte_, 64);
    EXPECT_EQ(allocator::allocationCalls, 2);
    EXPECT_EQ(allocator::deallocationCalls, 1);
    EXPECT_EQ(allocator::lastDeallocatedSize, 16);

    buff.deallocate(64);
    EXPECT_EQ(allocator::deallocationCalls, 2);
    EXPECT_EQ(allocator::lastDeallocatedSize, 64);
}

TEST(TupleVectorBuffTest, AllocateFailureAfterReallocationLeavesBufferEmpty)
{
    using allocator = TrackingAllocator::TrackingAllocator<char>;
    allocator::reset();
    myTuple::tupleVectorBuff<char, allocator> buff;
    buff.allocate(16);
    allocator::failNextAllocation = true;

    EXPECT_THROW(buff.allocate(64), std::bad_alloc);

    EXPECT_EQ(buff.m_buffer_, nullptr);
    EXPECT_EQ(buff.capacityByte_, 0);
    EXPECT_EQ(allocator::allocationCalls, 2);
    EXPECT_EQ(allocator::deallocationCalls, 1);
    EXPECT_EQ(allocator::lastDeallocatedSize, 16);
}

TEST(TupleVectorBuffTest, CopyConstructorAndAssignmentSafety)
{

    using allocatorXCh = AllocatorX::allocatorX<char>;
    myTuple::tupleVectorBuff<char, allocatorXCh> buff1(512);

    EXPECT_NE(buff1.m_buffer_, nullptr);
    EXPECT_EQ(buff1.capacityByte_, 512);

    EXPECT_FALSE((std::is_copy_constructible_v<myTuple::tupleVectorBuff<char, allocatorXCh>>));

    EXPECT_FALSE((std::is_copy_assignable_v<myTuple::tupleVectorBuff<char, allocatorXCh>>));

    EXPECT_NO_THROW({ buff1.~tupleVectorBuff(); });
}

TEST(TupleVectorBuffTest, MoveConstructorAndAssignmentSafety)
{
    using allocatorXCh = AllocatorX::allocatorX<char>;
    myTuple::tupleVectorBuff<char, allocatorXCh> buff1(512);
    myTuple::tupleVectorBuff<char, allocatorXCh> buff2(1024);
    myTuple::tupleVectorBuff<char, allocatorXCh> buff3(std::move(buff1)); // Проверка конструктора перемещения

    buff1 = std::move(buff2); // Проверка оператора перемещающего присваивания

    EXPECT_NE(buff1.m_buffer_, nullptr);
    EXPECT_EQ(buff1.capacityByte_, 1024);
    EXPECT_NE(buff3.m_buffer_, nullptr);
    EXPECT_EQ(buff3.capacityByte_, 512);
    EXPECT_EQ(buff2.m_buffer_, nullptr);
    EXPECT_EQ(buff2.capacityByte_, 0);

    EXPECT_TRUE(std::is_move_constructible_v<decltype(buff3)>);
    EXPECT_TRUE(std::is_move_assignable_v<decltype(buff3)>);

    EXPECT_NO_THROW({ buff1.~tupleVectorBuff(); });
    EXPECT_NO_THROW({ buff2.~tupleVectorBuff(); });
    EXPECT_NO_THROW({ buff3.~tupleVectorBuff(); });
}

TEST(TupleVectorTest, MoveConstructorSafety)
{
    using allocatorXCh = AllocatorX::allocatorX<char>;

    alignas(Example) char raw_buffer[sizeof(myTuple::tupleVector<char, allocatorXCh, int, double, std::string, Example>)];
    auto *tplVec1 = new (raw_buffer) myTuple::tupleVector<char, allocatorXCh, int, double, std::string, Example>(
        42,
        3.14,
        std::string{"Hello Tuple"},
        Example{7, 'b', 9.9});

    void *original_buffer_address = tplVec1->m_buffer_;
    ASSERT_NE(original_buffer_address, nullptr);

    myTuple::tupleVector<char, allocatorXCh, int, double, std::string, Example> tplVec2(std::move(*tplVec1));

    EXPECT_EQ(tplVec2.m_buffer_, original_buffer_address);
    EXPECT_GT(tplVec2.sizeByte_, 0);

    EXPECT_EQ(tplVec1->m_buffer_, nullptr);
    EXPECT_EQ(tplVec1->sizeByte_, 0);

    EXPECT_EQ(tplVec2.get<0>(), 42);
    EXPECT_DOUBLE_EQ(tplVec2.get<1>(), 3.14);
    EXPECT_EQ(tplVec2.get<std::string>(), "Hello Tuple");

    EXPECT_NO_THROW({
        tplVec1->~tupleVector();
    });
}

TEST(TupleVectorTest, TotalSizeAlignment)
{
    using allocatorXCh = AllocatorX::allocatorX<char>;
    myTuple::tupleVector<char, allocatorXCh, char, int, char, double> tplAlignCheck(
        'a', 10, 'b', 20.0);

    size_t expected_min_size = sizeof(char) + sizeof(int) + sizeof(char) + sizeof(double);
    EXPECT_GE(tplAlignCheck.sizeByte_, expected_min_size);
}

TEST(TupleVectorTest, squareBracketsOperatorMath)
{
    using allocatorXCh = AllocatorX::allocatorX<char>;
    using map_Type = std::map<std::string, int, std::less<>, AllocatorX::allocatorX<std::pair<const std::string, int>>>;
    myTuple::tupleVector<char, allocatorXCh, int, double, std::string, Example> tplVec1(
        42,
        3.14,
        std::string{"Hello Tuple"},
        Example{7, 'b', 9.9});
    ASSERT_NE(tplVec1.sizeByte_, 0);
    EXPECT_EQ(tplVec1.countArgs_, 4);

    EXPECT_EQ((tplVec1[std::integral_constant<size_t, 0>()]), 42);
    EXPECT_DOUBLE_EQ((tplVec1[std::integral_constant<size_t, 1>()]), 3.14);
    EXPECT_EQ((tplVec1[std::integral_constant<size_t, 2>()]), "Hello Tuple");
    EXPECT_EQ((tplVec1[std::integral_constant<size_t, 3>()].i), 7);
    EXPECT_EQ((tplVec1[std::integral_constant<size_t, 3>()].ch), 'b');
    EXPECT_DOUBLE_EQ((tplVec1[std::integral_constant<size_t, 3>()].d), 9.9);
}

TEST(TupleVectorTest, getMethodsMath)
{
    using allocatorXCh = AllocatorX::allocatorX<char>;
    using array_Type = std::array<int, 3>;
    using map_Type = std::map<std::string, int, std::less<>, AllocatorX::allocatorX<std::pair<const std::string, int>>>;
    myTuple::tupleVector<char, allocatorXCh,
                         array_Type,
                         int,
                         double,
                         map_Type,
                         std::string,
                         Example>
        tplVec1(
            array_Type{44, 55, 66},
            42,
            3.14,
            map_Type{{"Value1", 1}, {"Value2", 2}, {"Value3", 3}, {"Value4", 4}},
            std::string{"Hello Tuple"},
            Example{7, 'b', 9.9});

    EXPECT_EQ(tplVec1.countArgs_, 6);

    const array_Type expectedArray{44, 55, 66};
    EXPECT_EQ(tplVec1.get<0>(), expectedArray);
    EXPECT_EQ((tplVec1.get<1>()), 42);
    EXPECT_DOUBLE_EQ((tplVec1.get<2>()), 3.14);
    const map_Type expectedMap{{"Value1", 1}, {"Value2", 2}, {"Value3", 3}, {"Value4", 4}};
    EXPECT_EQ(tplVec1.get<3>(), expectedMap);
    EXPECT_EQ((tplVec1.get<4>()), "Hello Tuple");

    EXPECT_EQ(tplVec1.get<5>().i, 7);
    EXPECT_EQ(tplVec1.get<5>().ch, 'b');
    EXPECT_DOUBLE_EQ(tplVec1.get<5>().d, 9.9);

    EXPECT_EQ((tplVec1.get<array_Type>()), expectedArray);
    EXPECT_EQ((tplVec1.get<int>()), (tplVec1.get<1>()));
    EXPECT_EQ((tplVec1.get<double>()), (tplVec1.get<2>()));
    EXPECT_EQ((tplVec1.get<map_Type>()), expectedMap);
    EXPECT_EQ((tplVec1.get<std::string>()), (tplVec1.get<4>()));

    EXPECT_EQ(tplVec1.get<Example>().i, tplVec1.get<5>().i);
    EXPECT_EQ(tplVec1.get<Example>().ch, tplVec1.get<5>().ch);
    EXPECT_DOUBLE_EQ(tplVec1.get<Example>().d, tplVec1.get<5>().d);
}