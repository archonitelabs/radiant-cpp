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
#define RAD_DEFAULT_ALLOCATOR radtest::Mallocator
#include "test/TestAlloc.h"
#include "test/TestInputStringLiteralRange.h"

#include "radiant/List.h"
#include "radiant/Span.h"

#include <array>
#include <gtest/gtest.h>
#include <string>
#include <utility>

using namespace testing;

// ensure no_unique_address is doing what we want
RAD_S_ASSERTMSG(sizeof(rad::detail::ListUntyped) == 2 * sizeof(void*),
                "unexpected object size");
RAD_S_ASSERTMSG(sizeof(rad::List<int>) == 2 * sizeof(void*),
                "unexpected object size");
RAD_S_ASSERTMSG(sizeof(rad::detail::ListUntyped) == sizeof(rad::List<int>),
                "unexpected object size");

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
    using StatefulAlloc = radtest::StatefulAllocator;
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

        // regular move constructor needs to copy the original allocator
        rad::List<int, StatefulAlloc> moving_alloc_ctor(std::move(alloc_ctor));
        StatefulAlloc moved_from_alloc = alloc_ctor.GetAllocator();
        EXPECT_EQ(moved_from_alloc.m_state, 42u);
        StatefulAlloc moved_to_alloc = moving_alloc_ctor.GetAllocator();
        EXPECT_EQ(moved_to_alloc.m_state, 42u);

        StatefulAlloc source_alloc2;
        source_alloc2.m_state = 99;
        rad::List<int, StatefulAlloc> move_assigned(source_alloc2);
        // moving_alloc_ctor's "42" allocator will propagate to move_assigned
        move_assigned = std::move(moving_alloc_ctor);
        StatefulAlloc assigned_from_alloc = moving_alloc_ctor.GetAllocator();

        EXPECT_EQ(assigned_from_alloc.m_state, 42U);
        StatefulAlloc assigned_to_alloc = move_assigned.GetAllocator();
        EXPECT_EQ(assigned_to_alloc.m_state, 42u);
    }
}

