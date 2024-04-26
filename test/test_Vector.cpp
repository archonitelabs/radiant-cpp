// Copyright 2023 The Radiant Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "gtest/gtest.h"

#include "test/TestAlloc.h"
#define RAD_DEFAULT_ALLOCATOR radtest::Allocator

#include "radiant/Vector.h"

#include <vector>

struct VecTestStats
{
    int DtorCount = 0;
    int DefaultCtorCount = 0;
    int CtorCount = 0;
    int CopyCtorCount = 0;
    int MoveCtorCount = 0;
    int CopyAssignCount = 0;
    int MoveAssignCount = 0;

    void Reset() noexcept
    {
        DtorCount = 0;
        DefaultCtorCount = 0;
        CtorCount = 0;
        CopyCtorCount = 0;
        MoveCtorCount = 0;
        CopyAssignCount = 0;
        MoveAssignCount = 0;
    }

    bool Empty() const noexcept
    {
        return (DtorCount == 0 && DefaultCtorCount == 0 && CtorCount == 0 &&
                CopyCtorCount == 0 && MoveCtorCount == 0 &&
                CopyAssignCount == 0 && MoveAssignCount == 0);
    }
};

static VecTestStats g_stats;

class VectorTests : public ::testing::Test
{
public:

    void SetUp() override
    {
        g_stats.Reset();
    }

    void TearDown() override
    {
        EXPECT_TRUE(g_stats.Empty());
    }
};

class VecTester
{
public:

    ~VecTester()
    {
        g_stats.DtorCount++;
        m_stats.DtorCount++;
    }

    VecTester() noexcept
    {
        g_stats.DefaultCtorCount++;
        m_stats.DefaultCtorCount++;
    }

    VecTester(int value) noexcept
        : m_value(value)
    {
        g_stats.CtorCount++;
        m_stats.CtorCount++;
    }

    VecTester(const VecTester& o) noexcept
        : m_value(o.m_value)
    {
        g_stats.CopyCtorCount++;
        m_stats.CopyCtorCount++;
    }

    VecTester(VecTester&& o) noexcept
        : m_value(o.m_value)
    {
        g_stats.MoveCtorCount++;
        m_stats.MoveCtorCount++;
    }

    VecTester& operator=(const VecTester& o) noexcept
    {
        if (this != &o)
        {
            m_value = o.m_value;
        }

        g_stats.CopyAssignCount++;
        m_stats.CopyAssignCount++;

        return *this;
    }

    VecTester& operator=(VecTester&& o) noexcept
    {
        if (this != &o)
        {
            m_value = o.m_value;
            o.m_value = 0;
        }

        g_stats.MoveAssignCount++;
        m_stats.MoveAssignCount++;

        return *this;
    }

    int m_value{ 0 };
    VecTestStats m_stats{};
};

TEST(VectorTests, DefaultConstruct)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_EQ(vec.Capacity(), 0u);
}

TEST(VectorTests, InlineDefaultConstruct)
{
    rad::InlineVector<int, 10> vec;

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_EQ(vec.Capacity(), 10u);
}

TEST(VectorTests, AllocatorCopyConstruct)
{
    radtest::HeapAllocator heap;
    radtest::AllocWrapper<int, radtest::HeapAllocator> alloc(heap);
    rad::Vector<int, decltype(alloc)> vec(alloc);

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_EQ(vec.Capacity(), 0u);

    EXPECT_EQ(heap.allocCount, 0);
    EXPECT_EQ(heap.freeCount, 0);
    EXPECT_EQ(vec.GetAllocator().base, &heap);
}

TEST(VectorTests, AllocatorMoveConstruct)
{
    using AllocWrap = radtest::AllocWrapper<int, radtest::HeapAllocator>;

    radtest::HeapAllocator heap;
    rad::Vector<int, AllocWrap> vec(heap);

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_EQ(vec.Capacity(), 0u);

    EXPECT_EQ(heap.allocCount, 0);
    EXPECT_EQ(heap.freeCount, 0);
    EXPECT_EQ(vec.GetAllocator().base, &heap);
}

TEST(VectorTests, Reserve)
{
    using AllocWrap = radtest::AllocWrapper<int, radtest::HeapAllocator>;

    radtest::HeapAllocator heap;
    rad::Vector<int, AllocWrap> vec(heap);

    EXPECT_TRUE(vec.Reserve(100).IsOk());

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_GE(vec.Capacity(), 100u);
    EXPECT_EQ(heap.allocCount, 1);
    EXPECT_EQ(heap.freeCount, 0);

    EXPECT_TRUE(vec.Reserve(50).IsOk());

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_GE(vec.Capacity(), 100u);
    EXPECT_EQ(heap.allocCount, 1);
    EXPECT_EQ(heap.freeCount, 0);

    EXPECT_TRUE(vec.Reserve(100).IsOk());

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_GE(vec.Capacity(), 100u);
    EXPECT_EQ(heap.allocCount, 1);
    EXPECT_EQ(heap.freeCount, 0);

    EXPECT_TRUE(vec.Reserve(150).IsOk());

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_GE(vec.Capacity(), 150u);
    EXPECT_EQ(heap.allocCount, 2);
    EXPECT_EQ(heap.freeCount, 1);
}

