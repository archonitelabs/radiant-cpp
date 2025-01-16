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

//
// Disable nothrow assertions for unit testing
//
#define RAD_ENABLE_NOTHROW_ASSERTIONS      0
#define RAD_ENABLE_NOTHROW_DTOR_ASSERTIONS 0
#define RAD_ENABLE_NOTHROW_MOVE_ASSERTIONS 0

#define RAD_DEFAULT_ALLOCATOR radtest::Mallocator

#include "gtest/gtest.h"
#include "test/TestAlloc.h"

#include "radiant/Vector.h"

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

    void Add(VecTestStats& stats) noexcept
    {
        DtorCount += stats.DtorCount;
        DefaultCtorCount += stats.DefaultCtorCount;
        CtorCount += stats.CtorCount;
        CopyCtorCount += stats.CopyCtorCount;
        MoveCtorCount += stats.MoveCtorCount;
        CopyAssignCount += stats.CopyAssignCount;
        MoveAssignCount += stats.MoveAssignCount;
    }

    bool AllDestructorsCalled()
    {
        int ctor = MoveCtorCount + CopyCtorCount + CtorCount + DefaultCtorCount;
        return ctor == DtorCount;
    }
};

static VecTestStats g_stats;

template <typename T>
class VectorTest : public testing::Test
{
    void SetUp() override
    {
        g_stats.Reset();
        stats.Reset();
    }

    void TearDown() override
    {
        stats.Add(g_stats);
        EXPECT_TRUE(stats.AllDestructorsCalled());
        g_stats.Reset();
    }

    VecTestStats stats;

public:

    void ResetStats()
    {
        stats.Add(g_stats);
        g_stats.Reset();
    }
};

class SafetyException : public std::exception
{
};

template <bool NoThrow>
class VecTesterBase
{
public:

    static constexpr bool isNoThrow = NoThrow;

    ~VecTesterBase()
    {
        g_stats.DtorCount++;
        m_stats.DtorCount++;
    }

    VecTesterBase() noexcept(NoThrow)
    {
        TryThrow();

        g_stats.DefaultCtorCount++;
        m_stats.DefaultCtorCount++;
    }

    explicit VecTesterBase(int value) noexcept(NoThrow)
        : m_value(value)
    {
        TryThrow();

        g_stats.CtorCount++;
        m_stats.CtorCount++;
    }

    VecTesterBase(const VecTesterBase& o) noexcept(NoThrow)
        : m_value(o.m_value)
    {
        TryThrow();

        g_stats.CopyCtorCount++;
        m_stats.CopyCtorCount++;
    }

    VecTesterBase(VecTesterBase&& o) noexcept
        : m_value(o.m_value)
    {
        g_stats.MoveCtorCount++;
        m_stats.MoveCtorCount++;
    }

    VecTesterBase& operator=(const VecTesterBase& o) noexcept(NoThrow)
    {
        TryThrow();

        if (this != &o)
        {
            m_value = o.m_value;
        }

        g_stats.CopyAssignCount++;
        m_stats.CopyAssignCount++;

        return *this;
    }

    VecTesterBase& operator=(VecTesterBase&& o) noexcept
    {
        TryThrow();

        if (this != &o)
        {
            m_value = o.m_value;
            o.m_value = 0;
        }

        g_stats.MoveAssignCount++;
        m_stats.MoveAssignCount++;

        return *this;
    }

    static void ThrowIn(int count)
    {
        if (isNoThrow)
        {
            return;
        }

        s_throwCount = count - 1;
    }

    static void TryThrow()
    {
        if (isNoThrow)
        {
            return;
        }

        if (s_throwCount == 0)
        {
            s_throwCount--;
            throw SafetyException();
        }

        s_throwCount--;
    }

    int m_value{ 0 };
    VecTestStats m_stats{};

    static int s_throwCount;
};

template <>
int VecTesterBase<true>::s_throwCount = -1;
template <>
int VecTesterBase<false>::s_throwCount = -1;

using TestVectorIntegral = VectorTest<int>;

TEST_F(TestVectorIntegral, DefaultConstruct)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_EQ(vec.Capacity(), 0u);
}

TEST_F(TestVectorIntegral, InlineDefaultConstruct)
{
    rad::InlineVector<int, 10> vec;

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_EQ(vec.Capacity(), 10u);
}

TEST_F(TestVectorIntegral, AllocatorCopyConstruct)
{
    radtest::HeapResource heap;
    radtest::ResourceAllocator<radtest::HeapResource> alloc(heap);
    rad::Vector<int, decltype(alloc)> vec(alloc);

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_EQ(vec.Capacity(), 0u);

    EXPECT_EQ(heap.allocCount, 0);
    EXPECT_EQ(heap.freeCount, 0);
    EXPECT_EQ(vec.GetAllocator().m_res, &heap);
}

TEST_F(TestVectorIntegral, AllocatorMoveConstruct)
{
    using AllocWrap = radtest::ResourceAllocator<radtest::HeapResource>;

    radtest::HeapResource heap;
    rad::Vector<int, AllocWrap> vec(heap);

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_EQ(vec.Capacity(), 0u);

    EXPECT_EQ(heap.allocCount, 0);
    EXPECT_EQ(heap.freeCount, 0);
    EXPECT_EQ(vec.GetAllocator().m_res, &heap);
}

