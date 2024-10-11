// Copyright 2024 The Radiant Authors.
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
#define RAD_DEFAULT_ALLOCATOR radtest::Allocator

#include <radiant/List.h>

#include "test/TestAlloc.h"

#include <gtest/gtest.h>

#include <utility>

using namespace testing;

// ensure no_unique_address is doing what we want
static_assert(sizeof(rad::ListUntyped) == 2 * sizeof(void*));
static_assert(sizeof(rad::List<int>) == 2 * sizeof(void*));
static_assert(sizeof(rad::ListUntyped) == sizeof(rad::List<int>));

// TODO: don't forget to test iterator operations and iterator stability!

template <typename T, typename Alloc>
void ListEqual(const rad::List<T, Alloc>& list,
               std::initializer_list<T> init_list)
{
    size_t expensive_size = list.ExpensiveSize();
    EXPECT_EQ(expensive_size, init_list.size());
    EXPECT_EQ(expensive_size == 0, list.Empty());
    auto it = list.begin();
    for (const T& elt : init_list)
    {
        EXPECT_EQ(elt, *it);
        ++it;
    }
    EXPECT_EQ(it, list.end());
}

// for use with ImmovableStruct and friends
template <typename T, typename Alloc, typename U>
void ListValEqual(const rad::List<T, Alloc>& list,
                  std::initializer_list<U> init_list)
{
    size_t expensive_size = list.ExpensiveSize();
    EXPECT_EQ(expensive_size, init_list.size());
    EXPECT_EQ(expensive_size == 0, list.Empty());
    auto it = list.begin();
    for (const U& elt : init_list)
    {
        EXPECT_EQ(elt, it->val);
        ++it;
    }
    EXPECT_EQ(it, list.end());
}

TEST(ListTest, DefaultConstructIsEmpty)
{
    rad::List<int> i;
    EXPECT_TRUE(i.Empty());
    EXPECT_TRUE(i.begin() == i.end());
    EXPECT_TRUE(i.cbegin() == i.cend());
    const rad::List<int> j;
    EXPECT_TRUE(j.Empty());
    EXPECT_TRUE(j.begin() == j.end());
    EXPECT_TRUE(j.cbegin() == j.cend());
}

TEST(ListTest, AllocatorConstructors)
{
    using StatefulAlloc = radtest::StatefulAllocator<int>;
    {
        rad::List<int, StatefulAlloc> default_ctor;
        StatefulAlloc default_ctor_alloc = default_ctor.GetAllocator();
        EXPECT_EQ(default_ctor_alloc.m_state, 0u);
    }
    {
        // allocator constructor
        StatefulAlloc source_alloc;
        source_alloc.m_state = 42;
        rad::List<int, StatefulAlloc> alloc_ctor(source_alloc);
        StatefulAlloc alloc = alloc_ctor.GetAllocator();
        EXPECT_EQ(alloc.m_state, 42u);

        // regular move constructor needs to steal the original allocator
        rad::List<int, StatefulAlloc> moving_alloc_ctor(std::move(alloc_ctor));
        StatefulAlloc moved_from_alloc = alloc_ctor.GetAllocator();
        // Don't care about the moved from state, so long as it isn't the
        // original
        EXPECT_NE(moved_from_alloc.m_state, 42u);
        StatefulAlloc moved_to_alloc = moving_alloc_ctor.GetAllocator();
        EXPECT_EQ(moved_to_alloc.m_state, 42u);

        rad::List<int, StatefulAlloc> move_assigned;
        move_assigned = std::move(moving_alloc_ctor);
        StatefulAlloc assigned_from_alloc = moving_alloc_ctor.GetAllocator();
        // Don't care about the moved from state, so long as it isn't the
        // original
        EXPECT_NE(assigned_from_alloc.m_state, 42u);
        StatefulAlloc assigned_to_alloc = move_assigned.GetAllocator();
        EXPECT_EQ(assigned_to_alloc.m_state, 42u);
    }
}

TEST(ListTest, TestPushBackFailureRecovery)
{
    radtest::HeapAllocator heap;
    radtest::AllocWrapper<int, radtest::HeapAllocator> alloc(heap);

    {
        rad::List<int, radtest::AllocWrapper<int, radtest::HeapAllocator>> list(
            alloc);

        EXPECT_TRUE(list.PushBack(1).IsOk());
        EXPECT_EQ(heap.allocCount, 1);
        EXPECT_EQ(heap.freeCount, 0);

        heap.forceAllocFails = 1;
        EXPECT_EQ(rad::Error::NoMemory, list.PushBack(2));
        EXPECT_EQ(heap.allocCount, 1);
        EXPECT_EQ(heap.freeCount, 0);
        ListEqual(list, { 1 });

        EXPECT_TRUE(list.PushBack(3).IsOk());
        EXPECT_EQ(heap.allocCount, 2);
        EXPECT_EQ(heap.freeCount, 0);
        ListEqual(list, { 1, 3 });
    }
    EXPECT_EQ(heap.allocCount, 2);
    EXPECT_EQ(heap.freeCount, 2);
}

TEST(ListTest, DefaultConstructClear)
{
    rad::List<int> i;
    i.Clear();
    EXPECT_TRUE(i.Empty());
}

struct CopyStruct
{
    int val = 42;
};

TEST(ListTest, CopyPushBack)
{
    rad::List<CopyStruct> i;
    CopyStruct local;
    local.val = 42;
    EXPECT_TRUE(i.PushBack(local).IsOk());
    ListValEqual(i, { 42 });

    local.val = 99;
    EXPECT_TRUE(i.PushBack(local).IsOk());
    ListValEqual(i, { 42, 99 });

    local.val = 77;
    EXPECT_TRUE(i.PushBack(local).IsOk());
    ListValEqual(i, { 42, 99, 77 });
}