TEST(VectorTests, InlineReserve)
{
    using AllocWrap = radtest::AllocWrapper<int, radtest::HeapAllocator>;

    radtest::HeapAllocator heap;
    rad::InlineVector<int, 10, AllocWrap> vec(heap);

    EXPECT_TRUE(vec.Reserve(5).IsOk());

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_EQ(vec.Capacity(), decltype(vec)::InlineCount);
    EXPECT_EQ(heap.allocCount, 0);
    EXPECT_EQ(heap.freeCount, 0);

    EXPECT_TRUE(vec.Reserve(10).IsOk());

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_EQ(vec.Capacity(), decltype(vec)::InlineCount);
    EXPECT_EQ(heap.allocCount, 0);
    EXPECT_EQ(heap.freeCount, 0);

    EXPECT_TRUE(vec.Reserve(100).IsOk());

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_GE(vec.Capacity(), 100u);
    EXPECT_EQ(heap.allocCount, 1);
    EXPECT_EQ(heap.freeCount, 0);

    EXPECT_TRUE(vec.Reserve(50).IsOk());

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_GE(vec.Capacity(), 100u);
    EXPECT_EQ(heap.allocCount, 1);
    EXPECT_EQ(heap.freeCount, 0);

    EXPECT_TRUE(vec.Reserve(100).IsOk());

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_GE(vec.Capacity(), 100u);
    EXPECT_EQ(heap.allocCount, 1);
    EXPECT_EQ(heap.freeCount, 0);

    EXPECT_TRUE(vec.Reserve(150).IsOk());

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_GE(vec.Capacity(), 150u);
    EXPECT_EQ(heap.allocCount, 2);
    EXPECT_EQ(heap.freeCount, 1);
}

TEST(VectorTests, ReserveFail)
{
    using AllocWrap = radtest::AllocWrapper<int, radtest::HeapAllocator>;

    radtest::HeapAllocator heap;
    rad::Vector<int, AllocWrap> vec(heap);

    heap.forceAllocFails = 1;
    EXPECT_TRUE(vec.Reserve(100).IsErr());

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_EQ(vec.Capacity(), 0u);
    EXPECT_EQ(heap.forceAllocFails, 0);
    EXPECT_EQ(heap.allocCount, 0);
}

TEST(VectorTests, InlineReserveFail)
{
    using AllocWrap = radtest::AllocWrapper<int, radtest::HeapAllocator>;

    radtest::HeapAllocator heap;
    rad::InlineVector<int, 10, AllocWrap> vec(heap);

    heap.forceAllocFails = 1;
    EXPECT_TRUE(vec.Reserve(100).IsErr());

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_EQ(vec.Capacity(), decltype(vec)::InlineCount);
    EXPECT_EQ(heap.forceAllocFails, 0);
    EXPECT_EQ(heap.allocCount, 0);
}

TEST(VectorTests, PushBack)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.PushBack(123).IsOk());
    EXPECT_EQ(vec.Size(), 1u);
    EXPECT_EQ(vec.At(0), 123);

    EXPECT_TRUE(vec.PushBack(456).IsOk());
    EXPECT_EQ(vec.Size(), 2u);
    EXPECT_EQ(vec.At(0), 123);
    EXPECT_EQ(vec.At(1), 456);

    EXPECT_TRUE(vec.PushBack(789).IsOk());
    EXPECT_EQ(vec.Size(), 3u);
    EXPECT_EQ(vec.At(0), 123);
    EXPECT_EQ(vec.At(1), 456);
    EXPECT_EQ(vec.At(2), 789);
}

TEST(VectorTests, PushBackCopy)
{
    int value = 1337;
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.PushBack(value).IsOk());
    EXPECT_EQ(value, 1337);
    EXPECT_EQ(vec.Size(), 1u);
    EXPECT_EQ(vec.At(0), 1337);

    EXPECT_TRUE(vec.PushBack(value).IsOk());
    EXPECT_EQ(value, 1337);
    EXPECT_EQ(vec.Size(), 2u);
    EXPECT_EQ(vec.At(0), 1337);
    EXPECT_EQ(vec.At(1), 1337);

    EXPECT_TRUE(vec.PushBack(value).IsOk());
    EXPECT_EQ(value, 1337);
    EXPECT_EQ(vec.Size(), 3u);
    EXPECT_EQ(vec.At(0), 1337);
    EXPECT_EQ(vec.At(1), 1337);
    EXPECT_EQ(vec.At(2), 1337);
}

TEST(VectorTests, EmplaceBack)
{
    struct TestStruct
    {
        int First = 0;
        bool Second = 0;

        TestStruct() noexcept = default;

        TestStruct(int One, bool Two) noexcept
            : First(One),
              Second(Two)
        {
        }
    };

    rad::Vector<TestStruct> vec;

    EXPECT_TRUE(vec.EmplaceBack(123, false).IsOk());
    EXPECT_EQ(vec.Size(), 1u);
    EXPECT_EQ(vec.At(0).First, 123);
    EXPECT_EQ(vec.At(0).Second, false);

    EXPECT_TRUE(vec.EmplaceBack(456, true).IsOk());
    EXPECT_EQ(vec.Size(), 2u);
    EXPECT_EQ(vec.At(1).First, 456);
    EXPECT_EQ(vec.At(1).Second, true);

    EXPECT_TRUE(vec.EmplaceBack(789, false).IsOk());
    EXPECT_EQ(vec.Size(), 3u);
    EXPECT_EQ(vec.At(2).First, 789);
    EXPECT_EQ(vec.At(2).Second, false);
}

TEST(VectorTests, FrontBack)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.PushBack(123).IsOk());
    EXPECT_TRUE(vec.PushBack(456).IsOk());
    EXPECT_TRUE(vec.PushBack(789).IsOk());

    EXPECT_EQ(vec.Front(), 123);
    EXPECT_EQ(vec.Back(), 789);

    const auto& constVec = vec;

    EXPECT_EQ(constVec.Front(), 123);
    EXPECT_EQ(constVec.Back(), 789);
}