TEST_F(TestVectorIntegral, Reserve)
{
    using AllocWrap = radtest::ResourceAllocator<radtest::HeapResource>;

    radtest::HeapResource heap;
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

TEST_F(TestVectorIntegral, InlineReserve)
{
    using AllocWrap = radtest::ResourceAllocator<radtest::HeapResource>;

    radtest::HeapResource heap;
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

TEST_F(TestVectorIntegral, ReserveFail)
{
    using AllocWrap = radtest::ResourceAllocator<radtest::HeapResource>;

    radtest::HeapResource heap;
    rad::Vector<int, AllocWrap> vec(heap);

    heap.forceAllocFails = 1;
    EXPECT_TRUE(vec.Reserve(100).IsErr());

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_EQ(vec.Capacity(), 0u);
    EXPECT_EQ(heap.forceAllocFails, 0);
    EXPECT_EQ(heap.allocCount, 0);
}

TEST_F(TestVectorIntegral, InlineReserveFail)
{
    using AllocWrap = radtest::ResourceAllocator<radtest::HeapResource>;

    radtest::HeapResource heap;
    rad::InlineVector<int, 10, AllocWrap> vec(heap);

    heap.forceAllocFails = 1;
    EXPECT_TRUE(vec.Reserve(100).IsErr());

    EXPECT_TRUE(vec.Empty());
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_EQ(vec.Capacity(), decltype(vec)::InlineCount);
    EXPECT_EQ(heap.forceAllocFails, 0);
    EXPECT_EQ(heap.allocCount, 0);
}

TEST_F(TestVectorIntegral, PushBack)
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

TEST_F(TestVectorIntegral, PushBackCopy)
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

TEST_F(TestVectorIntegral, EmplaceBack)
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

TEST_F(TestVectorIntegral, FrontBack)
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

TEST_F(TestVectorIntegral, At)
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

TEST_F(TestVectorIntegral, Subscript)
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

TEST_F(TestVectorIntegral, Data)
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

TEST_F(TestVectorIntegral, PopBack)
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

TEST_F(TestVectorIntegral, Resize)
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

TEST_F(TestVectorIntegral, ResizeCopy)
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

TEST_F(TestVectorIntegral, Clear)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.Resize(5).IsOk());

    EXPECT_EQ(vec.Size(), 5u);

    vec.Clear();

    EXPECT_EQ(vec.Size(), 0u);
}

TEST_F(TestVectorIntegral, ShrinkToFit)
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

TEST_F(TestVectorIntegral, ShrinkToFitInline)
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

TEST_F(TestVectorIntegral, AssignCopy)
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

TEST_F(TestVectorIntegral, AssignInit)
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

TEST_F(TestVectorIntegral, AssignRange)
{
    rad::Vector<int> other;

    EXPECT_TRUE(other.Assign({ 1, 2, 3 }).IsOk());

    rad::Vector<int> vec;

    EXPECT_TRUE(vec.Assign(other.ToSpan()).IsOk());

    EXPECT_EQ(vec.Size(), other.Size());
    EXPECT_EQ(vec.Front(), other.Front());
    EXPECT_EQ(vec.Back(), other.Back());
}

TEST_F(TestVectorIntegral, Swap)
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

TEST_F(TestVectorIntegral, InlineSwapBothInline)
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

TEST_F(TestVectorIntegral, InlineSwapNeitherInline)
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

TEST_F(TestVectorIntegral, InlineSwapDisjoint)
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

TEST_F(TestVectorIntegral, Copy)
{
    rad::Vector<int> vec;
    rad::Vector<int> other;

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());

    EXPECT_TRUE(vec.Copy(vec).IsOk());

    EXPECT_EQ(vec.Size(), 3u);
    EXPECT_EQ(vec.Front(), 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec.Back(), 3);

    EXPECT_TRUE(vec.Copy(other).IsOk());

    EXPECT_EQ(vec.Size(), other.Size());
    EXPECT_EQ(1, other.Front());
    EXPECT_EQ(3, other.Back());
}

TEST_F(TestVectorIntegral, CopyOverwrite)
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

TEST_F(TestVectorIntegral, Move)
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

#if defined(RAD_GCC_VERSION) && RAD_GCC_VERSION >= 130000
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"
#endif
#ifdef RAD_CLANG_VERSION
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
#endif

    // self move assign
    other = std::move(other);
    EXPECT_EQ(other.Size(), 3u);
    EXPECT_EQ(other.Front(), 1);
    EXPECT_EQ(other.Back(), 3);

    vec = std::move(vec);
    EXPECT_EQ(vec.Size(), 0u);

#ifdef RAD_CLANG_VERSION
#pragma clang diagnostic pop
#endif
#if defined(RAD_GCC_VERSION) && RAD_GCC_VERSION >= 130000
#pragma GCC diagnostic pop
#endif
}

TEST_F(TestVectorIntegral, Double)
{
    rad::Vector<double> vec;

    EXPECT_TRUE(vec.Assign({ 1.1, 2.2, 3.3 }).IsOk());

    EXPECT_EQ(vec.At(0), 1.1);
    EXPECT_EQ(vec.At(1), 2.2);
    EXPECT_EQ(vec.At(2), 3.3);
}

TEST_F(TestVectorIntegral, Float)
{
    rad::Vector<float> vec;

    EXPECT_TRUE(vec.Assign({ 1.1f, 2.2f, 3.3f }).IsOk());

    EXPECT_EQ(vec.At(0), 1.1f);
    EXPECT_EQ(vec.At(1), 2.2f);
    EXPECT_EQ(vec.At(2), 3.3f);
}

TEST_F(TestVectorIntegral, TakeBack)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());

    EXPECT_EQ(vec.Size(), 3u);
    EXPECT_EQ(vec.TakeBack(), 3);
    EXPECT_EQ(vec.Size(), 2u);
    EXPECT_EQ(vec.TakeBack(), 2);
    EXPECT_EQ(vec.Size(), 1u);
    EXPECT_EQ(vec.TakeBack(), 1);
    EXPECT_EQ(vec.Size(), 0u);
    EXPECT_TRUE(vec.Empty());
}

TEST_F(TestVectorIntegral, Seek)
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

TEST_F(TestVectorIntegral, SeekFront)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.SeekFront().IsErr());

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());

    EXPECT_EQ(vec.SeekFront().Ok(), 1);

    const auto& other = vec;

    EXPECT_EQ(other.SeekFront(), 1);
}

TEST_F(TestVectorIntegral, SeekBack)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.SeekBack().IsErr());

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());

    EXPECT_EQ(vec.SeekBack().Ok(), 3);

    const auto& other = vec;

    EXPECT_EQ(other.SeekBack(), 3);
}

TEST_F(TestVectorIntegral, MoveOperator)
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

TEST_F(TestVectorIntegral, AssignDown)
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

TEST_F(TestVectorIntegral, AssignOverlapping)
{
    rad::Vector<int> vec;

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());

    EXPECT_EQ(vec.Assign(vec.ToSpan()).Err(), rad::Error::InvalidAddress);
    EXPECT_EQ(vec.Assign(vec.ToSpan(0, 1)).Err(), rad::Error::InvalidAddress);
    EXPECT_EQ(vec.Assign(vec.ToSpan(2, 1)).Err(), rad::Error::InvalidAddress);
    EXPECT_EQ(vec.Assign(vec.ToSpan(2)).Err(), rad::Error::InvalidAddress);
}