struct MoveStruct
{
    MoveStruct() = default;
    MoveStruct(const MoveStruct&) = delete;
    MoveStruct& operator=(const MoveStruct&) = delete;

    MoveStruct(MoveStruct&& other) noexcept
        : val(other.val)
    {
        other.val = -1;
    }

    MoveStruct& operator=(MoveStruct&& other) noexcept
    {
        val = other.val;
        other.val = -2;
    }

    int val = 42;
};

TEST(ListTest, MovePushBack)
{
    rad::List<MoveStruct> i;
    MoveStruct local;
    EXPECT_TRUE(i.PushBack(std::move(local)).IsOk());
    ListValEqual(i, { 42 });

    local.val = 99;
    EXPECT_TRUE(i.PushBack(std::move(local)).IsOk());
    ListValEqual(i, { 42, 99 });

    local.val = 77;
    EXPECT_TRUE(i.PushBack(std::move(local)).IsOk());
    ListValEqual(i, { 42, 99, 77 });
}

struct ImmovableStruct
{
    explicit ImmovableStruct(int x)
        : val(x)
    {
    }

    ImmovableStruct() = delete;
    ImmovableStruct(const ImmovableStruct&) = delete;
    ImmovableStruct& operator=(const ImmovableStruct&) = delete;

    ImmovableStruct(ImmovableStruct&&) = delete;
    ImmovableStruct& operator=(ImmovableStruct&&) = delete;
    const int val;
};

TEST(ListTest, ImmovableEmplaceBack)
{
    rad::List<ImmovableStruct> i;
    EXPECT_TRUE(i.EmplaceBack(42).IsOk());
    ListValEqual(i, { 42 });

    EXPECT_TRUE(i.EmplaceBack(99).IsOk());
    ListValEqual(i, { 42, 99 });

    EXPECT_TRUE(i.EmplaceBack(77).IsOk());
    ListValEqual(i, { 42, 99, 77 });
}

TEST(ListTest, MoveConstruct)
{
    {
        rad::List<int> empty;
        rad::List<int> move_from_empty(std::move(empty));
        ListEqual(empty, {});
        ListEqual(move_from_empty, {});
    }
    {
        rad::List<int> one;
        EXPECT_TRUE(one.PushBack(1).IsOk());
        rad::List<int> move_from_one(std::move(one));

        ListEqual(one, {});
        ListEqual(move_from_one, { 1 });
    }
    {
        rad::List<int> two;
        EXPECT_TRUE(two.PushBack(1).IsOk());
        EXPECT_TRUE(two.PushBack(2).IsOk());

        rad::List<int> move_from_two(std::move(two));
        ListEqual(two, {});
        ListEqual(move_from_two, { 1, 2 });
    }
    {
        rad::List<int> three;
        EXPECT_TRUE(three.PushBack(1).IsOk());
        EXPECT_TRUE(three.PushBack(2).IsOk());
        EXPECT_TRUE(three.PushBack(3).IsOk());

        rad::List<int> move_from_three(std::move(three));
        ListEqual(three, {});
        ListEqual(move_from_three, { 1, 2, 3 });
    }
    {
        rad::List<int> one;
        EXPECT_TRUE(one.PushBack(1).IsOk());

        rad::List<int> move_from_one(std::move(one));

        // ensure we can still mutate after moves
        EXPECT_TRUE(one.PushBack(101).IsOk());
        EXPECT_TRUE(move_from_one.PushBack(201).IsOk());
        ListEqual(one, { 101 });
        ListEqual(move_from_one, { 1, 201 });
    }
}

TEST(ListTest, ClearSome)
{
    rad::List<int> i;
    EXPECT_TRUE(i.PushBack(1).IsOk());
    EXPECT_FALSE(i.Empty());
    i.Clear();
    EXPECT_TRUE(i.Empty());

    EXPECT_TRUE(i.PushBack(2).IsOk());
    EXPECT_TRUE(i.PushBack(3).IsOk());
    EXPECT_EQ(i.ExpensiveSize(), 2);
    i.Clear();
    EXPECT_TRUE(i.Empty());

    EXPECT_TRUE(i.PushBack(4).IsOk());
    EXPECT_TRUE(i.PushBack(5).IsOk());
    EXPECT_TRUE(i.PushBack(6).IsOk());
    EXPECT_EQ(i.ExpensiveSize(), 3);
    i.Clear();
    EXPECT_TRUE(i.Empty());
}

#if RAD_WINDOWS
// MSVC warns that the body of the first for loop is unreachabe, which is true.
// But that's also what the test is trying to ensure.  So disable the unhelpful
// warning.  Doing this for the whole function, because narrower scoped
// disables aren't getting accepted.
#pragma warning(push)
#pragma warning(disable : 4702) // C4702: unreachable code
#endif
TEST(ListTest, RangeForLoop)
{
    rad::List<int> input;

    for (int elt : input)
    {
        (void)elt;
        FAIL() << "should be empty";
    }

    EXPECT_TRUE(input.PushBack(0).IsOk());
    EXPECT_TRUE(input.PushBack(1).IsOk());
    EXPECT_TRUE(input.PushBack(2).IsOk());

    // ensure that the references we get are "real"
    int i = 0;
    for (int& elt : input)
    {
        EXPECT_EQ(elt, i);
        elt = 100 + i;
        ++i;
    }

    i = 100;
    for (int elt : input)
    {
        EXPECT_EQ(elt, i);
        ++i;
    }
}
#if RAD_WINDOWS
#pragma warning(pop)
#endif