TEST(VectorTests, At)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.PushBack(123).IsOk());
    EXPECT_TRUE(vec.PushBack(456).IsOk());
    EXPECT_TRUE(vec.PushBack(789).IsOk());

    EXPECT_EQ(vec.At(0), 123);
    EXPECT_EQ(vec.At(1), 456);
    EXPECT_EQ(vec.At(2), 789);

    const auto& constVec = vec;

    EXPECT_EQ(constVec.At(0), 123);
    EXPECT_EQ(constVec.At(1), 456);
    EXPECT_EQ(constVec.At(2), 789);
}

TEST(VectorTests, Subscript)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.PushBack(123).IsOk());
    EXPECT_TRUE(vec.PushBack(456).IsOk());
    EXPECT_TRUE(vec.PushBack(789).IsOk());

    EXPECT_EQ(vec[0], 123);
    EXPECT_EQ(vec[1], 456);
    EXPECT_EQ(vec[2], 789);

    const auto& constVec = vec;

    EXPECT_EQ(constVec[0], 123);
    EXPECT_EQ(constVec[1], 456);
    EXPECT_EQ(constVec[2], 789);
}

TEST(VectorTests, Data)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.PushBack(123).IsOk());
    EXPECT_TRUE(vec.PushBack(456).IsOk());
    EXPECT_TRUE(vec.PushBack(789).IsOk());

    EXPECT_EQ(vec.Data()[0], 123);
    EXPECT_EQ(vec.Data()[1], 456);
    EXPECT_EQ(vec.Data()[2], 789);

    const auto& constVec = vec;

    EXPECT_EQ(constVec.Data()[0], 123);
    EXPECT_EQ(constVec.Data()[1], 456);
    EXPECT_EQ(constVec.Data()[2], 789);
}

TEST(VectorTests, PopBack)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.PushBack(123).IsOk());
    EXPECT_TRUE(vec.PushBack(456).IsOk());
    EXPECT_TRUE(vec.PushBack(789).IsOk());

    EXPECT_EQ(vec.Size(), 3u);
    EXPECT_EQ(vec.Back(), 789);

    vec.PopBack();

    EXPECT_EQ(vec.Size(), 2u);
    EXPECT_EQ(vec.Back(), 456);

    vec.PopBack();

    EXPECT_EQ(vec.Size(), 1u);
    EXPECT_EQ(vec.Back(), 123);

    vec.PopBack();

    EXPECT_EQ(vec.Size(), 0u);
}

TEST(VectorTests, Resize)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.PushBack(123).IsOk());

    EXPECT_TRUE(vec.Resize(5).IsOk());

    EXPECT_EQ(vec.Size(), 5u);
    EXPECT_EQ(vec.Front(), 123);
    EXPECT_EQ(vec.Back(), 0);

    EXPECT_TRUE(vec.Resize(4).IsOk());

    EXPECT_EQ(vec.Size(), 4u);
    EXPECT_EQ(vec.Front(), 123);
    EXPECT_EQ(vec.Back(), 0);

    EXPECT_TRUE(vec.Resize(5).IsOk());

    EXPECT_EQ(vec.Size(), 5u);
    EXPECT_EQ(vec.Front(), 123);
    EXPECT_EQ(vec.Back(), 0);

    EXPECT_TRUE(vec.Resize(6).IsOk());

    EXPECT_EQ(vec.Size(), 6u);
    EXPECT_EQ(vec.Front(), 123);
    EXPECT_EQ(vec.Back(), 0);
}

TEST(VectorTests, ResizeCopy)
{
    int value = 1337;

    rad::Vector<int> vec;

    EXPECT_TRUE(vec.PushBack(123).IsOk());

    EXPECT_TRUE(vec.Resize(5, value).IsOk());

    EXPECT_EQ(value, 1337);
    EXPECT_EQ(vec.Size(), 5u);
    EXPECT_EQ(vec.Front(), 123);
    EXPECT_EQ(vec.Back(), 1337);

    EXPECT_TRUE(vec.Resize(4, value).IsOk());

    EXPECT_EQ(vec.Size(), 4u);
    EXPECT_EQ(vec.Front(), 123);
    EXPECT_EQ(vec.Back(), 1337);

    EXPECT_TRUE(vec.Resize(5, value).IsOk());

    EXPECT_EQ(vec.Size(), 5u);
    EXPECT_EQ(vec.Front(), 123);
    EXPECT_EQ(vec.Back(), 1337);

    EXPECT_TRUE(vec.Resize(6, value).IsOk());

    EXPECT_EQ(vec.Size(), 6u);
    EXPECT_EQ(vec.Front(), 123);
    EXPECT_EQ(vec.Back(), 1337);
}

TEST(VectorTests, Clear)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.Resize(5).IsOk());

    EXPECT_EQ(vec.Size(), 5u);

    vec.Clear();

    EXPECT_EQ(vec.Size(), 0u);
}

TEST(VectorTests, ShrinkToFit)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.Resize(5).IsOk());

    EXPECT_EQ(vec.Size(), 5u);
    EXPECT_GE(vec.Capacity(), vec.Size());

    vec.PopBack();

    EXPECT_EQ(vec.Size(), 4u);
    EXPECT_GE(vec.Capacity(), vec.Size());

    EXPECT_TRUE(vec.ShrinkToFit().IsOk());

    EXPECT_EQ(vec.Size(), 4u);
    EXPECT_EQ(vec.Capacity(), vec.Size());

    EXPECT_TRUE(vec.ShrinkToFit().IsOk());

    EXPECT_EQ(vec.Size(), 4u);
    EXPECT_EQ(vec.Capacity(), vec.Size());

    vec.Clear();

    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_GT(vec.Capacity(), vec.Size());

    EXPECT_TRUE(vec.ShrinkToFit().IsOk());

    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_EQ(vec.Capacity(), vec.Size());

    EXPECT_TRUE(vec.ShrinkToFit().IsOk());

    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_EQ(vec.Capacity(), vec.Size());
}