TEST_F(TestVectorIntegral, ResizeSame)
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

TEST_F(TestVectorIntegral, ShrinkToFitNoMemory)
{
    using AllocWrap = radtest::ResourceAllocator<radtest::HeapResource>;

    radtest::HeapResource heap;
    rad::Vector<int, AllocWrap> vec(heap);

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());

    vec.PopBack();

    heap.forceAllocFails = 1;
    EXPECT_EQ(vec.ShrinkToFit().Err(), rad::Error::NoMemory);

    EXPECT_EQ(vec.Size(), 2u);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
}

TEST_F(TestVectorIntegral, InlineShrinkToFitNoMemory)
{
    using AllocWrap = radtest::ResourceAllocator<radtest::HeapResource>;

    radtest::HeapResource heap;
    rad::Vector<int, AllocWrap, 1> vec(heap);

    EXPECT_TRUE(vec.Assign({ 1, 2, 3 }).IsOk());

    vec.PopBack();

    heap.forceAllocFails = 1;
    EXPECT_EQ(vec.ShrinkToFit().Err(), rad::Error::NoMemory);

    EXPECT_EQ(vec.Size(), 2u);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
}

TEST_F(TestVectorIntegral, CopyNoMemory)
{
    using AllocWrap = radtest::ResourceAllocator<radtest::HeapResource>;

    radtest::HeapResource heap;
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

TEST_F(TestVectorIntegral, ResizeNoMemory)
{
    using AllocWrap = radtest::ResourceAllocator<radtest::HeapResource>;

    radtest::HeapResource heap;
    rad::Vector<int, AllocWrap> vec(heap);

    heap.forceAllocFails = 1;
    EXPECT_EQ(vec.Resize(3).Err(), rad::Error::NoMemory);

    EXPECT_TRUE(vec.Empty());

    heap.forceAllocFails = 1;
    EXPECT_EQ(vec.Resize(3, 123).Err(), rad::Error::NoMemory);

    EXPECT_TRUE(vec.Empty());
}

#if RAD_ENABLE_STD

TEST_F(TestVectorIntegral, AssignNoMemory)
{
    using AllocWrap = radtest::ResourceAllocator<radtest::HeapResource>;

    radtest::HeapResource heap;
    rad::Vector<int, AllocWrap> vec(heap);

    heap.forceAllocFails = 1;
    EXPECT_EQ(vec.Assign(3, 123).Err(), rad::Error::NoMemory);

    EXPECT_TRUE(vec.Empty());

    heap.forceAllocFails = 1;
    EXPECT_EQ(vec.Assign({ 1, 2, 3 }).Err(), rad::Error::NoMemory);

    EXPECT_TRUE(vec.Empty());
}

#endif // RAD_ENABLE_STD

TEST_F(TestVectorIntegral, EmplaceBackNoMemory)
{
    using AllocWrap = radtest::ResourceAllocator<radtest::HeapResource>;

    radtest::HeapResource heap;
    rad::Vector<int, AllocWrap> vec(heap);

    heap.forceAllocFails = 1;
    EXPECT_EQ(vec.EmplaceBack(1).Err(), rad::Error::NoMemory);

    EXPECT_TRUE(vec.Empty());
}

#if RAD_ENABLE_STD

TEST_F(TestVectorIntegral, EqualityOperators)
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

TEST_F(TestVectorIntegral, ComparisonOperators)
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
}

#endif // RAD_ENABLE_STD

template <typename T>
using NonTrivialStruct = VectorTest<T>;

TYPED_TEST_SUITE_P(NonTrivialStruct);

TYPED_TEST_P(NonTrivialStruct, Reserve)
{
    rad::Vector<TypeParam> vec;

    EXPECT_TRUE(vec.Reserve(10).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);
}

TYPED_TEST_P(NonTrivialStruct, Resize)
{
    TypeParam value(123);
    rad::Vector<TypeParam> vec;

    VectorTest<TypeParam>::ResetStats();
    EXPECT_TRUE(vec.Resize(5).IsOk());

    EXPECT_EQ(g_stats.DtorCount, TypeParam::isNoThrow ? 1 : 6);
    EXPECT_EQ(g_stats.DefaultCtorCount, 1);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 5);
    EXPECT_EQ(g_stats.MoveCtorCount, TypeParam::isNoThrow ? 0 : 5);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    VectorTest<TypeParam>::ResetStats();
    EXPECT_TRUE(vec.Resize(5).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 1);
    EXPECT_EQ(g_stats.DefaultCtorCount, 1);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    VectorTest<TypeParam>::ResetStats();
    EXPECT_TRUE(vec.Resize(2).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 4);
    EXPECT_EQ(g_stats.DefaultCtorCount, 1);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    VectorTest<TypeParam>::ResetStats();
    EXPECT_TRUE(vec.Resize(2, value).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    VectorTest<TypeParam>::ResetStats();
    EXPECT_TRUE(vec.Resize(5, value).IsOk());

    EXPECT_EQ(g_stats.DtorCount, TypeParam::isNoThrow ? 0 : 3);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 3);
    EXPECT_EQ(g_stats.MoveCtorCount, TypeParam::isNoThrow ? 0 : 3);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    VectorTest<TypeParam>::ResetStats();
    EXPECT_TRUE(vec.Resize(10, value).IsOk());

    EXPECT_EQ(g_stats.DtorCount, TypeParam::isNoThrow ? 5 : 10);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 5);
    EXPECT_EQ(g_stats.MoveCtorCount, TypeParam::isNoThrow ? 5 : 10);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    auto allocator = radtest::OOMAllocator(TypeParam::isNoThrow ? 1 : 3);
    rad::Vector<TypeParam, radtest::OOMAllocator> fail(allocator);
    EXPECT_TRUE(fail.Resize(5, value).IsOk());

    EXPECT_EQ(fail.Resize(10, value), rad::Error::NoMemory);
    EXPECT_EQ(fail.Resize(20, value), rad::Error::NoMemory);
}

