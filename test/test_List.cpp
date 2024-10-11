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
static_assert(sizeof(rad::List_untyped) == 2 * sizeof(void*));
static_assert(sizeof(rad::List<int>) == 2 * sizeof(void*));
static_assert(sizeof(rad::List_untyped) == sizeof(rad::List<int>));

// TODO: don't forget to test iterator operations and iterator stability!

template <typename T, typename Alloc>
void ListEqual(const rad::List<T, Alloc>& dlist,
               std::initializer_list<T> init_list)
{
    size_t expensive_size = dlist.expensive_size();
    EXPECT_EQ(expensive_size, init_list.size());
    EXPECT_EQ(expensive_size == 0, dlist.empty());
    auto it = dlist.begin();
    for (const T& elt : init_list)
    {
        EXPECT_EQ(elt, *it);
        ++it;
    }
    EXPECT_EQ(it, dlist.end());
}

// for use with ImmovableStruct and friends
template <typename T, typename Alloc, typename U>
void ListValEqual(const rad::List<T, Alloc>& dlist,
                  std::initializer_list<U> init_list)
{
    size_t expensive_size = dlist.expensive_size();
    EXPECT_EQ(expensive_size, init_list.size());
    EXPECT_EQ(expensive_size == 0, dlist.empty());
    auto it = dlist.begin();
    for (const U& elt : init_list)
    {
        EXPECT_EQ(elt, it->val);
        ++it;
    }
    EXPECT_EQ(it, dlist.end());
}

TEST(ListTest, DefaultConstructIsEmpty)
{
    rad::List<int> i;
    EXPECT_TRUE(i.empty());
    EXPECT_TRUE(i.begin() == i.end());
    EXPECT_TRUE(i.cbegin() == i.cend());
    const rad::List<int> j;
    EXPECT_TRUE(j.empty());
    EXPECT_TRUE(j.begin() == j.end());
    EXPECT_TRUE(j.cbegin() == j.cend());
}

TEST(ListTest, AllocatorConstructors)
{
    using StatefulAlloc = radtest::StatefulAllocator<int>;
    {
        rad::List<int, StatefulAlloc> default_ctor;
        StatefulAlloc default_ctor_alloc = default_ctor.get_allocator();
        EXPECT_EQ(default_ctor_alloc.m_state, 0u);
    }
    {
        // allocator constructor
        StatefulAlloc source_alloc;
        source_alloc.m_state = 42;
        rad::List<int, StatefulAlloc> alloc_ctor(source_alloc);
        StatefulAlloc alloc = alloc_ctor.get_allocator();
        EXPECT_EQ(alloc.m_state, 42u);

        // regular move constructor needs to steal the original allocator
        rad::List<int, StatefulAlloc> moving_alloc_ctor(std::move(alloc_ctor));
        StatefulAlloc moved_from_alloc = alloc_ctor.get_allocator();
        // Don't care about the moved from state, so long as it isn't the
        // original
        EXPECT_NE(moved_from_alloc.m_state, 42u);
        StatefulAlloc moved_to_alloc = moving_alloc_ctor.get_allocator();
        EXPECT_EQ(moved_to_alloc.m_state, 42u);

        rad::List<int, StatefulAlloc> move_assigned;
        move_assigned = std::move(moving_alloc_ctor);
        StatefulAlloc assigned_from_alloc = moving_alloc_ctor.get_allocator();
        // Don't care about the moved from state, so long as it isn't the
        // original
        EXPECT_NE(assigned_from_alloc.m_state, 42u);
        StatefulAlloc assigned_to_alloc = move_assigned.get_allocator();
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

        EXPECT_TRUE(list.push_back(1).IsOk());
        EXPECT_EQ(heap.allocCount, 1);
        EXPECT_EQ(heap.freeCount, 0);

        heap.forceAllocFails = 1;
        EXPECT_EQ(rad::Error::NoMemory, list.push_back(2));
        EXPECT_EQ(heap.allocCount, 1);
        EXPECT_EQ(heap.freeCount, 0);
        ListEqual(list, { 1 });

        EXPECT_TRUE(list.push_back(3).IsOk());
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
    i.clear();
    EXPECT_TRUE(i.empty());
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
    EXPECT_TRUE(i.push_back(local).IsOk());
    ListValEqual(i, { 42 });

    local.val = 99;
    EXPECT_TRUE(i.push_back(local).IsOk());
    ListValEqual(i, { 42, 99 });

    local.val = 77;
    EXPECT_TRUE(i.push_back(local).IsOk());
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
    EXPECT_TRUE(i.push_back(std::move(local)).IsOk());
    ListValEqual(i, { 42 });

    local.val = 99;
    EXPECT_TRUE(i.push_back(std::move(local)).IsOk());
    ListValEqual(i, { 42, 99 });

    local.val = 77;
    EXPECT_TRUE(i.push_back(std::move(local)).IsOk());
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
    EXPECT_TRUE(i.emplace_back(42).IsOk());
    ListValEqual(i, { 42 });

    EXPECT_TRUE(i.emplace_back(99).IsOk());
    ListValEqual(i, { 42, 99 });

    EXPECT_TRUE(i.emplace_back(77).IsOk());
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
        EXPECT_TRUE(one.push_back(1).IsOk());
        rad::List<int> move_from_one(std::move(one));

        ListEqual(one, {});
        ListEqual(move_from_one, { 1 });
    }
    {
        rad::List<int> two;
        EXPECT_TRUE(two.push_back(1).IsOk());
        EXPECT_TRUE(two.push_back(2).IsOk());

        rad::List<int> move_from_two(std::move(two));
        ListEqual(two, {});
        ListEqual(move_from_two, { 1, 2 });
    }
    {
        rad::List<int> three;
        EXPECT_TRUE(three.push_back(1).IsOk());
        EXPECT_TRUE(three.push_back(2).IsOk());
        EXPECT_TRUE(three.push_back(3).IsOk());

        rad::List<int> move_from_three(std::move(three));
        ListEqual(three, {});
        ListEqual(move_from_three, { 1, 2, 3 });
    }
    {
        rad::List<int> one;
        EXPECT_TRUE(one.push_back(1).IsOk());

        rad::List<int> move_from_one(std::move(one));

        // ensure we can still mutate after moves
        EXPECT_TRUE(one.push_back(101).IsOk());
        EXPECT_TRUE(move_from_one.push_back(201).IsOk());
        ListEqual(one, { 101 });
        ListEqual(move_from_one, { 1, 201 });
    }
}

TEST(ListTest, ClearSome)
{
    rad::List<int> i;
    EXPECT_TRUE(i.push_back(1).IsOk());
    EXPECT_FALSE(i.empty());
    i.clear();
    EXPECT_TRUE(i.empty());

    EXPECT_TRUE(i.push_back(2).IsOk());
    EXPECT_TRUE(i.push_back(3).IsOk());
    EXPECT_EQ(i.expensive_size(), 2);
    i.clear();
    EXPECT_TRUE(i.empty());

    EXPECT_TRUE(i.push_back(4).IsOk());
    EXPECT_TRUE(i.push_back(5).IsOk());
    EXPECT_TRUE(i.push_back(6).IsOk());
    EXPECT_EQ(i.expensive_size(), 3);
    i.clear();
    EXPECT_TRUE(i.empty());
}