TEST(VectorTests, ShrinkToFitInline)
{
    rad::InlineVector<int, 10> vec;

    EXPECT_TRUE(vec.Resize(5).IsOk());

    EXPECT_EQ(vec.Size(), 5u);
    EXPECT_EQ(vec.Capacity(), decltype(vec)::InlineCount);

    vec.PopBack();

    EXPECT_EQ(vec.Size(), 4u);
    EXPECT_EQ(vec.Capacity(), decltype(vec)::InlineCount);

    EXPECT_TRUE(vec.ShrinkToFit().IsOk());

    EXPECT_EQ(vec.Size(), 4u);
    EXPECT_EQ(vec.Capacity(), decltype(vec)::InlineCount);

    vec.Clear();

    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_EQ(vec.Capacity(), decltype(vec)::InlineCount);

    EXPECT_TRUE(vec.ShrinkToFit().IsOk());

    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_EQ(vec.Capacity(), decltype(vec)::InlineCount);

    EXPECT_TRUE(vec.Resize(15).IsOk());

    EXPECT_EQ(vec.Size(), 15u);
    EXPECT_GE(vec.Capacity(), vec.Size());

    vec.PopBack();

    EXPECT_TRUE(vec.ShrinkToFit().IsOk());

    EXPECT_EQ(vec.Size(), 14u);
    EXPECT_GE(vec.Capacity(), vec.Size());

    EXPECT_TRUE(vec.ShrinkToFit().IsOk());

    EXPECT_EQ(vec.Size(), 14u);
    EXPECT_GE(vec.Capacity(), vec.Size());

    vec.Clear();

    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_GE(vec.Capacity(), vec.Size());

    EXPECT_TRUE(vec.ShrinkToFit().IsOk());

    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_EQ(vec.Capacity(), decltype(vec)::InlineCount);

    EXPECT_TRUE(vec.ShrinkToFit().IsOk());

    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_EQ(vec.Capacity(), decltype(vec)::InlineCount);

    //
    // Shrink fits inline.
    //

    EXPECT_TRUE(vec.Resize(12).IsOk());

    vec.PopBack();
    vec.PopBack();

    EXPECT_EQ(vec.Size(), 10u);
    EXPECT_GT(vec.Capacity(), decltype(vec)::InlineCount);

    EXPECT_TRUE(vec.ShrinkToFit().IsOk());

    EXPECT_EQ(vec.Size(), 10u);
    EXPECT_EQ(vec.Capacity(), decltype(vec)::InlineCount);
}

TEST(VectorTests, AssignCopy)
{
    int value = 1337;

    rad::Vector<int> vec;

    EXPECT_TRUE(vec.Assign(5, value).IsOk());

    EXPECT_EQ(value, 1337);
    EXPECT_EQ(vec.Size(), 5u);
    EXPECT_EQ(vec.Front(), 1337);
    EXPECT_EQ(vec.Back(), 1337);

    EXPECT_TRUE(vec.Assign(5, 123).IsOk());

    EXPECT_EQ(vec.Size(), 5u);
    EXPECT_EQ(vec.Front(), 123);
    EXPECT_EQ(vec.Back(), 123);

    EXPECT_TRUE(vec.Assign(2, 456).IsOk());

    EXPECT_EQ(vec.Size(), 2u);
    EXPECT_EQ(vec.Front(), 456);
    EXPECT_EQ(vec.Back(), 456);

    EXPECT_TRUE(vec.Assign(10, 789).IsOk());

    EXPECT_EQ(vec.Size(), 10u);
    EXPECT_EQ(vec.Front(), 789);
    EXPECT_EQ(vec.Back(), 789);
}

#if RAD_ENABLE_STD

TEST(VectorTests, AssignInit)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());

    EXPECT_EQ(vec.Size(), 3u);
    EXPECT_EQ(vec.Front(), 1);
    EXPECT_EQ(vec.Back(), 3);

    EXPECT_TRUE(vec.Assign({ 4, 5 }).IsOk());

    EXPECT_EQ(vec.Size(), 2u);
    EXPECT_EQ(vec.Front(), 4);
    EXPECT_EQ(vec.Back(), 5);

    EXPECT_TRUE(vec.Assign({ 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 }).IsOk());

    EXPECT_EQ(vec.Size(), 10u);
    EXPECT_EQ(vec.Front(), 9);
    EXPECT_EQ(vec.Back(), 0);
}

TEST(VectorTests, AssignRange)
{
    rad::Vector<int> other;

    EXPECT_TRUE(other.Assign({ 1, 2, 3 }).IsOk());

    rad::Vector<int> vec;

    EXPECT_TRUE(vec.Assign(other.ToSpan()).IsOk());

    EXPECT_EQ(vec.Size(), other.Size());
    EXPECT_EQ(vec.Front(), other.Front());
    EXPECT_EQ(vec.Back(), other.Back());
}

TEST(VectorTests, Swap)
{
    rad::Vector<int> vec;
    rad::Vector<int> other;

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());
    EXPECT_TRUE(other.Assign({ 4, 5, 6, 7, 8, 9 }).IsOk());

    vec.Swap(vec);

    vec.Swap(other);

    EXPECT_EQ(vec.Size(), 6u);
    EXPECT_EQ(vec.Front(), 4);
    EXPECT_EQ(vec.Back(), 9);

    EXPECT_EQ(other.Size(), 3u);
    EXPECT_EQ(other.Front(), 1);
    EXPECT_EQ(other.Back(), 3);
}