TYPED_TEST_P(NonTrivialStruct, Assign)
{
    TypeParam value(123);

    rad::Vector<TypeParam> vec;

    VectorTest<TypeParam>::ResetStats();
    EXPECT_TRUE(vec.Assign(10, value).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 10);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    value = TypeParam(456);

    VectorTest<TypeParam>::ResetStats();
    EXPECT_TRUE(vec.Assign(20, value).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 10);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 20);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    VectorTest<TypeParam>::ResetStats();
    EXPECT_TRUE(vec.Assign(5, value).IsOk());

    EXPECT_EQ(g_stats.DtorCount, TypeParam::isNoThrow ? 20 : 25);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 5);
    EXPECT_EQ(g_stats.MoveCtorCount, TypeParam::isNoThrow ? 0 : 5);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    rad::Vector<TypeParam, radtest::OOMAllocator> fail(
        radtest::OOMAllocator(0));
    EXPECT_EQ(fail.Assign(10, value), rad::Error::NoMemory);
}

TYPED_TEST_P(NonTrivialStruct, AssignSpan)
{
    TypeParam value(123);

    rad::Vector<TypeParam> vec;

    VectorTest<TypeParam>::ResetStats();
    EXPECT_TRUE(vec.Assign(10, value).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 10);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    value = TypeParam(456);
    rad::Vector<TypeParam> spanVec;
    EXPECT_TRUE(spanVec.Assign(20, value).IsOk());

    VectorTest<TypeParam>::ResetStats();

    EXPECT_TRUE(vec.Assign(spanVec.ToSpan()).IsOk());

    ASSERT_EQ(vec.Size(), 20UL);
    for (auto& tester : vec.ToSpan())
    {
        EXPECT_EQ(tester.m_value, 456);
    }

    EXPECT_EQ(g_stats.DtorCount, TypeParam::isNoThrow ? 20 : 10);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 20);
    EXPECT_EQ(g_stats.MoveCtorCount, TypeParam::isNoThrow ? 10 : 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    EXPECT_TRUE(spanVec.Assign(5, value).IsOk());

    VectorTest<TypeParam>::ResetStats();
    EXPECT_TRUE(vec.Assign(spanVec.ToSpan()).IsOk());

    EXPECT_EQ(g_stats.DtorCount, TypeParam::isNoThrow ? 20 : 25);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 5);
    EXPECT_EQ(g_stats.MoveCtorCount, TypeParam::isNoThrow ? 0 : 5);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    ASSERT_TRUE(vec.Reserve(20));
    EXPECT_TRUE(spanVec.Assign(15, value).IsOk());

    VectorTest<TypeParam>::ResetStats();
    EXPECT_TRUE(vec.Assign(spanVec.ToSpan()).IsOk());

    EXPECT_EQ(g_stats.DtorCount, TypeParam::isNoThrow ? 5 : 20);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 15);
    EXPECT_EQ(g_stats.MoveCtorCount, TypeParam::isNoThrow ? 0 : 15);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    VectorTest<TypeParam>::ResetStats();
    EXPECT_FALSE(vec.Assign(vec.ToSpan().Subspan(0, 5)).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    rad::Vector<TypeParam, radtest::OOMAllocator> fail(
        radtest::OOMAllocator(0));
    EXPECT_EQ(fail.Assign(spanVec.ToSpan()), rad::Error::NoMemory);
}

TYPED_TEST_P(NonTrivialStruct, Clear)
{
    TypeParam value(123);

    rad::Vector<TypeParam> vec;

    VectorTest<TypeParam>::ResetStats();
    EXPECT_TRUE(vec.Assign(10, value).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 10);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    VectorTest<TypeParam>::ResetStats();
    vec.Clear();

    EXPECT_EQ(g_stats.DtorCount, 10);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);
}

TYPED_TEST_P(NonTrivialStruct, PushBackLVal)
{
    TypeParam value(123);

    rad::Vector<TypeParam> vec;

    VectorTest<TypeParam>::ResetStats();
    EXPECT_TRUE(vec.PushBack(value).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 1);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);
}

TYPED_TEST_P(NonTrivialStruct, PushBackRVal)
{
    TypeParam value(123);

    rad::Vector<TypeParam> vec;

    VectorTest<TypeParam>::ResetStats();
    EXPECT_TRUE(vec.PushBack(rad::Move(value)).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 1);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);
}

TYPED_TEST_P(NonTrivialStruct, PushEmplaceBack)
{
    rad::Vector<TypeParam> vec;

    VectorTest<TypeParam>::ResetStats();
    EXPECT_TRUE(vec.EmplaceBack(123).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 1);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);
}

TYPED_TEST_P(NonTrivialStruct, Copy)
{
    TypeParam value(123);

    rad::Vector<TypeParam> vec;

    EXPECT_TRUE(vec.Assign(10, value).IsOk());

    rad::Vector<TypeParam> other;

    VectorTest<TypeParam>::ResetStats();
    EXPECT_TRUE(vec.Copy(other).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 10);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    vec.PopBack();
    VectorTest<TypeParam>::ResetStats();
    EXPECT_TRUE(vec.Copy(other).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 10);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 9);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    VectorTest<TypeParam>::ResetStats();
    EXPECT_TRUE(vec.Copy(vec).IsOk());
    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    rad::Vector<TypeParam, radtest::OOMAllocator> good(
        radtest::OOMAllocator(10));
    EXPECT_TRUE(good.Assign(10, value).IsOk());

    rad::Vector<TypeParam, radtest::OOMAllocator> fail(
        radtest::OOMAllocator(0));
    EXPECT_EQ(good.Copy(fail), rad::Error::NoMemory);
}

TYPED_TEST_P(NonTrivialStruct, Move)
{
    TypeParam value(123);

    rad::Vector<TypeParam> vec;

    EXPECT_TRUE(vec.Assign(10, value).IsOk());

    rad::Vector<TypeParam> other;

    VectorTest<TypeParam>::ResetStats();
    vec.Move(other);

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);
}

TYPED_TEST_P(NonTrivialStruct, ShrinkToFit)
{
    TypeParam value(123);

    rad::Vector<TypeParam> vec;

    EXPECT_TRUE(vec.Assign(10, value).IsOk());

    vec.PopBack();

    VectorTest<TypeParam>::ResetStats();
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

    VectorTest<TypeParam>::ResetStats();
    EXPECT_TRUE(vec.ShrinkToFit().IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);
}

TYPED_TEST_P(NonTrivialStruct, Swap)
{
    TypeParam value(123);

    rad::Vector<TypeParam> vec;

    EXPECT_TRUE(vec.Assign(10, value).IsOk());

    rad::Vector<TypeParam> other;

    VectorTest<TypeParam>::ResetStats();
    vec.Swap(other);

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);
}

