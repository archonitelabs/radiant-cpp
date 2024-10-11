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
#include <radiant/Span.h>

#include "test/TestAlloc.h"

#include <gtest/gtest.h>

#include <array>
#include <utility>

using namespace testing;

// ensure no_unique_address is doing what we want
static_assert(sizeof(rad::detail::ListUntyped) == 2 * sizeof(void*),
              "unexpected object size");
static_assert(sizeof(rad::List<int>) == 2 * sizeof(void*),
              "unexpected object size");
static_assert(sizeof(rad::detail::ListUntyped) == sizeof(rad::List<int>),
              "unexpected object size");

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

TEST(ListTest, PushBackFailureRecovery)
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

TEST(ListTest, CopyPushFront)
{
    rad::List<CopyStruct> i;
    CopyStruct local;
    local.val = 42;
    EXPECT_TRUE(i.PushFront(local).IsOk());
    ListValEqual(i, { 42 });

    local.val = 99;
    EXPECT_TRUE(i.PushFront(local).IsOk());
    ListValEqual(i, { 99, 42 });

    local.val = 77;
    EXPECT_TRUE(i.PushFront(local).IsOk());
    ListValEqual(i, { 77, 99, 42 });
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

TEST(ListTest, AssignInitList)
{
    rad::List<int> li;
    EXPECT_TRUE(li.AssignInitializerList({}).IsOk());
    EXPECT_TRUE(li.Empty());

    EXPECT_TRUE(li.AssignInitializerList({ 42 }).IsOk());
    EXPECT_EQ(1, li.ExpensiveSize());

    EXPECT_TRUE(li.AssignInitializerList({ 100, 101, 102, 103 }).IsOk());
    EXPECT_EQ(4, li.ExpensiveSize());

    int i = 100;
    for (int elt : li)
    {
        EXPECT_EQ(i, elt);
        ++i;
    }

    EXPECT_TRUE(li.AssignInitializerList({}).IsOk());
    EXPECT_TRUE(li.Empty());
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
        EXPECT_TRUE(two.AssignInitializerList({ 1, 2 }).IsOk());

        rad::List<int> move_from_two(std::move(two));
        ListEqual(two, {});
        ListEqual(move_from_two, { 1, 2 });
    }
    {
        rad::List<int> three;
        EXPECT_TRUE(three.AssignInitializerList({ 1, 2, 3 }).IsOk());

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

    EXPECT_TRUE(i.AssignInitializerList({ 2, 3 }).IsOk());
    EXPECT_EQ(i.ExpensiveSize(), 2);
    i.Clear();
    EXPECT_TRUE(i.Empty());

    EXPECT_TRUE(i.AssignInitializerList({ 4, 5, 6 }).IsOk());
    EXPECT_EQ(i.ExpensiveSize(), 3);
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

    EXPECT_TRUE(input.AssignInitializerList({ 0, 1, 2 }).IsOk());

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
    // TODO: forward and input iterators.  Proxy iterators?
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
    // TODO: forward and input iterators.  Proxy iterators?
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
    radtest::HeapAllocator heap;
    radtest::AllocWrapper<ImmovableStruct, radtest::HeapAllocator> alloc(heap);
    rad::List<ImmovableStruct,
              radtest::AllocWrapper<ImmovableStruct, radtest::HeapAllocator>>
        input(alloc);
    decltype(input.begin()) iter;

    // emplace at the end
    iter = input.Emplace(input.end(), 42);
    EXPECT_TRUE(iter == --input.end());
    EXPECT_EQ(iter->val, 42);
    iter = input.Emplace(input.end(), 43);
    EXPECT_TRUE(iter == --input.end());
    EXPECT_EQ(iter->val, 43);
    ListValEqual(input, { 42, 43 });

    // emplace at the beginning
    iter = input.Emplace(input.begin(), 99);
    EXPECT_TRUE(iter == input.begin());
    EXPECT_EQ(iter->val, 99);
    iter = input.Emplace(input.begin(), 100);
    EXPECT_TRUE(iter == input.begin());
    EXPECT_EQ(iter->val, 100);
    ListValEqual(input, { 100, 99, 42, 43 });

    // emplace near the beginning
    auto old_iter = ++input.begin();
    iter = input.Emplace(old_iter, 23);
    EXPECT_TRUE(iter == ++input.begin());
    EXPECT_TRUE(iter != old_iter);
    ListValEqual(input, { 100, 23, 99, 42, 43 });

    // emplace near the end
    old_iter = --input.end();
    iter = input.Emplace(old_iter, 77);
    EXPECT_TRUE(iter == --(--input.end()));
    EXPECT_TRUE(iter != old_iter);
    ListValEqual(input, { 100, 23, 99, 42, 77, 43 });

    heap.forceAllocFails = 1;
    iter = input.Emplace(input.begin(), -1);
    EXPECT_TRUE(iter == input.end());
    ListValEqual(input, { 100, 23, 99, 42, 77, 43 });
}

TEST(ListTest, AssignFailure)
{
    radtest::HeapAllocator heap;
    radtest::AllocWrapper<int, radtest::HeapAllocator> alloc(heap);
    rad::List<int, radtest::AllocWrapper<int, radtest::HeapAllocator>> list(
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
    EXPECT_TRUE(list.AssignInitializerList({ 1, 2, 3, 4, 5 }).IsErr());
    EXPECT_EQ(heap.allocCount, 2);
    EXPECT_EQ(heap.freeCount, 2);
    EXPECT_TRUE(list.Empty());

    // make sure nothing is corrupted
    EXPECT_TRUE(list.AssignInitializerList({ 1, 2, 3, 4, 5 }).IsOk());
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
        list.AssignInitializerList({ 101, 102, 103, 104, 105 }).IsErr());
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
    EXPECT_TRUE(data.AssignInitializerList({ 0, 1, 2, 3 }));
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