TEST(VectorTests, InlineSwapBothInline)
{
    rad::InlineVector<int, 4> vec;
    rad::InlineVector<int, 4> other;

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());
    EXPECT_TRUE(other.Assign({ 4, 5, 6, 7 }).IsOk());

    vec.Swap(vec);

    vec.Swap(other);

    EXPECT_EQ(vec.Size(), 4u);
    EXPECT_EQ(vec.Front(), 4);
    EXPECT_EQ(vec.Back(), 7);

    EXPECT_EQ(other.Size(), 3u);
    EXPECT_EQ(other.Front(), 1);
    EXPECT_EQ(other.Back(), 3);
}

TEST(VectorTests, InlineSwapNeitherInline)
{
    rad::InlineVector<int, 4> vec;
    rad::InlineVector<int, 4> other;

    EXPECT_TRUE(vec.Assign({ 1, 2, 3, 4, 6 }).IsOk());
    EXPECT_TRUE(other.Assign({ 7, 8, 9, 10, 11, 12 }).IsOk());

    vec.Swap(vec);

    vec.Swap(other);

    EXPECT_EQ(vec.Size(), 6u);
    EXPECT_EQ(vec.Front(), 7);
    EXPECT_EQ(vec.Back(), 12);

    EXPECT_EQ(other.Size(), 5u);
    EXPECT_EQ(other.Front(), 1);
    EXPECT_EQ(other.Back(), 6);
}

TEST(VectorTests, InlineSwapDisjoint)
{
    rad::InlineVector<int, 4> vec;
    rad::InlineVector<int, 4> other;

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());
    EXPECT_TRUE(other.Assign({ 7, 8, 9, 10, 11, 12 }).IsOk());

    vec.Swap(vec);

    vec.Swap(other);

    EXPECT_EQ(vec.Size(), 6u);
    EXPECT_EQ(vec.Front(), 7);
    EXPECT_EQ(vec.Back(), 12);

    EXPECT_EQ(other.Size(), 3u);
    EXPECT_EQ(other.Front(), 1);
    EXPECT_EQ(other.Back(), 3);

    vec.Swap(other);

    EXPECT_EQ(other.Size(), 6u);
    EXPECT_EQ(other.Front(), 7);
    EXPECT_EQ(other.Back(), 12);

    EXPECT_EQ(vec.Size(), 3u);
    EXPECT_EQ(vec.Front(), 1);
    EXPECT_EQ(vec.Back(), 3);
}

TEST(VectorTests, Copy)
{
    rad::Vector<int> vec;
    rad::Vector<int> other;

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());

    EXPECT_TRUE(vec.Copy(vec).IsOk());

    EXPECT_EQ(vec.Size(), 3u);
    EXPECT_EQ(vec.Front(), 1);
    EXPECT_EQ(vec.Back(), 3);

    EXPECT_TRUE(vec.Copy(other).IsOk());

    EXPECT_EQ(vec.Size(), other.Size());
    EXPECT_EQ(vec.Front(), other.Front());
    EXPECT_EQ(vec.Back(), other.Back());
}

TEST(VectorTests, CopyOverwrite)
{
    rad::Vector<int> vec;
    rad::Vector<int> other;

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());
    EXPECT_TRUE(other.Assign({ 1, 2, 3, 4, 5 }).IsOk());

    EXPECT_EQ(vec.Size(), 3u);
    EXPECT_EQ(vec.Front(), 1);
    EXPECT_EQ(vec.Back(), 3);

    EXPECT_TRUE(vec.Copy(other).IsOk());

    EXPECT_EQ(vec.Size(), other.Size());
    EXPECT_EQ(vec.Front(), other.Front());
    EXPECT_EQ(vec.Back(), other.Back());
}

TEST(VectorTests, Move)
{
    rad::Vector<int> vec;
    rad::Vector<int> other;

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());

    vec.Move(vec);

    EXPECT_EQ(vec.Size(), 3u);
    EXPECT_EQ(vec.Front(), 1);
    EXPECT_EQ(vec.Back(), 3);

    vec.Move(other);

    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_EQ(other.Size(), 3u);
    EXPECT_EQ(other.Front(), 1);
    EXPECT_EQ(other.Back(), 3);
}

TEST(VectorTests, Double)
{
    rad::Vector<double> vec;

    EXPECT_TRUE(vec.Assign({ 1.1, 2.2, 3.3 }).IsOk());

    EXPECT_EQ(vec.At(0), 1.1);
    EXPECT_EQ(vec.At(1), 2.2);
    EXPECT_EQ(vec.At(2), 3.3);
}

TEST(VectorTests, Float)
{
    rad::Vector<float> vec;

    EXPECT_TRUE(vec.Assign({ 1.1f, 2.2f, 3.3f }).IsOk());

    EXPECT_EQ(vec.At(0), 1.1f);
    EXPECT_EQ(vec.At(1), 2.2f);
    EXPECT_EQ(vec.At(2), 3.3f);
}

TEST(VectorTests, RemoveBack)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());

    EXPECT_EQ(vec.RemoveBack(), 3);
    EXPECT_EQ(vec.RemoveBack(), 2);
    EXPECT_EQ(vec.RemoveBack(), 1);
}

TEST(VectorTests, Seek)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());

    EXPECT_EQ(vec.Seek(0).Ok(), 1);
    EXPECT_EQ(vec.Seek(1).Ok(), 2);
    EXPECT_EQ(vec.Seek(2).Ok(), 3);
    EXPECT_TRUE(vec.Seek(3).IsErr());

    const auto& other = vec;

    EXPECT_EQ(other.Seek(0).Ok(), 1);
    EXPECT_EQ(other.Seek(1).Ok(), 2);
    EXPECT_EQ(other.Seek(2).Ok(), 3);
    EXPECT_TRUE(other.Seek(3).IsErr());
}