TYPED_TEST_P(NonTrivialStruct, InlineShrinkToFit)
{
    TypeParam value(123);

    rad::InlineVector<TypeParam, 5> vec;

    EXPECT_TRUE(vec.Assign(10, value).IsOk());

    vec.PopBack();

    VectorTest<TypeParam>::ResetStats();
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

    VectorTest<TypeParam>::ResetStats();
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

    VectorTest<TypeParam>::ResetStats();
    EXPECT_TRUE(vec.ShrinkToFit().IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);
}

TYPED_TEST_P(NonTrivialStruct, InlineSwap)
{
    TypeParam value(123);

    rad::InlineVector<TypeParam, 5> vec;

    EXPECT_TRUE(vec.Assign(10, value).IsOk());

    rad::InlineVector<TypeParam, 5> other;

    EXPECT_TRUE(other.Assign(10, value).IsOk());

    VectorTest<TypeParam>::ResetStats();
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

    VectorTest<TypeParam>::ResetStats();
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

    VectorTest<TypeParam>::ResetStats();
    vec.Swap(other);

    EXPECT_EQ(g_stats.DtorCount, 15);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 15);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);
}

using ThrowingVecTester = VecTesterBase<false>;
using NonThrowingVecTester = VecTesterBase<true>;

REGISTER_TYPED_TEST_SUITE_P(NonTrivialStruct,
                            Reserve,
                            Resize,
                            Assign,
                            AssignSpan,
                            Clear,
                            PushBackLVal,
                            PushBackRVal,
                            PushEmplaceBack,
                            Copy,
                            Move,
                            ShrinkToFit,
                            Swap,
                            InlineShrinkToFit,
                            InlineSwap);

using Types = testing::Types<ThrowingVecTester, NonThrowingVecTester>;

INSTANTIATE_TYPED_TEST_SUITE_P(TestVector, NonTrivialStruct, Types, );

struct VecTestStruct
{
    uint64_t UInt64;
    double Double;
};

TEST(TestVectorTrivialStruct, PushBack)
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

TEST(TestVectorTrivialStruct, Pointers)
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
        EXPECT_EQ(entry->UInt64, 9UL);
        EXPECT_EQ(entry->Double, static_cast<double>(9));
    }
}

// Exception Safety Tests
// We provide the strong guarantee for Vector.  To do this we require that the
// contained types have noexcept move, swap, and destruction
//
// The methods of vector that require special consideration in regard to
// exception safety are Resize, Assign, EmplaceBack, PushBack, and Copy.
using TestVectorStrongGuarantee = VectorTest<ThrowingVecTester>;

TEST_F(TestVectorStrongGuarantee, ResizeDefaultConstructed)
{
    rad::Vector<ThrowingVecTester> vec;
    ThrowingVecTester::ThrowIn(3);
    EXPECT_THROW(vec.Resize(5).IsOk(), SafetyException);

    EXPECT_EQ(vec.Size(), 0UL);
    EXPECT_EQ(g_stats.DtorCount, 2);
    EXPECT_EQ(g_stats.DefaultCtorCount, 1);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 1);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    ResetStats();
    EXPECT_TRUE(vec.Resize(5).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 6);
    EXPECT_EQ(g_stats.DefaultCtorCount, 1);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 5);
    EXPECT_EQ(g_stats.MoveCtorCount, 5);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);
}

TEST_F(TestVectorStrongGuarantee, ResizeItemConstructed)
{
    ThrowingVecTester value(123);
    rad::Vector<ThrowingVecTester> vec;

    ResetStats();
    ThrowingVecTester::ThrowIn(2);

    EXPECT_THROW(vec.Resize(5, value).IsOk(), SafetyException);

    EXPECT_EQ(vec.Size(), 0UL);
    EXPECT_EQ(g_stats.DtorCount, 1);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 1);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    ResetStats();
    EXPECT_TRUE(vec.Resize(5, value).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 5);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 5);
    EXPECT_EQ(g_stats.MoveCtorCount, 5);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);
}

TEST_F(TestVectorStrongGuarantee, Assign)
{
    ThrowingVecTester value(123);

    rad::Vector<ThrowingVecTester> vec;

    ResetStats();
    EXPECT_TRUE(vec.Assign(10, value).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 10);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    value = ThrowingVecTester(456);

    ResetStats();
    ThrowingVecTester::ThrowIn(5);
    EXPECT_THROW(vec.Assign(20, value).IsOk(), SafetyException);

    ASSERT_EQ(vec.Size(), 10UL);
    for (auto& tester : vec.ToSpan())
    {
        EXPECT_EQ(tester.m_value, 123);
    }

    EXPECT_EQ(g_stats.DtorCount, 4);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 4);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    ResetStats();
    ThrowingVecTester::ThrowIn(3);
    EXPECT_THROW(vec.Assign(5, value).IsOk(), SafetyException);

    ASSERT_EQ(vec.Size(), 10UL);
    for (auto& tester : vec.ToSpan())
    {
        EXPECT_EQ(tester.m_value, 123);
    }

    EXPECT_EQ(g_stats.DtorCount, 2);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 2);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    ASSERT_TRUE(vec.Reserve(20));
    ResetStats();
    ThrowingVecTester::ThrowIn(2);
    EXPECT_THROW(vec.Assign(15, value).IsOk(), SafetyException);

    ASSERT_EQ(vec.Size(), 10UL);
    for (auto& tester : vec.ToSpan())
    {
        EXPECT_EQ(tester.m_value, 123);
    }

    EXPECT_EQ(g_stats.DtorCount, 1);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 1);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);
}