TEST(ListTest, PushBackFailureRecovery)
{
    radtest::HeapResource heap;
    radtest::ResourceAllocator<radtest::HeapResource> alloc(heap);

    {
        rad::List<int, radtest::ResourceAllocator<radtest::HeapResource>> list(
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
    CopyStruct() = default;
    CopyStruct(const CopyStruct&) = default;
    CopyStruct& operator=(const CopyStruct&) = default;

    // intentionally noexcept(false)
    CopyStruct(CopyStruct&& other)
        : val(other.val)
    {
        other.val = -1;
    }

    // intentionally noexcept(false)
    CopyStruct& operator=(CopyStruct&& other)
    {
        val = other.val;
        other.val = -1;
        return *this;
    }
};

TEST(ListTest, CopyPushBack)
{
    rad::List<CopyStruct> i;
    CopyStruct local;
    local.val = 42;
    EXPECT_TRUE(i.PushBack(local).IsOk());
    ListValEqual(i, { 42 });
    EXPECT_EQ(local.val, 42);

    local.val = 99;
    EXPECT_TRUE(i.PushBack(local).IsOk());
    ListValEqual(i, { 42, 99 });
    EXPECT_EQ(local.val, 99);

    local.val = 77;
    EXPECT_TRUE(i.PushBack(local).IsOk());
    ListValEqual(i, { 42, 99, 77 });
    EXPECT_EQ(local.val, 77);
}

TEST(ListTest, CopyPushFront)
{
    rad::List<CopyStruct> i;
    CopyStruct local;
    local.val = 42;
    EXPECT_TRUE(i.PushFront(local).IsOk());
    ListValEqual(i, { 42 });
    EXPECT_EQ(local.val, 42);

    local.val = 99;
    EXPECT_TRUE(i.PushFront(local).IsOk());
    ListValEqual(i, { 99, 42 });
    EXPECT_EQ(local.val, 99);

    local.val = 77;
    EXPECT_TRUE(i.PushFront(local).IsOk());
    ListValEqual(i, { 77, 99, 42 });
    EXPECT_EQ(local.val, 77);
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
        return *this;
    }

    int val = 42;
};

TEST(ListTest, MovePushBack)
{
    {
        rad::List<MoveStruct> i;
        MoveStruct local;
        EXPECT_TRUE(i.PushBack(std::move(local)).IsOk());
        ListValEqual(i, { 42 });
        EXPECT_EQ(local.val, -1);

        local.val = 99;
        EXPECT_TRUE(i.PushBack(std::move(local)).IsOk());
        ListValEqual(i, { 42, 99 });
        EXPECT_EQ(local.val, -1);

        local.val = 77;
        EXPECT_TRUE(i.PushBack(std::move(local)).IsOk());
        ListValEqual(i, { 42, 99, 77 });
        EXPECT_EQ(local.val, -1);
    }
    {
        rad::List<rad::List<int>> lli;
        rad::List<int> ints;

        EXPECT_TRUE(
            ints.AssignRange(std::initializer_list<int>{ 1, 2, 3 }).IsOk());
        EXPECT_TRUE(lli.PushBack(std::move(ints)).IsOk());
        EXPECT_TRUE(ints.Empty());
        EXPECT_EQ(lli.ExpensiveSize(), 1u);
        ListEqual(*lli.begin(), { 1, 2, 3 });

        EXPECT_TRUE(
            ints.AssignRange(std::initializer_list<int>{ 4, 5, 6 }).IsOk());
        EXPECT_TRUE(lli.PushBack(std::move(ints)).IsOk());
        EXPECT_TRUE(ints.Empty());
        EXPECT_EQ(lli.ExpensiveSize(), 2u);
        ListEqual(*lli.begin(), { 1, 2, 3 });
        ListEqual(*++lli.begin(), { 4, 5, 6 });
    }
}

TEST(ListTest, MovePushFront)
{
    rad::List<MoveStruct> i;
    MoveStruct local;
    EXPECT_TRUE(i.PushFront(std::move(local)).IsOk());
    ListValEqual(i, { 42 });
    EXPECT_EQ(local.val, -1);

    local.val = 99;
    EXPECT_TRUE(i.PushFront(std::move(local)).IsOk());
    ListValEqual(i, { 99, 42 });
    EXPECT_EQ(local.val, -1);

    local.val = 77;
    EXPECT_TRUE(i.PushFront(std::move(local)).IsOk());
    ListValEqual(i, { 77, 99, 42 });
    EXPECT_EQ(local.val, -1);
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

TEST(ListTest, ImmovableEmplaceFront)
{
    rad::List<ImmovableStruct> i;
    EXPECT_TRUE(i.EmplaceFront(42).IsOk());
    ListValEqual(i, { 42 });

    EXPECT_TRUE(i.EmplaceFront(99).IsOk());
    ListValEqual(i, { 99, 42 });

    EXPECT_TRUE(i.EmplaceFront(77).IsOk());
    ListValEqual(i, { 77, 99, 42 });
}

#if RAD_ENABLE_STD
TEST(ListTest, AssignInitList)
{
    rad::List<int> li;
    EXPECT_TRUE(li.AssignInitializerList({}).IsOk());
    EXPECT_TRUE(li.Empty());

    EXPECT_TRUE(li.AssignInitializerList({ 42 }).IsOk());
    EXPECT_EQ(1u, li.ExpensiveSize());

    EXPECT_TRUE(li.AssignInitializerList({ 100, 101, 102, 103 }).IsOk());
    EXPECT_EQ(4u, li.ExpensiveSize());

    int i = 100;
    for (int elt : li)
    {
        EXPECT_EQ(i, elt);
        ++i;
    }

    EXPECT_TRUE(li.AssignInitializerList({}).IsOk());
    EXPECT_TRUE(li.Empty());
}
#endif

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
        EXPECT_TRUE(two.AssignRange(std::initializer_list<int>{ 1, 2 }).IsOk());

        rad::List<int> move_from_two(std::move(two));
        ListEqual(two, {});
        ListEqual(move_from_two, { 1, 2 });
    }
    {
        rad::List<int> three;
        EXPECT_TRUE(
            three.AssignRange(std::initializer_list<int>{ 1, 2, 3 }).IsOk());

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

TEST(ListTest, MoveAssign)
{
    {
        rad::List<int> empty;
        rad::List<int> move_from_empty;
        move_from_empty = std::move(empty);
        ListEqual(empty, {});
        ListEqual(move_from_empty, {});
    }
    {
        rad::List<int> one;
        EXPECT_TRUE(one.PushBack(1).IsOk());
        rad::List<int> move_from_one;

        move_from_one = std::move(one);

        ListEqual(one, {});
        ListEqual(move_from_one, { 1 });
    }
    {
        rad::List<int> two;
        EXPECT_TRUE(two.AssignRange(std::initializer_list<int>{ 1, 2 }).IsOk());

        rad::List<int> move_from_two;

        move_from_two = std::move(two);
        ListEqual(two, {});
        ListEqual(move_from_two, { 1, 2 });
    }
    {
        rad::List<int> one;
        EXPECT_TRUE(one.PushBack(1).IsOk());

        rad::List<int> move_from_one;
        move_from_one = std::move(one);

        // ensure we can still mutate after moves
        EXPECT_TRUE(one.PushBack(101).IsOk());
        EXPECT_TRUE(move_from_one.PushBack(201).IsOk());
        ListEqual(one, { 101 });
        ListEqual(move_from_one, { 1, 201 });
    }
#if defined(RAD_GCC_VERSION) && RAD_GCC_VERSION >= 130000
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wself-move"
#endif
#ifdef RAD_CLANG_VERSION
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
#endif
    {
        rad::List<int> self_move;
        self_move = std::move(self_move);
        ListEqual(self_move, {});
    }
    {
        rad::List<int> self_move;
        EXPECT_TRUE(self_move.PushBack(101).IsOk());
        self_move = std::move(self_move);
        EXPECT_TRUE(self_move.PushBack(102).IsOk());
        ListEqual(self_move, { 101, 102 });
    }
#ifdef RAD_CLANG_VERSION
#pragma clang diagnostic pop
#endif
#if defined(RAD_GCC_VERSION) && RAD_GCC_VERSION >= 130000
#pragma GCC diagnostic pop
#endif
}

TEST(ListTest, ClearSome)
{
    rad::List<int> i;
    EXPECT_TRUE(i.PushBack(1).IsOk());
    EXPECT_FALSE(i.Empty());
    i.Clear();
    EXPECT_TRUE(i.Empty());

    EXPECT_TRUE(i.AssignRange(std::initializer_list<int>{ 2, 3 }).IsOk());
    EXPECT_EQ(i.ExpensiveSize(), 2u);
    i.Clear();
    EXPECT_TRUE(i.Empty());

    EXPECT_TRUE(i.AssignRange(std::initializer_list<int>{ 4, 5, 6 }).IsOk());
    EXPECT_EQ(i.ExpensiveSize(), 3u);
    i.Clear();
    EXPECT_TRUE(i.Empty());
}

#if RAD_WINDOWS
// MSVC warns that the body of the first for loop is unreachable, which is true.
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

    EXPECT_TRUE(
        input.AssignRange(std::initializer_list<int>{ 0, 1, 2 }).IsOk());

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

TEST(ListTest, AssignSome)
{
    rad::List<int> input;
    int* np = nullptr;
    EXPECT_TRUE(input.AssignSome(np, np).IsOk());
    EXPECT_TRUE(input.Empty());

    int arr[] = { 101, 203, 304 };
    EXPECT_TRUE(input.AssignSome(std::begin(arr), std::end(arr)).IsOk());
    ListEqual(input, { 101, 203, 304 });

    rad::List<int> new_copy;
    EXPECT_TRUE(new_copy.AssignSome(input.begin(), input.end()).IsOk());
    ListEqual(input, { 101, 203, 304 });
}

TEST(ListTest, AssignRange)
{
    rad::List<int> input;
    rad::Span<int> empty;

    EXPECT_TRUE(input.AssignRange(empty).IsOk());
    EXPECT_TRUE(input.Empty());

    std::array<int, 3> arr = { 101, 203, 304 };
    EXPECT_TRUE(input.AssignRange(arr).IsOk());
    ListEqual(input, { 101, 203, 304 });

    rad::List<int> new_copy;
    EXPECT_TRUE(new_copy.AssignRange(input).IsOk());
    ListEqual(input, { 101, 203, 304 });

    rad::List<char> chars;
    EXPECT_TRUE(
        chars.AssignRange(radtest::TestInputStringLiteralRange("abc")).IsOk());
    ListEqual(chars, { 'a', 'b', 'c' });
}

TEST(ListTest, AssignCount)
{
    rad::List<int> input;

    EXPECT_TRUE(input.AssignCount(0, 42).IsOk());
    EXPECT_TRUE(input.Empty());

    EXPECT_TRUE(input.AssignCount(3, 99).IsOk());
    ListEqual(input, { 99, 99, 99 });
}

TEST(ListTest, Emplace)
{
    radtest::HeapResource heap;
    radtest::ResourceAllocator<radtest::HeapResource> alloc(heap);
    rad::List<ImmovableStruct,
              radtest::ResourceAllocator<radtest::HeapResource>>
        input(alloc);

    // emplace at the end
    auto /* Res<IteratorType> */ iter = input.Emplace(input.end(), 42);
    ASSERT_TRUE(iter);
    EXPECT_TRUE(iter == --input.end());
    EXPECT_EQ(iter.Ok()->val, 42);
    iter = input.Emplace(input.end(), 43);
    ASSERT_TRUE(iter);
    EXPECT_TRUE(iter == --input.end());
    EXPECT_EQ(iter.Ok()->val, 43);
    ListValEqual(input, { 42, 43 });

    // emplace at the beginning
    iter = input.Emplace(input.begin(), 99);
    ASSERT_TRUE(iter);
    EXPECT_TRUE(iter == input.begin());
    EXPECT_EQ(iter.Ok()->val, 99);
    iter = input.Emplace(input.begin(), 100);
    ASSERT_TRUE(iter);
    EXPECT_TRUE(iter == input.begin());
    EXPECT_EQ(iter.Ok()->val, 100);
    ListValEqual(input, { 100, 99, 42, 43 });

    // emplace near the beginning
    auto old_iter = ++input.begin();
    iter = input.Emplace(old_iter, 23);
    ASSERT_TRUE(iter);
    EXPECT_TRUE(iter == ++input.begin());
    EXPECT_TRUE(iter != old_iter);
    ListValEqual(input, { 100, 23, 99, 42, 43 });

    // emplace near the end
    old_iter = --input.end();
    iter = input.Emplace(old_iter, 77);
    ASSERT_TRUE(iter);
    EXPECT_TRUE(iter == --(--input.end()));
    EXPECT_TRUE(iter != old_iter);
    ListValEqual(input, { 100, 23, 99, 42, 77, 43 });

    heap.forceAllocFails = 1;
    iter = input.Emplace(input.begin(), -1);
    EXPECT_TRUE(iter.IsErr());
    ListValEqual(input, { 100, 23, 99, 42, 77, 43 });
}

TEST(ListTest, MoveInsert)
{
    radtest::HeapResource heap;
    radtest::ResourceAllocator<radtest::HeapResource> alloc(heap);
    rad::List<MoveStruct, radtest::ResourceAllocator<radtest::HeapResource>>
        input(alloc);

    MoveStruct ms;
    ms.val = 42;
    // insert at the end
    auto /* Res<IteratorType> */ iter =
        input.Insert(input.end(), std::move(ms));
    ASSERT_TRUE(iter);
    EXPECT_TRUE(iter == --input.end());
    EXPECT_EQ(iter.Ok()->val, 42);
    EXPECT_EQ(ms.val, -1);

    ms.val = 43;
    iter = input.Insert(input.end(), std::move(ms));
    ASSERT_TRUE(iter);
    EXPECT_TRUE(iter == --input.end());
    EXPECT_EQ(iter.Ok()->val, 43);
    ListValEqual(input, { 42, 43 });
    EXPECT_EQ(ms.val, -1);

    // insert at the beginning
    ms.val = 99;
    iter = input.Insert(input.begin(), std::move(ms));
    ASSERT_TRUE(iter);
    EXPECT_TRUE(iter == input.begin());
    EXPECT_EQ(iter.Ok()->val, 99);
    EXPECT_EQ(ms.val, -1);

    ms.val = 100;
    iter = input.Insert(input.begin(), std::move(ms));
    ASSERT_TRUE(iter);
    EXPECT_TRUE(iter == input.begin());
    EXPECT_EQ(iter.Ok()->val, 100);
    ListValEqual(input, { 100, 99, 42, 43 });
    EXPECT_EQ(ms.val, -1);

    // insert near the beginning
    ms.val = 23;
    auto old_iter = ++input.begin();
    iter = input.Insert(old_iter, std::move(ms));
    ASSERT_TRUE(iter);
    EXPECT_TRUE(iter == ++input.begin());
    EXPECT_TRUE(iter != old_iter);
    ListValEqual(input, { 100, 23, 99, 42, 43 });
    EXPECT_EQ(ms.val, -1);

    // insert near the end
    ms.val = 77;
    old_iter = --input.end();
    iter = input.Insert(old_iter, std::move(ms));
    ASSERT_TRUE(iter);
    EXPECT_TRUE(iter == --(--input.end()));
    EXPECT_TRUE(iter != old_iter);
    ListValEqual(input, { 100, 23, 99, 42, 77, 43 });
    EXPECT_EQ(ms.val, -1);

    ms.val = -100;
    heap.forceAllocFails = 1;
    iter = input.Insert(input.begin(), std::move(ms));
    EXPECT_TRUE(iter.IsErr());
    ListValEqual(input, { 100, 23, 99, 42, 77, 43 });
    EXPECT_EQ(ms.val, -100);
}

TEST(ListTest, CopyInsert)
{
    radtest::HeapResource heap;
    radtest::ResourceAllocator<radtest::HeapResource> alloc(heap);
    rad::List<CopyStruct, radtest::ResourceAllocator<radtest::HeapResource>>
        input(alloc);

    CopyStruct cs;
    cs.val = 42;
    // insert at the end
    auto /* Res<IteratorType> */ iter = input.Insert(input.end(), cs);
    ASSERT_TRUE(iter);
    EXPECT_TRUE(iter == --input.end());
    EXPECT_EQ(iter.Ok()->val, 42);
    EXPECT_EQ(cs.val, 42);

    cs.val = 43;
    iter = input.Insert(input.end(), cs);
    ASSERT_TRUE(iter);
    EXPECT_TRUE(iter == --input.end());
    EXPECT_EQ(iter.Ok()->val, 43);
    ListValEqual(input, { 42, 43 });
    EXPECT_EQ(cs.val, 43);

    // insert at the beginning
    cs.val = 99;
    iter = input.Insert(input.begin(), cs);
    ASSERT_TRUE(iter);
    EXPECT_TRUE(iter == input.begin());
    EXPECT_EQ(iter.Ok()->val, 99);
    EXPECT_EQ(cs.val, 99);

    cs.val = 100;
    iter = input.Insert(input.begin(), cs);
    ASSERT_TRUE(iter);
    EXPECT_TRUE(iter == input.begin());
    EXPECT_EQ(iter.Ok()->val, 100);
    ListValEqual(input, { 100, 99, 42, 43 });
    EXPECT_EQ(cs.val, 100);

    // insert near the beginning
    cs.val = 23;
    auto old_iter = ++input.begin();
    iter = input.Insert(old_iter, cs);
    ASSERT_TRUE(iter);
    EXPECT_TRUE(iter == ++input.begin());
    EXPECT_TRUE(iter != old_iter);
    ListValEqual(input, { 100, 23, 99, 42, 43 });
    EXPECT_EQ(cs.val, 23);

    // insert near the end
    cs.val = 77;
    old_iter = --input.end();
    iter = input.Insert(old_iter, cs);
    ASSERT_TRUE(iter);
    EXPECT_TRUE(iter == --(--input.end()));
    EXPECT_TRUE(iter != old_iter);
    ListValEqual(input, { 100, 23, 99, 42, 77, 43 });
    EXPECT_EQ(cs.val, 77);

    cs.val = -100;
    heap.forceAllocFails = 1;
    iter = input.Insert(input.begin(), cs);
    EXPECT_TRUE(iter.IsErr());
    ListValEqual(input, { 100, 23, 99, 42, 77, 43 });
    EXPECT_EQ(cs.val, -100);
}

TEST(ListTest, AssignFailure)
{
    radtest::HeapResource heap;
    radtest::ResourceAllocator<radtest::HeapResource> alloc(heap);
    rad::List<int, radtest::ResourceAllocator<radtest::HeapResource>> list(
        alloc);

    // AssignCount fails back to empty when it starts empty
    heap.forceFutureAllocFail = 3;
    EXPECT_TRUE(list.AssignCount(5, 99).IsErr());
    EXPECT_EQ(heap.allocCount, 2);
    EXPECT_EQ(heap.freeCount, 2);
    EXPECT_TRUE(list.Empty());

    // AssignInitializerList fails back to empty when it starts empty
    heap.allocCount = heap.freeCount = 0;
    heap.forceFutureAllocFail = 3;
    EXPECT_TRUE(
        list.AssignRange(std::initializer_list<int>{ 1, 2, 3, 4, 5 }).IsErr());
    EXPECT_EQ(heap.allocCount, 2);
    EXPECT_EQ(heap.freeCount, 2);
    EXPECT_TRUE(list.Empty());

    // make sure nothing is corrupted
    EXPECT_TRUE(
        list.AssignRange(std::initializer_list<int>{ 1, 2, 3, 4, 5 }).IsOk());
    ListEqual(list, { 1, 2, 3, 4, 5 });
    auto old_begin = list.begin();
    auto old_end = list.end();
    int* first_addr = &*old_begin;

    // Original contents still in place if there's a failure
    heap.allocCount = heap.freeCount = 0;
    heap.forceFutureAllocFail = 5;
    EXPECT_TRUE(list.AssignCount(5, 99).IsErr());
    EXPECT_EQ(heap.allocCount, 4);
    EXPECT_EQ(heap.freeCount, 4);
    EXPECT_EQ(list.end(), old_end);
    EXPECT_EQ(list.begin(), old_begin);
    EXPECT_EQ(&*list.begin(), first_addr);
    ListEqual(list, { 1, 2, 3, 4, 5 });

    heap.allocCount = heap.freeCount = 0;
    heap.forceFutureAllocFail = 5;
    EXPECT_TRUE(
        list.AssignRange(std::initializer_list<int>{ 101, 102, 103, 104, 105 })
            .IsErr());
    EXPECT_EQ(heap.allocCount, 4);
    EXPECT_EQ(heap.freeCount, 4);
    EXPECT_EQ(list.end(), old_end);
    EXPECT_EQ(list.begin(), old_begin);
    EXPECT_EQ(&*list.begin(), first_addr);
    ListEqual(list, { 1, 2, 3, 4, 5 });
}

TEST(ListTest, PostIncrPostDecr)
{
    rad::List<int> data;
    EXPECT_TRUE(data.AssignRange(std::initializer_list<int>{ 0, 1, 2, 3 }));
    auto pre_begin = data.begin();
    auto post_begin = data.begin();
    EXPECT_EQ(pre_begin, post_begin++);
    EXPECT_EQ(*pre_begin, 0);
    EXPECT_EQ(*post_begin, 1);
    EXPECT_NE(pre_begin, post_begin);
    EXPECT_EQ(++pre_begin, post_begin);
    EXPECT_EQ(*pre_begin, 1);
    EXPECT_EQ(*post_begin, 1);
    EXPECT_EQ(*++pre_begin, 2);
    EXPECT_EQ(*post_begin++, 1);

    auto pre_end = --data.end();
    auto post_end = --data.end();
    EXPECT_EQ(pre_end, post_end--);
    EXPECT_EQ(*pre_end, 3);
    EXPECT_EQ(*post_end, 2);
    EXPECT_NE(pre_end, post_end);
    EXPECT_EQ(--pre_end, post_end);
    EXPECT_EQ(*pre_end, 2);
    EXPECT_EQ(*post_end, 2);
    EXPECT_EQ(*--pre_end, 1);
    EXPECT_EQ(*post_end--, 2);

    auto pre_cbegin = data.cbegin();
    auto post_cbegin = data.cbegin();
    EXPECT_EQ(pre_cbegin, post_cbegin++);
    EXPECT_EQ(*pre_cbegin, 0);
    EXPECT_EQ(*post_cbegin, 1);
    EXPECT_NE(pre_cbegin, post_cbegin);
    EXPECT_EQ(++pre_cbegin, post_cbegin);
    EXPECT_EQ(*pre_cbegin, 1);
    EXPECT_EQ(*post_cbegin, 1);
    EXPECT_EQ(*++pre_cbegin, 2);
    EXPECT_EQ(*post_cbegin++, 1);

    auto pre_cend = --data.cend();
    auto post_cend = --data.cend();
    EXPECT_EQ(pre_cend, post_cend--);
    EXPECT_EQ(*pre_cend, 3);
    EXPECT_EQ(*post_cend, 2);
    EXPECT_NE(pre_cend, post_cend);
    EXPECT_EQ(--pre_cend, post_cend);
    EXPECT_EQ(*pre_cend, 2);
    EXPECT_EQ(*post_cend, 2);
    EXPECT_EQ(*--pre_cend, 1);
    EXPECT_EQ(*post_cend--, 2);
}

#if !RAD_DBG
TEST(ListTest, SelfSplice)
{
    rad::List<int> list;
    EXPECT_TRUE(list.AssignRange(std::initializer_list<int>{ 1, 2, 3 }).IsOk());

    list.SpliceAll(list.end(), list);
    ListEqual(list, { 1, 2, 3 });
    list.SpliceAll(++list.begin(), list);
    ListEqual(list, { 1, 2, 3 });

    list.SpliceOne(list.end(), list, list.begin());
    ListEqual(list, { 1, 2, 3 });
    list.SpliceOne(list.begin(), list, ++list.begin());
    ListEqual(list, { 1, 2, 3 });

    list.SpliceSome(list.end(), list, list.begin(), list.end());
    ListEqual(list, { 1, 2, 3 });
    list.SpliceSome(list.begin(), list, list.begin(), ++list.begin());
    ListEqual(list, { 1, 2, 3 });
}
#endif

TEST(ListTest, SpliceSomeEmpties)
{
    std::array<int, 3> arr = { 101, 203, 304 };
    {
        // double empty
        rad::List<int> source;
        rad::List<int> dest;

        dest.SpliceSome(dest.begin(), source, source.begin(), source.end());
        EXPECT_TRUE(source.Empty());
        EXPECT_TRUE(dest.Empty());

        dest.SpliceSome(dest.begin(),
                        std::move(source),
                        source.begin(),
                        source.end());
        EXPECT_TRUE(source.Empty());
        EXPECT_TRUE(dest.Empty());

        dest.SpliceSome(dest.end(), source, source.begin(), source.end());
        EXPECT_TRUE(source.Empty());
        EXPECT_TRUE(dest.Empty());

        dest.SpliceSome(dest.end(),
                        std::move(source),
                        source.begin(),
                        source.end());
        EXPECT_TRUE(source.Empty());
        EXPECT_TRUE(dest.Empty());
    }

    {
        // source empty
        rad::List<int> source;
        rad::List<int> dest;

        EXPECT_TRUE(dest.AssignRange(arr).IsOk());
        ListEqual(dest, { 101, 203, 304 });

        dest.SpliceSome(dest.begin(), source, source.begin(), source.end());
        EXPECT_TRUE(source.Empty());
        ListEqual(dest, { 101, 203, 304 });

        dest.SpliceSome(dest.begin(),
                        std::move(source),
                        source.begin(),
                        source.end());
        EXPECT_TRUE(source.Empty());
        ListEqual(dest, { 101, 203, 304 });
    }

    {
        // dest empty lval
        rad::List<int> source;
        rad::List<int> dest;

        EXPECT_TRUE(source.AssignRange(arr).IsOk());
        ListEqual(source, { 101, 203, 304 });

        dest.SpliceSome(dest.begin(), source, source.begin(), source.end());
        EXPECT_TRUE(source.Empty());
        ListEqual(dest, { 101, 203, 304 });

        dest.Clear();
        EXPECT_TRUE(dest.Empty());
        EXPECT_TRUE(source.AssignRange(arr).IsOk());
        ListEqual(source, { 101, 203, 304 });

        dest.SpliceSome(dest.end(), source, source.begin(), source.end());
        EXPECT_TRUE(source.Empty());
        ListEqual(dest, { 101, 203, 304 });
    }

    {
        // dest empty rval
        rad::List<int> source;
        rad::List<int> dest;

        EXPECT_TRUE(source.AssignRange(arr).IsOk());
        ListEqual(source, { 101, 203, 304 });

        dest.SpliceSome(dest.begin(),
                        std::move(source),
                        source.begin(),
                        source.end());
        EXPECT_TRUE(source.Empty());
        ListEqual(dest, { 101, 203, 304 });

        dest.Clear();
        EXPECT_TRUE(dest.Empty());
        EXPECT_TRUE(source.AssignRange(arr).IsOk());
        ListEqual(source, { 101, 203, 304 });

        dest.SpliceSome(dest.end(),
                        std::move(source),
                        source.begin(),
                        source.end());
        EXPECT_TRUE(source.Empty());
        ListEqual(dest, { 101, 203, 304 });
    }
}

struct SpliceSomeExhaustive_data
{
    SpliceSomeExhaustive_data(
        int src_size, int dest_size, int src_begin, int src_end, int dest_pos)
        : src_size(src_size),
          dest_size(dest_size),
          src_begin(src_begin),
          src_end(src_end),
          dest_pos(dest_pos)
    {
    }

    int src_size;
    int dest_size;
    int src_begin;
    int src_end;
    int dest_pos;
    rad::List<int> source;
    rad::List<int> dest;
    rad::List<int>::IteratorType src_begin_iter;
    rad::List<int>::IteratorType src_end_iter;
    rad::List<int>::IteratorType dest_pos_iter;

    void BuildSrc()
    {
        src_begin_iter = source.end(); // .end() is the base case
        src_end_iter = source.end();
        for (int i = 0; i < src_size; ++i)
        {
            // populate with i offset by 100, so that we can tell src and dest
            // apart
            ASSERT_TRUE(source.PushBack(i + 100).IsOk());
            if (i == src_begin)
            {
                src_begin_iter = --source.end();
            }
            if (i == src_end)
            {
                src_end_iter = --source.end();
            }
        }
    }

    void BuildDest()
    {
        dest_pos_iter = dest.end(); // base case
        for (int i = 0; i < dest_size; ++i)
        {
            ASSERT_TRUE(dest.PushBack(i).IsOk());
            if (i == dest_pos)
            {
                dest_pos_iter = --dest.end();
            }
        }
    }

    void VerifySrc(const std::string& case_id)
    {
        EXPECT_EQ((int)source.ExpensiveSize(), src_size - (src_end - src_begin))
            << case_id;
        int i = 0;
        for (auto it = source.begin(); it != source.end(); ++it, ++i)
        {
            if (i < src_begin)
            {
                EXPECT_EQ(*it, 100 + i) << i << case_id;
                continue;
            }
            if (i < src_size - (src_end - src_begin))
            {
                EXPECT_EQ(*it, 100 + i + (src_end - src_begin)) << i << case_id;
                continue;
            }
            EXPECT_EQ(-1, *it) << "should be unreachable " << i << case_id;
        }
    }

    void VerifyDest(const std::string& case_id)
    {
        EXPECT_EQ((int)dest.ExpensiveSize(), dest_size + (src_end - src_begin))
            << case_id;
        int i = 0;
        for (auto it = dest.begin(); it != dest.end(); ++it, ++i)
        {
            if (i < dest_pos)
            {
                EXPECT_EQ(*it, i) << i << case_id;
                continue;
            }
            if (i < dest_pos + (src_end - src_begin))
            {
                int src_idx = (i - dest_pos) + src_begin;
                EXPECT_EQ(*it, 100 + src_idx) << i << case_id;
                continue;
            }
            if (i < dest_size + (src_end - src_begin))
            {
                EXPECT_EQ(*it, i - (src_end - src_begin)) << i << case_id;
                continue;
            }
            EXPECT_EQ(-1, *it) << "should be unreachable " << i << case_id;
        }
    }

    void Verify(const std::string& case_id)
    {
        VerifySrc(case_id);
        VerifyDest(case_id);
    }

    void Test()
    {
        // stringifying parameters so that failures have a chance at being
        // useful.
        std::string case_id =
            " case id: " + std::to_string(src_size) + ", " +
            std::to_string(dest_size) + ", " + std::to_string(src_begin) +
            ", " + std::to_string(src_end) + ", " + std::to_string(dest_pos);

        // Populate Src with {100, 101, ...} and Dest with {0, 1, ...}.
        // Then Splice a subrange of Src into a position in Dest.
        BuildSrc();
        BuildDest();

        dest.SpliceSome(dest_pos_iter, source, src_begin_iter, src_end_iter);

        Verify(case_id);
    }

    void TestRValue()
    {
        // stringifying parameters so that failures have a chance at being
        // useful.
        std::string case_id =
            " case id: rvalue " + std::to_string(src_size) + ", " +
            std::to_string(dest_size) + ", " + std::to_string(src_begin) +
            ", " + std::to_string(src_end) + ", " + std::to_string(dest_pos);

        // Populate Src with {100, 101, ...} and Dest with {0, 1, ...}.
        // Then Splice a subrange of Src into a position in Dest.
        BuildSrc();
        BuildDest();

        dest.SpliceSome(dest_pos_iter,
                        std::move(source),
                        src_begin_iter,
                        src_end_iter);

        Verify(case_id);
    }
};

TEST(ListTest, SpliceSomeExhaustive)
{
    const int kMaxListSize = 3;
    // dimensions: Source size, dest size, dest pos, source begin, source end
    // 3 * 3 * 4 * 4 * 4 = 576 iterations (less than that actually)
    // not testing empty containers, but we are testing empty ranges
    for (int src_size = 1; src_size <= kMaxListSize; ++src_size)
    {
        for (int dest_size = 1; dest_size <= kMaxListSize; ++dest_size)
        {
            for (int dest_pos = 0; dest_pos <= dest_size; ++dest_pos)
            {
                for (int src_begin = 0; src_begin <= src_size; ++src_begin)
                {
                    for (int src_end = src_begin; src_end <= src_size;
                         ++src_end)
                    {
                        SpliceSomeExhaustive_data data{ src_size,
                                                        dest_size,
                                                        src_begin,
                                                        src_end,
                                                        dest_pos };
                        data.Test();
                        SpliceSomeExhaustive_data data2{ src_size,
                                                         dest_size,
                                                         src_begin,
                                                         src_end,
                                                         dest_pos };
                        data2.TestRValue();
                    }
                }
            }
        }
    }
}

struct SpliceOneExhaustive_data
{
    SpliceOneExhaustive_data(int src_size,
                             int dest_size,
                             int src_pos,
                             int dest_pos)
        : src_size(src_size),
          dest_size(dest_size),
          src_pos(src_pos),
          dest_pos(dest_pos)
    {
    }

    int src_size;
    int dest_size;
    int src_pos;
    int dest_pos;
    rad::List<int> source;
    rad::List<int> dest;
    rad::List<int>::IteratorType src_pos_iter;
    rad::List<int>::IteratorType dest_pos_iter;

    void BuildSrc()
    {
        src_pos_iter = source.end();
        for (int i = 0; i < src_size; ++i)
        {
            // populate with i offset by 100, so that we can tell src and dest
            // apart
            ASSERT_TRUE(source.PushBack(i + 100).IsOk());
            if (i == src_pos)
            {
                src_pos_iter = --source.end();
            }
        }
        ASSERT_NE(src_pos_iter, source.end());
    }

    void BuildDest()
    {
        dest_pos_iter = dest.end(); // base case
        for (int i = 0; i < dest_size; ++i)
        {
            ASSERT_TRUE(dest.PushBack(i).IsOk());
            if (i == dest_pos)
            {
                dest_pos_iter = --dest.end();
            }
        }
    }

    void VerifySrc(const std::string& case_id)
    {
        EXPECT_EQ((int)source.ExpensiveSize(), src_size - 1) << case_id;
        int i = 0;
        for (auto it = source.begin(); it != source.end(); ++it, ++i)
        {
            if (i < src_pos)
            {
                EXPECT_EQ(*it, 100 + i) << i << case_id;
                continue;
            }
            if (i < src_size - 1)
            {
                EXPECT_EQ(*it, 100 + i + 1) << i << case_id;
                continue;
            }
            EXPECT_EQ(-1, *it) << "should be unreachable " << i << case_id;
        }
    }

    void VerifyDest(const std::string& case_id)
    {
        EXPECT_EQ((int)dest.ExpensiveSize(), dest_size + 1) << case_id;
        int i = 0;
        for (auto it = dest.begin(); it != dest.end(); ++it, ++i)
        {
            if (i < dest_pos)
            {
                EXPECT_EQ(*it, i) << i << case_id;
                continue;
            }
            if (i == dest_pos)
            {
                EXPECT_EQ(*it, 100 + src_pos) << i << case_id;
                continue;
            }
            if (i < dest_size + 1)
            {
                EXPECT_EQ(*it, i - 1) << i << case_id;
                continue;
            }
            EXPECT_EQ(-1, *it) << "should be unreachable " << i << case_id;
        }
    }

    void Verify(const std::string& case_id)
    {
        VerifySrc(case_id);
        VerifyDest(case_id);
    }

    void Test()
    {
        // stringifying parameters so that failures have a chance at being
        // useful.
        std::string case_id = " case id: " + std::to_string(src_size) + ", " +
                              std::to_string(dest_size) + ", " +
                              std::to_string(src_pos) + ", " +
                              std::to_string(dest_pos);

        // Populate Src with {100, 101, ...} and Dest with {0, 1, ...}.
        // Then Splice an element of Src into a position in Dest.
        BuildSrc();
        BuildDest();

        dest.SpliceOne(dest_pos_iter, source, src_pos_iter);

        Verify(case_id);
    }

    void TestRValue()
    {
        // stringifying parameters so that failures have a chance at being
        // useful.
        std::string case_id = " case id: rvalue " + std::to_string(src_size) +
                              ", " + std::to_string(dest_size) + ", " +
                              std::to_string(src_pos) + ", " +
                              std::to_string(dest_pos);

        // Populate Src with {100, 101, ...} and Dest with {0, 1, ...}.
        // Then Splice an element of Src into a position in Dest.
        BuildSrc();
        BuildDest();

        dest.SpliceOne(dest_pos_iter, std::move(source), src_pos_iter);

        Verify(case_id);
    }
};

TEST(ListTest, SpliceOneExhaustive)
{
    const int kMaxListSize = 3;
    // dimensions: Source size, dest size, dest pos, source pos
    // 3 * 4 * 4 * 3 = 144 iterations (less than that actually)
    // not testing empty source containers
    for (int src_size = 1; src_size <= kMaxListSize; ++src_size)
    {
        for (int dest_size = 0; dest_size <= kMaxListSize; ++dest_size)
        {
            for (int dest_pos = 0; dest_pos <= dest_size; ++dest_pos)
            {
                for (int src_pos = 0; src_pos < src_size; ++src_pos)
                {
                    SpliceOneExhaustive_data data{ src_size,
                                                   dest_size,
                                                   src_pos,
                                                   dest_pos };
                    data.Test();
                    SpliceOneExhaustive_data data2{ src_size,
                                                    dest_size,
                                                    src_pos,
                                                    dest_pos };
                    data2.TestRValue();
                }
            }
        }
    }
}

struct SpliceAllExhaustive_data
{
    SpliceAllExhaustive_data(int src_size, int dest_size, int dest_pos)
        : src_size(src_size),
          dest_size(dest_size),
          dest_pos(dest_pos)
    {
    }

    int src_size;
    int dest_size;
    int dest_pos;
    rad::List<int> source;
    rad::List<int> dest;
    rad::List<int>::IteratorType dest_pos_iter;

    void BuildSrc()
    {
        for (int i = 0; i < src_size; ++i)
        {
            // populate with i offset by 100, so that we can tell src and dest
            // apart
            ASSERT_TRUE(source.PushBack(i + 100).IsOk());
        }
    }

    void BuildDest()
    {
        dest_pos_iter = dest.end(); // base case
        for (int i = 0; i < dest_size; ++i)
        {
            ASSERT_TRUE(dest.PushBack(i).IsOk());
            if (i == dest_pos)
            {
                dest_pos_iter = --dest.end();
            }
        }
    }

    void VerifySrc(const std::string& case_id)
    {
        EXPECT_TRUE(source.Empty()) << case_id;
    }

    void VerifyDest(const std::string& case_id)
    {
        EXPECT_EQ((int)dest.ExpensiveSize(), dest_size + src_size) << case_id;
        int i = 0;
        for (auto it = dest.begin(); it != dest.end(); ++it, ++i)
        {
            if (i < dest_pos)
            {
                EXPECT_EQ(*it, i) << i << case_id;
                continue;
            }
            if (i < dest_pos + src_size)
            {
                int src_idx = i - dest_pos;
                EXPECT_EQ(*it, 100 + src_idx) << i << case_id;
                continue;
            }
            if (i < dest_size + src_size)
            {
                EXPECT_EQ(*it, i - src_size) << i << case_id;
                continue;
            }
            EXPECT_EQ(-1, *it) << "should be unreachable " << i << case_id;
        }
    }

    void Verify(const std::string& case_id)
    {
        VerifySrc(case_id);
        VerifyDest(case_id);
    }

    void Test()
    {
        // stringifying parameters so that failures have a chance at being
        // useful.
        std::string case_id = " case id: " + std::to_string(src_size) + ", " +
                              std::to_string(dest_size) + ", " +
                              std::to_string(dest_pos);

        // Populate Src with {100, 101, ...} and Dest with {0, 1, ...}.
        // Then Splice Src into a position in Dest.
        BuildSrc();
        BuildDest();

        dest.SpliceAll(dest_pos_iter, source);

        Verify(case_id);
    }

    void TestRValue()
    {
        // stringifying parameters so that failures have a chance at being
        // useful.
        std::string case_id = " case id: rvalue " + std::to_string(src_size) +
                              ", " + std::to_string(dest_size) + ", " +
                              std::to_string(dest_pos);

        // Populate Src with {100, 101, ...} and Dest with {0, 1, ...}.
        // Then Splice Src into a position in Dest.
        BuildSrc();
        BuildDest();

        dest.SpliceAll(dest_pos_iter, std::move(source));

        Verify(case_id);
    }
};

TEST(ListTest, SpliceAllExhaustive)
{
    const int kMaxListSize = 3;
    // dimensions: Source size, dest size, dest pos
    // 3 * 4 * 4 = 48 iterations (less than that actually)
    // not testing empty source containers
    for (int src_size = 1; src_size <= kMaxListSize; ++src_size)
    {
        for (int dest_size = 0; dest_size <= kMaxListSize; ++dest_size)
        {
            for (int dest_pos = 0; dest_pos <= dest_size; ++dest_pos)
            {
                SpliceAllExhaustive_data data{ src_size, dest_size, dest_pos };
                data.Test();
                SpliceAllExhaustive_data data2{ src_size, dest_size, dest_pos };
                data2.TestRValue();
            }
        }
    }
}

TEST(ListTest, PrependRange)
{
    rad::List<int> dest;
    EXPECT_TRUE(
        dest.AssignRange(std::initializer_list<int>{ 0, 1, 2, 3 }).IsOk());
    std::array<int, 2> source = { 100, 101 };

    EXPECT_TRUE(dest.PrependRange(source).IsOk());
    ListEqual(dest, { 100, 101, 0, 1, 2, 3 });

    rad::List<int> empty;
    EXPECT_TRUE(dest.PrependRange(empty).IsOk());
    ListEqual(dest, { 100, 101, 0, 1, 2, 3 });

    rad::List<char> chars;
    EXPECT_TRUE(
        chars.AssignRange(std::initializer_list<char>{ 'x', 'y', 'z' }).IsOk());
    EXPECT_TRUE(
        chars.PrependRange(radtest::TestInputStringLiteralRange("abc")).IsOk());
    ListEqual(chars, { 'a', 'b', 'c', 'x', 'y', 'z' });

    radtest::HeapResource heap;
    radtest::ResourceAllocator<radtest::HeapResource> alloc(heap);
    rad::List<int, radtest::ResourceAllocator<radtest::HeapResource>> list(
        alloc);

    EXPECT_TRUE(list.AssignRange(std::initializer_list<int>{ 1, 2, 3 }).IsOk());
    heap.allocCount = heap.freeCount = 0;
    // PrependRange doesn't modify the list when allocations fail
    heap.forceFutureAllocFail = 2;
    EXPECT_TRUE(list.PrependRange(source).IsErr());
    EXPECT_EQ(heap.allocCount, 1);
    EXPECT_EQ(heap.freeCount, 1);
    ListEqual(list, { 1, 2, 3 });
}

TEST(ListTest, AppendRange)
{
    rad::List<int> dest;
    EXPECT_TRUE(
        dest.AssignRange(std::initializer_list<int>{ 0, 1, 2, 3 }).IsOk());
    std::array<int, 2> source = { 100, 101 };

    EXPECT_TRUE(dest.AppendRange(source).IsOk());
    ListEqual(dest, { 0, 1, 2, 3, 100, 101 });

    rad::List<int> empty;
    EXPECT_TRUE(dest.AppendRange(empty).IsOk());
    ListEqual(dest, { 0, 1, 2, 3, 100, 101 });

    rad::List<char> chars;
    EXPECT_TRUE(
        chars.AssignRange(std::initializer_list<char>{ 'x', 'y', 'z' }).IsOk());
    EXPECT_TRUE(
        chars.AppendRange(radtest::TestInputStringLiteralRange("abc")).IsOk());
    ListEqual(chars, { 'x', 'y', 'z', 'a', 'b', 'c' });

    radtest::HeapResource heap;
    radtest::ResourceAllocator<radtest::HeapResource> alloc(heap);
    rad::List<int, radtest::ResourceAllocator<radtest::HeapResource>> list(
        alloc);

    EXPECT_TRUE(list.AssignRange(std::initializer_list<int>{ 1, 2, 3 }).IsOk());
    heap.allocCount = heap.freeCount = 0;
    // AppendRange doesn't modify the list when allocations fail
    heap.forceFutureAllocFail = 2;
    EXPECT_TRUE(list.AppendRange(source).IsErr());
    EXPECT_EQ(heap.allocCount, 1);
    EXPECT_EQ(heap.freeCount, 1);
    ListEqual(list, { 1, 2, 3 });
}

TEST(ListTest, InsertRange)
{
    std::array<int, 2> source = { 100, 101 };

    {
        rad::List<int> dest;
        EXPECT_TRUE(
            dest.AssignRange(std::initializer_list<int>{ 0, 1, 2, 3 }).IsOk());

        rad::Res<rad::List<int>::IteratorType> it;
        rad::List<int>::IteratorType insert_pos;
        insert_pos = ++dest.begin();
        it = dest.InsertRange(insert_pos, source);
        EXPECT_EQ(it, ++dest.begin());
        EXPECT_NE(it, insert_pos);
        ListEqual(dest, { 0, 100, 101, 1, 2, 3 });

        rad::List<int> empty;
        insert_pos = ++dest.begin();
        it = dest.InsertRange(insert_pos, empty);
        EXPECT_EQ(it, ++dest.begin());
        EXPECT_EQ(it, insert_pos);
        ListEqual(dest, { 0, 100, 101, 1, 2, 3 });

        rad::List<char> chars;
        EXPECT_TRUE(
            chars.AssignRange(std::initializer_list<char>{ 'x', 'y', 'z' })
                .IsOk());
        auto char_insert_pos = ++chars.begin();
        auto res_char_it =
            chars.InsertRange(char_insert_pos,
                              radtest::TestInputStringLiteralRange("abc"));
        EXPECT_EQ(*res_char_it.Ok(), 'a');
        EXPECT_EQ(*char_insert_pos, 'y');
        EXPECT_EQ(*(++chars.begin()), 'a');
        ListEqual(chars, { 'x', 'a', 'b', 'c', 'y', 'z' });
    }
    {
        radtest::HeapResource heap;
        radtest::ResourceAllocator<radtest::HeapResource> alloc(heap);
        rad::List<int, radtest::ResourceAllocator<radtest::HeapResource>> list(
            alloc);

        EXPECT_TRUE(
            list.AssignRange(std::initializer_list<int>{ 1, 2, 3 }).IsOk());
        heap.allocCount = heap.freeCount = 0;
        auto insert_pos = ++list.begin();
        // InsertRange doesn't modify the list when allocations fail
        heap.forceFutureAllocFail = 2;
        auto it = list.InsertRange(insert_pos, source);
        EXPECT_EQ(heap.allocCount, 1);
        EXPECT_EQ(heap.freeCount, 1);
        EXPECT_TRUE(it.IsErr());
        EXPECT_NE(insert_pos, it);
        ListEqual(list, { 1, 2, 3 });
    }
}

TEST(ListTest, InsertSome)
{
    std::array<int, 2> source = { 100, 101 };

    {
        rad::List<int> dest;
        EXPECT_TRUE(
            dest.AssignRange(std::initializer_list<int>{ 0, 1, 2, 3 }).IsOk());

        rad::Res<rad::List<int>::IteratorType> it;
        rad::List<int>::IteratorType insert_pos;
        insert_pos = ++dest.begin();
        it = dest.InsertSome(insert_pos, source.begin(), source.end());
        EXPECT_EQ(it, ++dest.begin());
        EXPECT_NE(it, insert_pos);
        ListEqual(dest, { 0, 100, 101, 1, 2, 3 });

        int* empty = nullptr;
        insert_pos = ++dest.begin();
        it = dest.InsertSome(insert_pos, empty, empty);
        EXPECT_EQ(it, ++dest.begin());
        EXPECT_EQ(it, insert_pos);
        ListEqual(dest, { 0, 100, 101, 1, 2, 3 });
    }
    {
        radtest::HeapResource heap;
        radtest::ResourceAllocator<radtest::HeapResource> alloc(heap);
        rad::List<int, radtest::ResourceAllocator<radtest::HeapResource>> list(
            alloc);

        EXPECT_TRUE(
            list.AssignRange(std::initializer_list<int>{ 1, 2, 3 }).IsOk());
        heap.allocCount = heap.freeCount = 0;
        auto insert_pos = ++list.begin();
        // InsertSome doesn't modify the list when allocations fail
        heap.forceFutureAllocFail = 2;
        auto it = list.InsertSome(insert_pos, source.begin(), source.end());
        EXPECT_EQ(heap.allocCount, 1);
        EXPECT_EQ(heap.freeCount, 1);
        EXPECT_TRUE(it.IsErr());
        EXPECT_NE(insert_pos, it);
        ListEqual(list, { 1, 2, 3 });
    }
}

#if RAD_ENABLE_STD
TEST(ListTest, InsertInitializerList)
{
    {
        rad::List<int> dest;
        EXPECT_TRUE(
            dest.AssignRange(std::initializer_list<int>{ 0, 1, 2, 3 }).IsOk());

        rad::Res<rad::List<int>::IteratorType> it;
        rad::List<int>::IteratorType insert_pos;
        insert_pos = ++dest.begin();
        it = dest.InsertInitializerList(insert_pos, { 100, 101 });
        EXPECT_EQ(it, ++dest.begin());
        EXPECT_NE(it, insert_pos);
        ListEqual(dest, { 0, 100, 101, 1, 2, 3 });

        insert_pos = ++dest.begin();
        it = dest.InsertInitializerList(insert_pos, {});
        EXPECT_EQ(it, ++dest.begin());
        EXPECT_EQ(it, insert_pos);
        ListEqual(dest, { 0, 100, 101, 1, 2, 3 });
    }
    {
        radtest::HeapResource heap;
        radtest::ResourceAllocator<radtest::HeapResource> alloc(heap);
        rad::List<int, radtest::ResourceAllocator<radtest::HeapResource>> list(
            alloc);

        EXPECT_TRUE(
            list.AssignRange(std::initializer_list<int>{ 1, 2, 3 }).IsOk());
        heap.allocCount = heap.freeCount = 0;
        auto insert_pos = ++list.begin();
        // InsertSome doesn't modify the list when allocations fail
        heap.forceFutureAllocFail = 2;
        auto it = list.InsertInitializerList(insert_pos, { 100, 101 });
        EXPECT_EQ(heap.allocCount, 1);
        EXPECT_EQ(heap.freeCount, 1);
        EXPECT_TRUE(it.IsErr());
        EXPECT_NE(insert_pos, it);
        ListEqual(list, { 1, 2, 3 });
    }
}
#endif

TEST(ListTest, InsertCount)
{
    {
        rad::List<int> dest;
        EXPECT_TRUE(
            dest.AssignRange(std::initializer_list<int>{ 0, 1, 2, 3 }).IsOk());

        rad::Res<rad::List<int>::IteratorType> it;
        rad::List<int>::IteratorType insert_pos;
        insert_pos = ++dest.begin();
        it = dest.InsertCount(insert_pos, 2, 100);
        EXPECT_EQ(it, ++dest.begin());
        EXPECT_NE(it, insert_pos);
        ListEqual(dest, { 0, 100, 100, 1, 2, 3 });

        insert_pos = ++dest.begin();
        it = dest.InsertCount(insert_pos, 0, -3);
        EXPECT_EQ(it, ++dest.begin());
        EXPECT_EQ(it, insert_pos);
        ListEqual(dest, { 0, 100, 100, 1, 2, 3 });
    }
    {
        radtest::HeapResource heap;
        radtest::ResourceAllocator<radtest::HeapResource> alloc(heap);
        rad::List<int, radtest::ResourceAllocator<radtest::HeapResource>> list(
            alloc);

        EXPECT_TRUE(
            list.AssignRange(std::initializer_list<int>{ 1, 2, 3 }).IsOk());
        heap.allocCount = heap.freeCount = 0;
        auto insert_pos = ++list.begin();
        // InsertSome doesn't modify the list when allocations fail
        heap.forceFutureAllocFail = 2;
        auto it = list.InsertCount(insert_pos, 2, 100);
        EXPECT_EQ(heap.allocCount, 1);
        EXPECT_EQ(heap.freeCount, 1);
        EXPECT_TRUE(it.IsErr());
        EXPECT_NE(insert_pos, it);
        ListEqual(list, { 1, 2, 3 });
    }
}

TEST(ListTest, Clone)
{
    {
        rad::List<int> li;

        rad::Res<rad::List<int>> empty_clone = li.Clone();
        ASSERT_TRUE(empty_clone.IsOk());
        ASSERT_TRUE(empty_clone.Ok().Empty());

        EXPECT_TRUE(
            li.AssignRange(std::initializer_list<int>{ 1, 2, 3 }).IsOk());

        rad::Res<rad::List<int>> li2 = li.Clone();
        ASSERT_TRUE(li2.IsOk());
        ListEqual(li2.Ok(), { 1, 2, 3 });
    }
    {
        radtest::HeapResource heap;
        radtest::ResourceAllocator<radtest::HeapResource> alloc(heap);
        rad::List<int, radtest::ResourceAllocator<radtest::HeapResource>> list(
            alloc);

        EXPECT_TRUE(
            list.AssignRange(std::initializer_list<int>{ 1, 2, 3 }).IsOk());
        heap.allocCount = heap.freeCount = 0;

        auto success_clone_res = list.Clone();
        ASSERT_TRUE(success_clone_res.IsOk());
        auto& success_clone = success_clone_res.Ok();
        EXPECT_EQ(heap.allocCount, 3);
        ListEqual(success_clone, { 1, 2, 3 });
        heap.allocCount = heap.freeCount = 0;

        // Ensure the allocator was copied
        EXPECT_TRUE(success_clone.PushBack(4).IsOk());
        EXPECT_EQ(heap.allocCount, 1);
        success_clone.Clear();
        EXPECT_EQ(heap.freeCount, 4);

        heap.allocCount = heap.freeCount = 0;
        heap.forceFutureAllocFail = 2;
        auto fail_clone = list.Clone();
        EXPECT_EQ(heap.allocCount, 1);
        EXPECT_EQ(heap.freeCount, 1);
        EXPECT_TRUE(fail_clone.IsErr());
    }
}

struct EraseSomeExhaustive_data
{
    EraseSomeExhaustive_data(int size, int begin, int end)
        : size(size),
          begin(begin),
          end(end)
    {
    }

    int size;
    int begin;
    int end;
    rad::List<int> list;
    rad::List<int>::IteratorType begin_iter;
    rad::List<int>::IteratorType end_iter;

    void BuildList()
    {
        begin_iter = list.end(); // .end() is the base case
        end_iter = list.end();
        for (int i = 0; i < size; ++i)
        {
            ASSERT_TRUE(list.PushBack(i).IsOk());
            if (i == begin)
            {
                begin_iter = --list.end();
            }
            if (i == end)
            {
                end_iter = --list.end();
            }
        }
    }

    void Verify(const std::string& case_id)
    {
        EXPECT_EQ((int)list.ExpensiveSize(), size - (end - begin)) << case_id;
        int i = 0;
        for (auto it = list.begin(); it != list.end(); ++it, ++i)
        {
            if (i < begin)
            {
                EXPECT_EQ(*it, i) << i << case_id;
                continue;
            }
            if (i < size - (end - begin))
            {
                EXPECT_EQ(*it, i + (end - begin)) << i << case_id;
                continue;
            }
            EXPECT_EQ(-1, *it) << "should be unreachable " << i << case_id;
        }
    }

    void Test()
    {
        // stringifying parameters so that failures have a chance at being
        // useful.
        std::string case_id = " case id: " + std::to_string(size) + ", " +
                              std::to_string(begin) + ", " +
                              std::to_string(end);

        BuildList();

        auto ret_iter = list.EraseSome(begin_iter, end_iter);
        EXPECT_EQ(ret_iter, end_iter) << case_id;

        Verify(case_id);
    }
};

TEST(ListTest, EraseSomeExhaustive)
{
    const int kMaxListSize = 4;
    // dimensions: Size, begin, end
    // 5 * 5 * 5 = 125 iterations (less than that actually)
    for (int size = 0; size <= kMaxListSize; ++size)
    {
        for (int begin = 0; begin <= size; ++begin)
        {
            for (int end = begin; end <= size; ++end)
            {
                EraseSomeExhaustive_data data{ size, begin, end };
                data.Test();
            }
        }
    }
}

struct EraseOneExhaustive_data
{
    EraseOneExhaustive_data(int size, int pos)
        : size(size),
          pos(pos)
    {
    }

    int size;
    int pos;
    rad::List<int> list;
    rad::List<int>::IteratorType pos_iter;

    void BuildList()
    {
        pos_iter = list.end(); // .end() is the base case
        for (int i = 0; i < size; ++i)
        {
            ASSERT_TRUE(list.PushBack(i).IsOk());
            if (i == pos)
            {
                pos_iter = --list.end();
            }
        }
    }

    void Verify(const std::string& case_id)
    {
        EXPECT_EQ((int)list.ExpensiveSize(), size - 1) << case_id;
        int i = 0;
        for (auto it = list.begin(); it != list.end(); ++it, ++i)
        {
            if (i < pos)
            {
                EXPECT_EQ(*it, i) << i << case_id;
                continue;
            }
            if (i < size - 1)
            {
                EXPECT_EQ(*it, i + 1) << i << case_id;
                continue;
            }
            EXPECT_EQ(-1, *it) << "should be unreachable " << i << case_id;
        }
    }

    void Test()
    {
        // stringifying parameters so that failures have a chance at being
        // useful.
        std::string case_id =
            " case id: " + std::to_string(size) + ", " + std::to_string(pos);

        BuildList();

        rad::List<int>::IteratorType expected_iter = pos_iter;
        ++expected_iter;

        auto ret_iter = list.EraseOne(pos_iter);
        EXPECT_EQ(ret_iter, expected_iter);

        Verify(case_id);
    }
};

TEST(ListTest, EraseOneExhaustive)
{
    const int kMaxListSize = 4;
    // dimensions: Size, pos.  Not testing empty containers or end iterators
    // 4 * 4  = 16 iterations (less than that actually)
    for (int size = 1; size <= kMaxListSize; ++size)
    {
        for (int pos = 0; pos < size; ++pos)
        {
            EraseOneExhaustive_data data{ size, pos };
            data.Test();
        }
    }
}

TEST(ListTest, EraseOneEnd)
{
    rad::List<int> list;
    EXPECT_EQ(list.end(), list.EraseOne(list.end()));
    EXPECT_TRUE(list.Empty());

    EXPECT_TRUE(list.AssignRange(std::initializer_list<int>{ 1 }).IsOk());
    EXPECT_EQ(list.end(), list.EraseOne(list.end()));
    ListEqual(list, { 1 });

    EXPECT_TRUE(list.AssignRange(std::initializer_list<int>{ 1, 2 }).IsOk());
    EXPECT_EQ(list.end(), list.EraseOne(list.end()));
    ListEqual(list, { 1, 2 });

    EXPECT_TRUE(list.AssignRange(std::initializer_list<int>{ 1, 2, 3 }).IsOk());
    EXPECT_EQ(list.end(), list.EraseOne(list.end()));
    ListEqual(list, { 1, 2, 3 });
}

TEST(ListTest, EraseValue)
{
    rad::List<int> list;
    EXPECT_EQ(0u, list.EraseValue(42));

    EXPECT_TRUE(list.AssignRange(std::initializer_list<int>{ 1 }).IsOk());
    EXPECT_EQ(0u, list.EraseValue(42));
    ListEqual(list, { 1 });

    EXPECT_TRUE(list.AssignRange(std::initializer_list<int>{ 1, 42 }).IsOk());
    EXPECT_EQ(1u, list.EraseValue(42));
    ListEqual(list, { 1 });

    EXPECT_TRUE(list.AssignRange(std::initializer_list<int>{ 42, 1 }).IsOk());
    EXPECT_EQ(1u, list.EraseValue(42));
    ListEqual(list, { 1 });

    EXPECT_TRUE(list.AssignRange(std::initializer_list<int>{ 42, 42 }).IsOk());
    EXPECT_EQ(2u, list.EraseValue(42));
    EXPECT_TRUE(list.Empty());

    EXPECT_TRUE(
        list.AssignRange(std::initializer_list<int>{ 1, 42, 1 }).IsOk());
    EXPECT_EQ(1u, list.EraseValue(42));
    ListEqual(list, { 1, 1 });

    EXPECT_TRUE(list.AssignCount(50, 42).IsOk());
    EXPECT_EQ(50u, list.EraseValue(42));
    EXPECT_TRUE(list.Empty());
}

TEST(ListTest, EraseIf)
{
    auto is_even = [](int val) { return val % 2 == 0; };
    rad::List<int> list;
    EXPECT_EQ(0u, list.EraseIf(is_even));

    EXPECT_TRUE(list.AssignRange(std::initializer_list<int>{ 1 }).IsOk());
    EXPECT_EQ(0u, list.EraseIf(is_even));
    ListEqual(list, { 1 });

    EXPECT_TRUE(list.AssignRange(std::initializer_list<int>{ 1, 42 }).IsOk());
    EXPECT_EQ(1u, list.EraseIf(is_even));
    ListEqual(list, { 1 });

    EXPECT_TRUE(list.AssignRange(std::initializer_list<int>{ 42, 1 }).IsOk());
    EXPECT_EQ(1u, list.EraseIf(is_even));
    ListEqual(list, { 1 });

    EXPECT_TRUE(list.AssignRange(std::initializer_list<int>{ 42, 42 }).IsOk());
    EXPECT_EQ(2u, list.EraseIf(is_even));
    EXPECT_TRUE(list.Empty());

    EXPECT_TRUE(
        list.AssignRange(std::initializer_list<int>{ 1, 42, 1 }).IsOk());
    EXPECT_EQ(1u, list.EraseIf(is_even));
    ListEqual(list, { 1, 1 });

    EXPECT_TRUE(list.AssignCount(50, 42).IsOk());
    EXPECT_EQ(50u, list.EraseIf(is_even));
    EXPECT_TRUE(list.Empty());
}

TEST(ListTest, PopFront)
{
    rad::List<int> list;
    EXPECT_TRUE(
        list.AssignRange(std::initializer_list<int>{ 1, 2, 3, 4, 5 }).IsOk());

    list.PopFront();
    ListEqual(list, { 2, 3, 4, 5 });
    list.PopFront();
    ListEqual(list, { 3, 4, 5 });
    list.PopFront();
    ListEqual(list, { 4, 5 });
    list.PopFront();
    ListEqual(list, { 5 });
    list.PopFront();
    EXPECT_TRUE(list.Empty());

#if !RAD_DBG
    list.PopFront();
    EXPECT_TRUE(list.Empty());
#endif
}

TEST(ListTest, PopBack)
{
    rad::List<int> list;
    EXPECT_TRUE(
        list.AssignRange(std::initializer_list<int>{ 1, 2, 3, 4, 5 }).IsOk());

    list.PopBack();
    ListEqual(list, { 1, 2, 3, 4 });
    list.PopBack();
    ListEqual(list, { 1, 2, 3 });
    list.PopBack();
    ListEqual(list, { 1, 2 });
    list.PopBack();
    ListEqual(list, { 1 });
    list.PopBack();
    EXPECT_TRUE(list.Empty());

#if !RAD_DBG
    list.PopBack();
    EXPECT_TRUE(list.Empty());
#endif
}

TEST(ListTest, TakeFront)
{
    {
        rad::List<int> list;
        EXPECT_TRUE(
            list.AssignRange(std::initializer_list<int>{ 1, 2, 3, 4, 5 })
                .IsOk());

        EXPECT_EQ(list.TakeFront(), 1);
        ListEqual(list, { 2, 3, 4, 5 });
        EXPECT_EQ(list.TakeFront(), 2);
        ListEqual(list, { 3, 4, 5 });
        EXPECT_EQ(list.TakeFront(), 3);
        ListEqual(list, { 4, 5 });
        EXPECT_EQ(list.TakeFront(), 4);
        ListEqual(list, { 5 });
        EXPECT_EQ(list.TakeFront(), 5);
        EXPECT_TRUE(list.Empty());

        EXPECT_TRUE(list.TakeFront().IsErr());
        EXPECT_TRUE(list.Empty());
    }
    {
        rad::List<MoveStruct> list;
        EXPECT_TRUE(list.EmplaceFront().IsOk());

        EXPECT_EQ(list.TakeFront().Ok().val, 42);

        EXPECT_TRUE(list.TakeFront().IsErr());
        EXPECT_TRUE(list.Empty());
    }
#if 0
    {
        // This intentionally does not build, because ImmovableStruct is immovable
        rad::List<ImmovableStruct> list;
        list.TakeFront();
    }
#endif
#if 0
    {
        // This intentionally does not build, because CopyStruct has a throwing move
        rad::List<CopyStruct> list;
        list.TakeFront();
    }
#endif
    {
        rad::List<rad::Error> list;
        EXPECT_TRUE(list.PushBack(rad::Error::InvalidAddress).IsOk());
        auto res = list.TakeFront();
        EXPECT_TRUE(res.IsOk());
        EXPECT_EQ(res.Ok(), rad::Error::InvalidAddress);

        auto res2 = list.TakeFront();
        EXPECT_TRUE(res2.IsErr());
        EXPECT_EQ(res2.Err(), rad::Error::OutOfRange);
    }
}

TEST(ListTest, TakeBack)
{
    {
        rad::List<int> list;
        EXPECT_TRUE(
            list.AssignRange(std::initializer_list<int>{ 1, 2, 3, 4, 5 })
                .IsOk());

        EXPECT_EQ(list.TakeBack(), 5);
        ListEqual(list, { 1, 2, 3, 4 });
        EXPECT_EQ(list.TakeBack(), 4);
        ListEqual(list, { 1, 2, 3 });
        EXPECT_EQ(list.TakeBack(), 3);
        ListEqual(list, { 1, 2 });
        EXPECT_EQ(list.TakeBack(), 2);
        ListEqual(list, { 1 });
        EXPECT_EQ(list.TakeBack(), 1);
        EXPECT_TRUE(list.Empty());

        EXPECT_TRUE(list.TakeBack().IsErr());
        EXPECT_TRUE(list.Empty());
    }
    {
        rad::List<MoveStruct> list;
        EXPECT_TRUE(list.EmplaceFront().IsOk());

        EXPECT_EQ(list.TakeBack().Ok().val, 42);

        EXPECT_TRUE(list.TakeBack().IsErr());
        EXPECT_TRUE(list.Empty());
    }
#if 0
    {
        // This intentionally does not build, because ImmovableStruct is immovable
        rad::List<ImmovableStruct> list;
        list.TakeBack();
    }
#endif
#if 0
    {
        // This intentionally does not build, because CopyStruct has a throwing move
        rad::List<CopyStruct> list;
        list.TakeBack();
    }
#endif
    {
        rad::List<rad::Error> list;
        EXPECT_TRUE(list.PushBack(rad::Error::InvalidAddress).IsOk());
        auto res = list.TakeBack();
        EXPECT_TRUE(res.IsOk());
        EXPECT_EQ(res.Ok(), rad::Error::InvalidAddress);

        auto res2 = list.TakeBack();
        EXPECT_TRUE(res2.IsErr());
        EXPECT_EQ(res2.Err(), rad::Error::OutOfRange);
    }
}

TEST(ListTest, Reverse)
{
    rad::List<int> list;
    list.Reverse();
    EXPECT_TRUE(list.Empty());

    EXPECT_TRUE(list.AssignRange(std::initializer_list<int>{ 1 }).IsOk());
    list.Reverse();
    ListEqual(list, { 1 });

    EXPECT_TRUE(list.AssignRange(std::initializer_list<int>{ 1, 2 }).IsOk());
    list.Reverse();
    ListEqual(list, { 2, 1 });

    EXPECT_TRUE(list.AssignRange(std::initializer_list<int>{ 1, 2, 3 }).IsOk());
    list.Reverse();
    ListEqual(list, { 3, 2, 1 });

    EXPECT_TRUE(
        list.AssignRange(std::initializer_list<int>{ 1, 2, 3, 4 }).IsOk());
    list.Reverse();
    ListEqual(list, { 4, 3, 2, 1 });
}

TEST(ListTest, FrontBack)
{
    {
        rad::List<int> list;
        const rad::List<int>& clist = list;

        EXPECT_TRUE(list.Front().IsErr());
        EXPECT_TRUE(list.Back().IsErr());
        EXPECT_TRUE(clist.Front().IsErr());
        EXPECT_TRUE(clist.Back().IsErr());
    }
    {
        rad::List<int> list;
        EXPECT_TRUE(
            list.AssignRange(std::initializer_list<int>{ 1, 2, 3, 4, 5 })
                .IsOk());
        EXPECT_EQ(list.Front(), 1);
        EXPECT_EQ(list.Back(), 5);

        const rad::List<int>& clist = list;
        EXPECT_EQ(clist.Front(), 1);
        EXPECT_EQ(clist.Back(), 5);

        // ensure Res::op= rebinds, and doesn't assign through
        int val = 42;
        auto res = list.Front();
        res = val;
        EXPECT_EQ(res.Ok(), 42);
        EXPECT_EQ(list.Front(), 1);

        res = list.Front();
        *res = val; // ensure *res::op= assigns through
        EXPECT_EQ(list.Front(), 42);

        res = list.Back();
        *res = 99;
        EXPECT_EQ(list.Back(), 99);
    }
}

TEST(ListTest, Swap)
{
    {
        rad::List<int> list1;
        EXPECT_TRUE(
            list1.AssignRange(std::initializer_list<int>{ 1, 2, 3, 4, 5 })
                .IsOk());

        rad::List<int> list2;
        EXPECT_TRUE(
            list2.AssignRange(std::initializer_list<int>{ 11, 12, 13, 14, 15 })
                .IsOk());

        list1.Swap(list2);
        ListEqual(list1, { 11, 12, 13, 14, 15 });
        ListEqual(list2, { 1, 2, 3, 4, 5 });
    }
    {
        rad::List<int> list1;
        EXPECT_TRUE(
            list1.AssignRange(std::initializer_list<int>{ 1, 2, 3, 4, 5 })
                .IsOk());

        rad::List<int> list2;

        list1.Swap(list2);
        ListEqual(list1, {});
        ListEqual(list2, { 1, 2, 3, 4, 5 });
    }
    {
        rad::List<int> list1;
        EXPECT_TRUE(
            list1.AssignRange(std::initializer_list<int>{ 1, 2, 3, 4, 5 })
                .IsOk());

        list1.Swap(list1);
        ListEqual(list1, { 1, 2, 3, 4, 5 });
    }
    {
        rad::List<int> list1;
        list1.Swap(list1);
        ListEqual(list1, {});
    }
}

TEST(ListTest, ReverseIterators)
{
    {
        rad::List<int> list1;
        EXPECT_EQ(list1.rbegin(), list1.rend());

        rad::List<int> list2;
        EXPECT_TRUE(list2.AssignSome(list1.rbegin(), list1.rend()).IsOk());
        EXPECT_TRUE(list2.Empty());
    }
    {
        rad::List<int> list1;
        EXPECT_TRUE(
            list1.AssignRange(std::initializer_list<int>{ 1, 2, 3, 4, 5 })
                .IsOk());

        rad::List<int> list2;
        EXPECT_TRUE(list2.AssignSome(list1.rbegin(), list1.rend()).IsOk());
        ListEqual(list2, { 5, 4, 3, 2, 1 });
    }

    {
        rad::List<int> list1;
        EXPECT_EQ(list1.crbegin(), list1.crend());

        rad::List<int> list2;
        EXPECT_TRUE(list2.AssignSome(list1.crbegin(), list1.crend()).IsOk());
        EXPECT_TRUE(list2.Empty());
    }
    {
        rad::List<int> list1;
        EXPECT_TRUE(
            list1.AssignRange(std::initializer_list<int>{ 1, 2, 3, 4, 5 })
                .IsOk());

        rad::List<int> list2;
        EXPECT_TRUE(list2.AssignSome(list1.crbegin(), list1.crend()).IsOk());
        ListEqual(list2, { 5, 4, 3, 2, 1 });
    }
}

#ifdef RAD_GCC_VERSION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmultichar"
#endif
TEST(ListTest, TroubleAllocators)
{
    {
        using StickyList = rad::List<int, radtest::StickyTaggedAllocator>;
        StickyList list1(radtest::StickyTaggedAllocator{ 'tst1' });
        EXPECT_TRUE(list1.PushBack(1).IsOk());

        StickyList list2(std::move(list1));
        EXPECT_TRUE(list1.Empty());
        EXPECT_EQ(list1.GetAllocator().m_tag, uint32_t('tst1'));
        EXPECT_EQ(list2.ExpensiveSize(), 1u);
        EXPECT_EQ(list2.GetAllocator().m_tag, uint32_t('tst1'));

        // These all static_assert, as they should
        // list1 = std::move(list2);
        // list1.Swap(list2);
        // list1.SpliceAll(list1.end(), list2);
        // list1.Clone();
    }
    {
        using StickyDefaultList =
            rad::List<int, radtest::StickyDefaultTaggedAllocator>;
        StickyDefaultList list1(
            radtest::StickyDefaultTaggedAllocator{ 'tst1' });
        EXPECT_TRUE(list1.PushBack(1).IsOk());

        StickyDefaultList list2(std::move(list1));
        EXPECT_TRUE(list1.Empty());
        EXPECT_EQ(list1.GetAllocator().m_tag, uint32_t('tst1'));
        EXPECT_EQ(list2.ExpensiveSize(), 1u);
        EXPECT_EQ(list2.GetAllocator().m_tag, uint32_t('tst1'));

        auto expected_list3 = list2.Clone();
        EXPECT_EQ(list2.ExpensiveSize(), 1u);
        EXPECT_EQ(expected_list3.Ok().ExpensiveSize(), 1u);
        EXPECT_EQ(expected_list3.Ok().GetAllocator().m_tag, uint32_t(1234));

        // These all static_assert, as they should
        // list1 = std::move(list2);
        // list1.Swap(list2);
        // list1.SpliceAll(list1.end(), list2);
    }
    {
        using MovingList = rad::List<int, radtest::MovingTaggedAllocator>;
        MovingList list1(radtest::MovingTaggedAllocator{ 'abcd' });
        EXPECT_TRUE(list1.PushBack(1).IsOk());

        MovingList list2(std::move(list1));
        EXPECT_TRUE(list1.Empty());
        EXPECT_EQ(list2.ExpensiveSize(), 1u);

        auto expected_list3 = list2.Clone();
        EXPECT_EQ(list2.ExpensiveSize(), 1u);
        EXPECT_EQ(expected_list3.Ok().ExpensiveSize(), 1u);

        MovingList list4(radtest::MovingTaggedAllocator{ 'wxyz' });
        EXPECT_TRUE(list1.PushBack(99).IsOk());
        list1.Swap(list4);
        EXPECT_TRUE(list1.Empty());
        EXPECT_EQ(list4.ExpensiveSize(), 1u);
        EXPECT_EQ(list1.GetAllocator().m_tag, uint32_t('wxyz'));
        EXPECT_EQ(list4.GetAllocator().m_tag, uint32_t('abcd'));

        list1 = std::move(list4);
        EXPECT_TRUE(list4.Empty());
        EXPECT_EQ(list1.ExpensiveSize(), 1u);
        EXPECT_EQ(list1.GetAllocator().m_tag, uint32_t('abcd'));
        EXPECT_EQ(list4.GetAllocator().m_tag, uint32_t('abcd'));

        // This static_asserts, as it should
        // list1.SpliceAll(list1.end(), list2);
    }
    {
        using TypedList = rad::List<int, radtest::TypedAllocator>;
        TypedList list1;
        EXPECT_TRUE(list1.PushBack(1).IsOk());

        TypedList list2(std::move(list1));
        EXPECT_TRUE(list1.Empty());
        EXPECT_EQ(list2.ExpensiveSize(), 1u);

        auto expected_list3 = list2.Clone();
        EXPECT_EQ(list2.ExpensiveSize(), 1u);
        EXPECT_EQ(expected_list3.Ok().ExpensiveSize(), 1u);

        list1 = std::move(list2);
        EXPECT_TRUE(list2.Empty());
        EXPECT_EQ(list1.ExpensiveSize(), 1u);

        list1.Swap(list2);
        EXPECT_TRUE(list1.Empty());
        EXPECT_EQ(list2.ExpensiveSize(), 1u);

        list1.SpliceAll(list1.end(), list2);
        EXPECT_TRUE(list2.Empty());
        EXPECT_EQ(list1.ExpensiveSize(), 1u);
    }
}
#ifdef RAD_GCC_VERSION
#pragma GCC diagnostic pop
#endif