TEST(VectorTests, SeekFront)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.SeekFront().IsErr());

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());

    EXPECT_EQ(vec.SeekFront().Ok(), 1);

    const auto& other = vec;

    EXPECT_EQ(other.SeekFront(), 1);
}

TEST(VectorTests, SeekBack)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.SeekBack().IsErr());

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());

    EXPECT_EQ(vec.SeekBack().Ok(), 3);

    const auto& other = vec;

    EXPECT_EQ(other.SeekBack(), 3);
}

TEST(VectorTests, MoveOperator)
{
    rad::Vector<int> vec;
    rad::Vector<int> other;

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());

    other = Move(vec);

    EXPECT_TRUE(vec.Empty());

    EXPECT_EQ(other.Size(), 3u);
    EXPECT_EQ(other[0], 1);
    EXPECT_EQ(other[1], 2);
    EXPECT_EQ(other[2], 3);
}

TEST(VectorTests, AssignDown)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());

    EXPECT_TRUE(vec.Assign(1, 123).IsOk());

    EXPECT_EQ(vec.Size(), 1u);
    EXPECT_EQ(vec[0], 123);

    EXPECT_TRUE(vec.Assign(2, 123).IsOk());

    EXPECT_EQ(vec.Size(), 2u);
    EXPECT_EQ(vec[0], 123);
    EXPECT_EQ(vec[1], 123);
}

TEST(VectorTests, AssignOverlapping)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());

    EXPECT_EQ(vec.Assign(vec.ToSpan()).Err(), rad::Error::InvalidAddress);
    EXPECT_EQ(vec.Assign(vec.ToSpan(0, 1)).Err(), rad::Error::InvalidAddress);
    EXPECT_EQ(vec.Assign(vec.ToSpan(2, 1)).Err(), rad::Error::InvalidAddress);
    EXPECT_EQ(vec.Assign(vec.ToSpan(2)).Err(), rad::Error::InvalidAddress);
}

TEST(VectorTests, ResizeSame)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());

    EXPECT_TRUE(vec.Resize(3).IsOk());
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);

    EXPECT_TRUE(vec.Resize(3, 123).IsOk());
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);
}

TEST(VectorTests, ShrinkToFitNoMemory)
{
    using AllocWrap = radtest::AllocWrapper<int, radtest::HeapAllocator>;

    radtest::HeapAllocator heap;
    rad::Vector<int, AllocWrap> vec(heap);

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());

    vec.PopBack();

    heap.forceAllocFails = 1;
    EXPECT_EQ(vec.ShrinkToFit().Err(), rad::Error::NoMemory);

    EXPECT_EQ(vec.Size(), 2u);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
}

TEST(VectorTests, InlineShrinkToFitNoMemory)
{
    using AllocWrap = radtest::AllocWrapper<int, radtest::HeapAllocator>;

    radtest::HeapAllocator heap;
    rad::Vector<int, AllocWrap, 1> vec(heap);

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());

    vec.PopBack();

    heap.forceAllocFails = 1;
    EXPECT_EQ(vec.ShrinkToFit().Err(), rad::Error::NoMemory);

    EXPECT_EQ(vec.Size(), 2u);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
}

TEST(VectorTests, CopyNoMemory)
{
    using AllocWrap = radtest::AllocWrapper<int, radtest::HeapAllocator>;

    radtest::HeapAllocator heap;
    rad::Vector<int, AllocWrap> vec(heap);
    rad::Vector<int, AllocWrap> other(heap);

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());

    heap.forceAllocFails = 1;
    EXPECT_EQ(vec.Copy(other).Err(), rad::Error::NoMemory);

    EXPECT_EQ(vec.Size(), 3u);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 3);
}

#endif // RAD_ENABLE_STD

TEST(VectorTests, ResizeNoMemory)
{
    using AllocWrap = radtest::AllocWrapper<int, radtest::HeapAllocator>;

    radtest::HeapAllocator heap;
    rad::Vector<int, AllocWrap> vec(heap);

    heap.forceAllocFails = 1;
    EXPECT_EQ(vec.Resize(3).Err(), rad::Error::NoMemory);

    EXPECT_TRUE(vec.Empty());

    heap.forceAllocFails = 1;
    EXPECT_EQ(vec.Resize(3, 123).Err(), rad::Error::NoMemory);

    EXPECT_TRUE(vec.Empty());
}

#if RAD_ENABLE_STD

TEST(VectorTests, AssignNoMemory)
{
    using AllocWrap = radtest::AllocWrapper<int, radtest::HeapAllocator>;

    radtest::HeapAllocator heap;
    rad::Vector<int, AllocWrap> vec(heap);

    heap.forceAllocFails = 1;
    EXPECT_EQ(vec.Assign(3, 123).Err(), rad::Error::NoMemory);

    EXPECT_TRUE(vec.Empty());

    heap.forceAllocFails = 1;
    EXPECT_EQ(vec.Assign({ 1, 2, 3 }).Err(), rad::Error::NoMemory);

    EXPECT_TRUE(vec.Empty());
}

#endif // RAD_ENABLE_STD

TEST(VectorTests, EmplaceBackNoMemory)
{
    using AllocWrap = radtest::AllocWrapper<int, radtest::HeapAllocator>;

    radtest::HeapAllocator heap;
    rad::Vector<int, AllocWrap> vec(heap);

    heap.forceAllocFails = 1;
    EXPECT_EQ(vec.EmplaceBack(1).Err(), rad::Error::NoMemory);

    EXPECT_TRUE(vec.Empty());
}

#if RAD_ENABLE_STD