TEST_F(TestVectorStrongGuarantee, AssignSpan)
{
    ThrowingVecTester value(123);

    rad::Vector<ThrowingVecTester> vec;

    ResetStats();
    EXPECT_TRUE(vec.Assign(10, value).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 10);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    value = ThrowingVecTester(456);
    rad::Vector<ThrowingVecTester> spanVec;
    EXPECT_TRUE(spanVec.Assign(20, value).IsOk());

    ResetStats();

    ThrowingVecTester::ThrowIn(5);
    EXPECT_THROW(vec.Assign(spanVec.ToSpan()).IsOk(), SafetyException);

    ASSERT_EQ(vec.Size(), 10UL);
    for (auto& tester : vec.ToSpan())
    {
        EXPECT_EQ(tester.m_value, 123);
    }

    EXPECT_EQ(g_stats.DtorCount, 4);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 4);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    EXPECT_TRUE(spanVec.Assign(5, value).IsOk());

    ResetStats();
    ThrowingVecTester::ThrowIn(3);
    EXPECT_THROW(vec.Assign(spanVec.ToSpan()).IsOk(), SafetyException);

    ASSERT_EQ(vec.Size(), 10UL);
    for (auto& tester : vec.ToSpan())
    {
        EXPECT_EQ(tester.m_value, 123);
    }

    EXPECT_EQ(g_stats.DtorCount, 2);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 2);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    ASSERT_TRUE(vec.Reserve(20));
    EXPECT_TRUE(spanVec.Assign(15, value).IsOk());

    ResetStats();
    ThrowingVecTester::ThrowIn(2);
    EXPECT_THROW(vec.Assign(spanVec.ToSpan()).IsOk(), SafetyException);

    ASSERT_EQ(vec.Size(), 10UL);
    for (auto& tester : vec.ToSpan())
    {
        EXPECT_EQ(tester.m_value, 123);
    }

    EXPECT_EQ(g_stats.DtorCount, 1);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 1);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);
}

#if RAD_ENABLE_STD

TEST_F(TestVectorStrongGuarantee, AssignIList)
{
    ThrowingVecTester value(123);

    rad::Vector<ThrowingVecTester> vec;

    ResetStats();
    EXPECT_TRUE(vec.Assign(5, value).IsOk());

    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 5);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    ThrowingVecTester v(456);
    ResetStats();

    ThrowingVecTester::ThrowIn(8);
    EXPECT_THROW(vec.Assign({ v, v, v, v, v, v }).IsOk(), SafetyException);

    ASSERT_EQ(vec.Size(), 5UL);
    for (auto& tester : vec.ToSpan())
    {
        EXPECT_EQ(tester.m_value, 123);
    }

    EXPECT_EQ(g_stats.DtorCount, 7);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 7);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    ResetStats();
    ThrowingVecTester::ThrowIn(5);
    EXPECT_THROW(vec.Assign({ v, v, v }).IsOk(), SafetyException);

    ASSERT_EQ(vec.Size(), 5UL);
    for (auto& tester : vec.ToSpan())
    {
        EXPECT_EQ(tester.m_value, 123);
    }

    EXPECT_EQ(g_stats.DtorCount, 4);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 4);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);

    ASSERT_TRUE(vec.Reserve(20));

    ResetStats();
    ThrowingVecTester::ThrowIn(8);
    EXPECT_THROW(vec.Assign({ v, v, v, v, v, v }).IsOk(), SafetyException);

    ASSERT_EQ(vec.Size(), 5UL);
    for (auto& tester : vec.ToSpan())
    {
        EXPECT_EQ(tester.m_value, 123);
    }

    EXPECT_EQ(g_stats.DtorCount, 7);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 7);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);
}

#endif // RAD_ENABLE_STD

TEST_F(TestVectorStrongGuarantee, EmplaceBack)
{
    rad::Vector<ThrowingVecTester> vec;

    ResetStats();
    ThrowingVecTester::ThrowIn(1);
    EXPECT_THROW(vec.EmplaceBack(123).IsOk(), SafetyException);

    EXPECT_EQ(vec.Size(), 0UL);
    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);
}

TEST_F(TestVectorStrongGuarantee, PushBackLVal)
{
    ThrowingVecTester value(123);

    rad::Vector<ThrowingVecTester> vec;

    ResetStats();
    ThrowingVecTester::ThrowIn(1);
    EXPECT_THROW(vec.PushBack(value).IsOk(), SafetyException);

    EXPECT_EQ(vec.Size(), 0UL);
    EXPECT_EQ(g_stats.DtorCount, 0);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 0);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);
}

TEST_F(TestVectorStrongGuarantee, Copy)
{
    ThrowingVecTester value(123);
    rad::Vector<ThrowingVecTester> vec;

    EXPECT_TRUE(vec.Assign(10, value).IsOk());

    ThrowingVecTester otherValue(456);
    rad::Vector<ThrowingVecTester> other;

    EXPECT_TRUE(other.Assign(2, otherValue).IsOk());

    ResetStats();
    ThrowingVecTester::ThrowIn(6);
    EXPECT_THROW(vec.Copy(other).IsOk(), SafetyException);

    EXPECT_EQ(other.Size(), 2UL);
    for (auto& tester : other.ToSpan())
    {
        EXPECT_EQ(tester.m_value, 456);
    }

    EXPECT_EQ(g_stats.DtorCount, 5);
    EXPECT_EQ(g_stats.DefaultCtorCount, 0);
    EXPECT_EQ(g_stats.CtorCount, 0);
    EXPECT_EQ(g_stats.CopyCtorCount, 5);
    EXPECT_EQ(g_stats.MoveCtorCount, 0);
    EXPECT_EQ(g_stats.CopyAssignCount, 0);
    EXPECT_EQ(g_stats.MoveAssignCount, 0);
}