TEST(VectorTests, EqualityOperators)
{
    rad::Vector<int> left;
    rad::Vector<int> right;

    EXPECT_TRUE(left.Assign({ 1, 2, 3, 4 }).IsOk());
    EXPECT_TRUE(right.Assign({ 1, 2, 3, 4 }).IsOk());

    EXPECT_TRUE(left == right);
    EXPECT_TRUE(right == left);
    EXPECT_FALSE(left != right);
    EXPECT_FALSE(right != left);

    right.Clear();

    EXPECT_FALSE(left == right);
    EXPECT_FALSE(right == left);
    EXPECT_TRUE(left != right);
    EXPECT_TRUE(right != left);

    rad::InlineVector<int, 5> ilright;

    EXPECT_TRUE(ilright.Assign({ 1, 2, 3, 4 }).IsOk());

    EXPECT_TRUE(left == ilright);
    EXPECT_TRUE(ilright == left);
    EXPECT_FALSE(left != ilright);
    EXPECT_FALSE(ilright != left);

    ilright.Clear();

    EXPECT_FALSE(left == ilright);
    EXPECT_FALSE(ilright == left);
    EXPECT_TRUE(left != ilright);
    EXPECT_TRUE(ilright != left);

    EXPECT_TRUE(ilright.Assign({ 1, 1, 1, 1 }).IsOk());

    EXPECT_FALSE(left == ilright);
    EXPECT_FALSE(ilright == left);
    EXPECT_TRUE(left != ilright);
    EXPECT_TRUE(ilright != left);
}

TEST(VectorTests, ComparisonOperators)
{
    rad::Vector<int> left;
    rad::Vector<int> right;

    EXPECT_TRUE(left.Assign({ 1, 2, 3, 4 }).IsOk());
    EXPECT_TRUE(right.Assign({ 1, 2, 3, 4 }).IsOk());

    EXPECT_FALSE(left < right);
    EXPECT_FALSE(left > right);
    EXPECT_TRUE(left <= right);
    EXPECT_TRUE(left >= right);

    EXPECT_FALSE(right < left);
    EXPECT_FALSE(right > left);
    EXPECT_TRUE(right <= left);
    EXPECT_TRUE(right >= left);

    EXPECT_TRUE(left.Assign({ 1, 1, 1, 1 }).IsOk());
    EXPECT_TRUE(right.Assign({ 2, 2, 2, 2 }).IsOk());

    EXPECT_TRUE(left < right);
    EXPECT_FALSE(left > right);
    EXPECT_TRUE(left <= right);
    EXPECT_FALSE(left >= right);

    EXPECT_FALSE(right < left);
    EXPECT_TRUE(right > left);
    EXPECT_FALSE(right <= left);
    EXPECT_TRUE(right >= left);

    EXPECT_TRUE(left.Assign({ 1, 1, 1 }).IsOk());
    EXPECT_TRUE(right.Assign({ 1, 1, 1, 1 }).IsOk());

    EXPECT_TRUE(left < right);
    EXPECT_FALSE(left > right);
    EXPECT_TRUE(left <= right);
    EXPECT_FALSE(left >= right);

    EXPECT_FALSE(right < left);
    EXPECT_TRUE(right > left);
    EXPECT_FALSE(right <= left);
    EXPECT_TRUE(right >= left);

    rad::InlineVector<int, 5> ilright;

    EXPECT_TRUE(ilright.Assign({ 1, 1, 1, 1 }).IsOk());

    EXPECT_TRUE(left < right);
    EXPECT_FALSE(left > right);
    EXPECT_TRUE(left <= right);
    EXPECT_FALSE(left >= right);

    EXPECT_FALSE(right < left);
    EXPECT_TRUE(right > left);
    EXPECT_FALSE(right <= left);
    EXPECT_TRUE(right >= left);

    std::vector<int> v;
}

#endif // RAD_ENABLE_STD

TEST(VectorTests, NonTrivReserve)
{
    g_stats.Reset();

    rad::Vector<VecTester> vec;

    EXPECT_TRUE(vec.Reserve(10).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    g_stats.Reset();
}

TEST(VectorTests, NonTrivResize)
{
    VecTester value(123);
    rad::Vector<VecTester> vec;

    g_stats.Reset();
    EXPECT_TRUE(vec.Resize(5).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 5);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    g_stats.Reset();
    EXPECT_TRUE(vec.Resize(5).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    g_stats.Reset();
    EXPECT_TRUE(vec.Resize(2).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 3);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    g_stats.Reset();
    EXPECT_TRUE(vec.Resize(2, value).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    g_stats.Reset();
    EXPECT_TRUE(vec.Resize(5, value).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 3);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    g_stats.Reset();
    EXPECT_TRUE(vec.Resize(10, value).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 5);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 5);
    EXPECT_EQ(g_stats.MoveCtorCount, 5);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    g_stats.Reset();
}

TEST(VectorTests, NonTrivAssign)
{
    VecTester value(123);

    rad::Vector<VecTester> vec;

    g_stats.Reset();
    EXPECT_TRUE(vec.Assign(10, value).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 10);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    value = 456;

    g_stats.Reset();
    EXPECT_TRUE(vec.Assign(20, value).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 10);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 20);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    g_stats.Reset();
    EXPECT_TRUE(vec.Assign(5, value).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 20);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 5);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    g_stats.Reset();
}

TEST(VectorTests, NonTrivClear)
{
    VecTester value(123);

    rad::Vector<VecTester> vec;

    g_stats.Reset();
    EXPECT_TRUE(vec.Assign(10, value).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 10);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    g_stats.Reset();
    vec.Clear();

    EXPECT_EQ(g_stats.DtorCount, 10);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    g_stats.Reset();
}