#ifdef RAD_GCC_VERSION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmultichar"
#endif
TEST(VectorTest, TroubleAllocators)
{
    {
        using StickyVec = rad::Vector<int, radtest::StickyTaggedAllocator>;
        StickyVec vec1(radtest::StickyTaggedAllocator{ 'tst1' });
        EXPECT_TRUE(vec1.PushBack(1).IsOk());

        StickyVec vec2(rad::Move(vec1));
        EXPECT_TRUE(vec1.Empty());
        EXPECT_EQ(vec2.Size(), 1u);

        StickyVec vec3(radtest::StickyTaggedAllocator{ 'tst3' });
        EXPECT_TRUE(vec2.Copy(vec3).IsOk());
        EXPECT_EQ(vec2.Size(), 1u);
        EXPECT_EQ(vec2.GetAllocator().m_tag, uint32_t('tst1'));
        EXPECT_EQ(vec3.Size(), 1u);
        EXPECT_EQ(vec3.GetAllocator().m_tag, uint32_t('tst3'));

        StickyVec vec3a(radtest::StickyTaggedAllocator{ 'tst4' });
        ASSERT_TRUE(vec3a.PushBack(42).IsOk());
        ASSERT_TRUE(vec3a.PushBack(99).IsOk());
        EXPECT_TRUE(vec2.Copy(vec3a).IsOk());
        EXPECT_EQ(vec2.Size(), 1u);
        EXPECT_EQ(vec2.GetAllocator().m_tag, uint32_t('tst1'));
        EXPECT_EQ(vec3a.Size(), 1u);
        EXPECT_EQ(vec3a.GetAllocator().m_tag, uint32_t('tst4'));

        // These all static_assert, as they should
        // vec1 = std::move(vec2);
        // vec1.Swap(vec2);
        // vec1.Move(vec2);
        // vec1.Clone();
    }
    {
        using StickyVecNt =
            rad::Vector<ThrowingVecTester, radtest::StickyTaggedAllocator>;
        StickyVecNt vec1(radtest::StickyTaggedAllocator{ 'tst1' });
        EXPECT_TRUE(vec1.PushBack(ThrowingVecTester(1)).IsOk());

        StickyVecNt vec2(rad::Move(vec1));
        EXPECT_TRUE(vec1.Empty());
        EXPECT_EQ(vec2.Size(), 1u);

        StickyVecNt vec3(radtest::StickyTaggedAllocator{ 'tst3' });
        EXPECT_TRUE(vec2.Copy(vec3).IsOk());
        EXPECT_EQ(vec2.Size(), 1u);
        EXPECT_EQ(vec2.GetAllocator().m_tag, uint32_t('tst1'));
        EXPECT_EQ(vec3.Size(), 1u);
        EXPECT_EQ(vec3.GetAllocator().m_tag, uint32_t('tst3'));

        StickyVecNt vec3a(radtest::StickyTaggedAllocator{ 'tst4' });
        ASSERT_TRUE(vec3a.PushBack(ThrowingVecTester(42)).IsOk());
        ASSERT_TRUE(vec3a.PushBack(ThrowingVecTester(99)).IsOk());
        EXPECT_TRUE(vec2.Copy(vec3a).IsOk());
        EXPECT_EQ(vec2.Size(), 1u);
        EXPECT_EQ(vec2.GetAllocator().m_tag, uint32_t('tst1'));
        EXPECT_EQ(vec3a.Size(), 1u);
        EXPECT_EQ(vec3a.GetAllocator().m_tag, uint32_t('tst4'));

        // These all static_assert, as they should
        // vec1 = std::move(vec2);
        // vec1.Swap(vec2);
        // vec1.Move(vec2);
        // vec1.Clone();
    }
    {
        using StickyDefaultVec =
            rad::Vector<int, radtest::StickyDefaultTaggedAllocator>;
        StickyDefaultVec vec1(radtest::StickyDefaultTaggedAllocator{ 'tst1' });
        EXPECT_TRUE(vec1.PushBack(1).IsOk());

        StickyDefaultVec vec2(std::move(vec1));
        EXPECT_TRUE(vec1.Empty());
        EXPECT_EQ(vec1.GetAllocator().m_tag, uint32_t('tst1'));
        EXPECT_EQ(vec2.Size(), 1u);
        EXPECT_EQ(vec2.GetAllocator().m_tag, uint32_t('tst1'));

        StickyDefaultVec vec3(radtest::StickyDefaultTaggedAllocator{ 'tst3' });
        EXPECT_TRUE(vec2.Copy(vec3).IsOk());
        EXPECT_EQ(vec2.Size(), 1u);
        EXPECT_EQ(vec2.GetAllocator().m_tag, uint32_t('tst1'));
        EXPECT_EQ(vec3.Size(), 1u);
        EXPECT_EQ(vec3.GetAllocator().m_tag, uint32_t('tst3'));

        StickyDefaultVec vec3a(radtest::StickyDefaultTaggedAllocator{ 'tst4' });
        ASSERT_TRUE(vec3a.PushBack(42).IsOk());
        ASSERT_TRUE(vec3a.PushBack(99).IsOk());
        EXPECT_TRUE(vec2.Copy(vec3a).IsOk());
        EXPECT_EQ(vec2.Size(), 1u);
        EXPECT_EQ(vec2.GetAllocator().m_tag, uint32_t('tst1'));
        EXPECT_EQ(vec3a.Size(), 1u);
        EXPECT_EQ(vec3a.GetAllocator().m_tag, uint32_t('tst4'));

        auto expected_vec4 = vec2.Clone();
        EXPECT_EQ(vec2.Size(), 1u);
        EXPECT_EQ(expected_vec4.Ok().Size(), 1u);
        EXPECT_EQ(expected_vec4.Ok().GetAllocator().m_tag, uint32_t(1234));

        // These all static_assert, as they should
        // vec1 = std::move(vec2);
        // vec1.Swap(vec2);
        // vec1.Move(vec2);
    }

    {
        using MovingVec = rad::Vector<int, radtest::MovingTaggedAllocator>;
        MovingVec vec1(radtest::MovingTaggedAllocator{ 'abcd' });
        EXPECT_TRUE(vec1.PushBack(1).IsOk());

        MovingVec vec2(rad::Move(vec1));
        EXPECT_TRUE(vec1.Empty());
        EXPECT_EQ(vec2.Size(), 1u);

        MovingVec vec3(radtest::MovingTaggedAllocator{ 'tst3' });
        EXPECT_TRUE(vec2.Copy(vec3).IsOk());
        EXPECT_EQ(vec2.Size(), 1u);
        EXPECT_EQ(vec2.GetAllocator().m_tag, uint32_t('abcd'));
        EXPECT_EQ(vec3.Size(), 1u);
        EXPECT_EQ(vec3.GetAllocator().m_tag, uint32_t('abcd'));

        MovingVec vec3a(radtest::MovingTaggedAllocator{ 'tst4' });
        ASSERT_TRUE(vec3a.PushBack(42).IsOk());
        ASSERT_TRUE(vec3a.PushBack(99).IsOk());
        EXPECT_TRUE(vec2.Copy(vec3a).IsOk());
        EXPECT_EQ(vec2.Size(), 1u);
        EXPECT_EQ(vec2.GetAllocator().m_tag, uint32_t('abcd'));
        EXPECT_EQ(vec3a.Size(), 1u);
        EXPECT_EQ(vec3a.GetAllocator().m_tag, uint32_t('abcd'));

        MovingVec vec4(radtest::MovingTaggedAllocator{ 'wxyz' });
        EXPECT_TRUE(vec1.PushBack(99).IsOk());
        vec1.Swap(vec4);
        EXPECT_TRUE(vec1.Empty());
        EXPECT_EQ(vec4.Size(), 1u);
        EXPECT_EQ(vec1.GetAllocator().m_tag, uint32_t('wxyz'));
        EXPECT_EQ(vec4.GetAllocator().m_tag, uint32_t('abcd'));

        vec1 = rad::Move(vec4);
        EXPECT_TRUE(vec4.Empty());
        EXPECT_EQ(vec1.Size(), 1u);
        EXPECT_EQ(vec1.GetAllocator().m_tag, uint32_t('abcd'));
        EXPECT_EQ(vec4.GetAllocator().m_tag, uint32_t('abcd'));

        MovingVec vec5(radtest::MovingTaggedAllocator{ 'crwd' });
        vec1.Move(vec5);
        EXPECT_TRUE(vec1.Empty());
        EXPECT_EQ(vec5.Size(), 1u);
        EXPECT_EQ(vec1.GetAllocator().m_tag, uint32_t('abcd'));
        EXPECT_EQ(vec5.GetAllocator().m_tag, uint32_t('abcd'));
    }
    {
        using MovingVecNt =
            rad::Vector<ThrowingVecTester, radtest::MovingTaggedAllocator>;
        MovingVecNt vec1(radtest::MovingTaggedAllocator{ 'abcd' });
        EXPECT_TRUE(vec1.PushBack(ThrowingVecTester(1)).IsOk());

        MovingVecNt vec2(rad::Move(vec1));
        EXPECT_TRUE(vec1.Empty());
        EXPECT_EQ(vec2.Size(), 1u);

        MovingVecNt vec3(radtest::MovingTaggedAllocator{ 'tst3' });
        EXPECT_TRUE(vec2.Copy(vec3).IsOk());
        EXPECT_EQ(vec2.Size(), 1u);
        EXPECT_EQ(vec2.GetAllocator().m_tag, uint32_t('abcd'));
        EXPECT_EQ(vec3.Size(), 1u);
        EXPECT_EQ(vec3.GetAllocator().m_tag, uint32_t('abcd'));

        MovingVecNt vec3a(radtest::MovingTaggedAllocator{ 'tst4' });
        ASSERT_TRUE(vec3a.PushBack(ThrowingVecTester(42)).IsOk());
        ASSERT_TRUE(vec3a.PushBack(ThrowingVecTester(99)).IsOk());
        EXPECT_TRUE(vec2.Copy(vec3a).IsOk());
        EXPECT_EQ(vec2.Size(), 1u);
        EXPECT_EQ(vec2.GetAllocator().m_tag, uint32_t('abcd'));
        EXPECT_EQ(vec3a.Size(), 1u);
        EXPECT_EQ(vec3a.GetAllocator().m_tag, uint32_t('abcd'));

        MovingVecNt vec4(radtest::MovingTaggedAllocator{ 'wxyz' });
        EXPECT_TRUE(vec1.PushBack(ThrowingVecTester(99)).IsOk());
        vec1.Swap(vec4);
        EXPECT_TRUE(vec1.Empty());
        EXPECT_EQ(vec4.Size(), 1u);
        EXPECT_EQ(vec1.GetAllocator().m_tag, uint32_t('wxyz'));
        EXPECT_EQ(vec4.GetAllocator().m_tag, uint32_t('abcd'));

        vec1 = rad::Move(vec4);
        EXPECT_TRUE(vec4.Empty());
        EXPECT_EQ(vec1.Size(), 1u);
        EXPECT_EQ(vec1.GetAllocator().m_tag, uint32_t('abcd'));
        EXPECT_EQ(vec4.GetAllocator().m_tag, uint32_t('abcd'));

        MovingVecNt vec5(radtest::MovingTaggedAllocator{ 'crwd' });
        vec1.Move(vec5);
        EXPECT_TRUE(vec1.Empty());
        EXPECT_EQ(vec5.Size(), 1u);
        EXPECT_EQ(vec1.GetAllocator().m_tag, uint32_t('abcd'));
        EXPECT_EQ(vec5.GetAllocator().m_tag, uint32_t('abcd'));
    }

    {
        using TypedVec = rad::Vector<int, radtest::TypedAllocator>;
        TypedVec vec1;
        EXPECT_TRUE(vec1.PushBack(1).IsOk());

        TypedVec vec2(rad::Move(vec1));
        EXPECT_TRUE(vec1.Empty());
        EXPECT_EQ(vec2.Size(), 1u);

        TypedVec vec3;
        EXPECT_TRUE(vec2.Copy(vec3).IsOk());
        EXPECT_EQ(vec2.Size(), 1u);
        EXPECT_EQ(vec3.Size(), 1u);

        vec1 = rad::Move(vec2);
        EXPECT_TRUE(vec2.Empty());
        EXPECT_EQ(vec1.Size(), 1u);

        vec1.Swap(vec2);
        EXPECT_TRUE(vec1.Empty());
        EXPECT_EQ(vec2.Size(), 1u);

        TypedVec vec4;
        vec2.Move(vec4);
        EXPECT_TRUE(vec2.Empty());
        EXPECT_EQ(vec4.Size(), 1u);
    }

    {
        int value = 42;
        rad::Vector<int, radtest::MoveOOMAllocator> good(
            radtest::MoveOOMAllocator(1, 1234)); // you get one good allocation
        EXPECT_TRUE(good.Assign(10, value).IsOk());

        rad::Vector<int, radtest::MoveOOMAllocator> fail(
            radtest::MoveOOMAllocator(99, 5678)); // not ever used
        EXPECT_EQ(good.Copy(fail), rad::Error::NoMemory);
        EXPECT_EQ(fail.GetAllocator().m_id, 5678);

        rad::Vector<int, radtest::MoveOOMAllocator> pass(
            radtest::MoveOOMAllocator(99, 5678));
        EXPECT_TRUE(pass.Assign(10, value).IsOk());
        EXPECT_TRUE(pass.Copy(good).IsOk());
        EXPECT_EQ(good.GetAllocator().m_id, 5678);
    }
}
#ifdef RAD_GCC_VERSION
#pragma GCC diagnostic pop
#endif