TEST(VectorTests, NonTrivPushBackLVal)
{
    VecTester value(123);

    rad::Vector<VecTester> vec;

    g_stats.Reset();
    EXPECT_TRUE(vec.PushBack(value).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 1);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    g_stats.Reset();
}

TEST(VectorTests, NonTrivPushBackRVal)
{
    VecTester value(123);

    rad::Vector<VecTester> vec;

    g_stats.Reset();
    EXPECT_TRUE(vec.PushBack(rad::Move(value)).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 1);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    g_stats.Reset();
}

TEST(VectorTests, NonTrivPushEmplaceBack)
{
    rad::Vector<VecTester> vec;

    g_stats.Reset();
    EXPECT_TRUE(vec.EmplaceBack(123).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 1);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    g_stats.Reset();
}

TEST(VectorTests, NonTrivCopy)
{
    VecTester value(123);

    rad::Vector<VecTester> vec;

    EXPECT_TRUE(vec.Assign(10, value).IsOk());

    rad::Vector<VecTester> other;

    g_stats.Reset();
    EXPECT_TRUE(vec.Copy(other).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 10);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 10);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    g_stats.Reset();
}

TEST(VectorTests, NonTrivMove)
{
    VecTester value(123);

    rad::Vector<VecTester> vec;

    EXPECT_TRUE(vec.Assign(10, value).IsOk());

    rad::Vector<VecTester> other;

    g_stats.Reset();
    vec.Move(other);

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    g_stats.Reset();
}

TEST(VectorTests, NonTrivShrinkToFit)
{
    VecTester value(123);

    rad::Vector<VecTester> vec;

    EXPECT_TRUE(vec.Assign(10, value).IsOk());

    vec.PopBack();

    g_stats.Reset();
    EXPECT_TRUE(vec.ShrinkToFit().IsOk());

    EXPECT_EQ(g_stats.DtorCount, 9);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 9);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    while (!vec.Empty())
    {
        vec.PopBack();
    }

    g_stats.Reset();
    EXPECT_TRUE(vec.ShrinkToFit().IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    g_stats.Reset();
}

TEST(VectorTests, NonTrivSwap)
{
    VecTester value(123);

    rad::Vector<VecTester> vec;

    EXPECT_TRUE(vec.Assign(10, value).IsOk());

    rad::Vector<VecTester> other;

    g_stats.Reset();
    vec.Swap(other);

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    g_stats.Reset();
}

TEST(VectorTests, InlineNonTrivShrinkToFit)
{
    VecTester value(123);

    rad::InlineVector<VecTester, 5> vec;

    EXPECT_TRUE(vec.Assign(10, value).IsOk());

    vec.PopBack();

    g_stats.Reset();
    EXPECT_TRUE(vec.ShrinkToFit().IsOk());

    EXPECT_EQ(g_stats.DtorCount, 9);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 9);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    vec.PopBack();
    vec.PopBack();
    vec.PopBack();
    vec.PopBack();

    g_stats.Reset();
    EXPECT_TRUE(vec.ShrinkToFit().IsOk());

    EXPECT_EQ(g_stats.DtorCount, 5);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 5);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    while (!vec.Empty())
    {
        vec.PopBack();
    }

    g_stats.Reset();
    EXPECT_TRUE(vec.ShrinkToFit().IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    g_stats.Reset();
}

TEST(VectorTests, InlineNonTrivSwap)
{
    VecTester value(123);

    rad::InlineVector<VecTester, 5> vec;

    EXPECT_TRUE(vec.Assign(10, value).IsOk());

    rad::InlineVector<VecTester, 5> other;

    EXPECT_TRUE(other.Assign(10, value).IsOk());

    g_stats.Reset();
    vec.Swap(other);

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    vec.PopBack();
    vec.PopBack();
    vec.PopBack();
    vec.PopBack();
    vec.PopBack();
    EXPECT_TRUE(vec.ShrinkToFit().IsOk());

    g_stats.Reset();
    vec.Swap(other);

    EXPECT_EQ(g_stats.DtorCount, 5);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 5);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    vec.PopBack();
    vec.PopBack();
    vec.PopBack();
    vec.PopBack();
    vec.PopBack();
    EXPECT_TRUE(vec.ShrinkToFit().IsOk());

    g_stats.Reset();
    vec.Swap(other);

    EXPECT_EQ(g_stats.DtorCount, 5);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 10);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 5);

    g_stats.Reset();
}

struct VecTestStruct
{
    uint64_t UInt64;
    double Double;
};

TEST(VectorTests, TrivialStructurePushBack)
{
    rad::Vector<VecTestStruct> vec;

    VecTestStruct data;
    for (uint32_t i = 0; i < 10; i++)
    {
        data.UInt64 = i;
        data.Double = static_cast<double>(i);

        EXPECT_TRUE(vec.PushBack(data).IsOk());
    }

    for (uint32_t i = 0; i < 10; i++)
    {
        auto& entry = vec.At(i);
        EXPECT_EQ(entry.UInt64, i);
        EXPECT_EQ(entry.Double, static_cast<double>(i));
    }
}

TEST(VectorTests, Pointers)
{
    rad::Vector<VecTestStruct*> vec;

    VecTestStruct data;
    for (uint32_t i = 0; i < 10; i++)
    {
        data.UInt64 = i;
        data.Double = static_cast<double>(i);

        EXPECT_TRUE(vec.PushBack(&data).IsOk());
    }

    for (uint32_t i = 0; i < 10; i++)
    {
        auto& entry = vec.At(i);

        EXPECT_EQ(entry, &data);
        EXPECT_EQ(entry->UInt64, 9);
        EXPECT_EQ(entry->Double, static_cast<double>(9));
    }
}
