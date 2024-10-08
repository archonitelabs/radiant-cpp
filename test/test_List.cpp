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
#include <radiant/List.h>

#include <gtest/gtest.h>

#include <utility>

using namespace testing;

// TODO: don't forget to test iterator operations and iterator stability!

class ListTestAlloc
{
public:

    static constexpr ULONG k_PoolTag = '2iLD';
    static bool g_ForceAllocateFailure;
    static int32_t g_AllocateCallCount;
    static int32_t g_FreeCallCount;

    static void ResetInstrumentation()
    {
        g_ForceAllocateFailure = false;
        g_AllocateCallCount = 0;
        g_FreeCallCount = 0;
    }

    // Per allocator state, so we can see if the allocator is copied / moved
    // right
    int32_t m_id = 0;
    ~ListTestAlloc() = default;
    ListTestAlloc() = default;
    ListTestAlloc(const ListTestAlloc&) = default;
    ListTestAlloc& operator=(const ListTestAlloc&) = default;

    ListTestAlloc(ListTestAlloc&& other) noexcept
        : m_id(other.m_id)
    {
        other.m_id = -other.m_id;
    }

    ListTestAlloc& operator=(ListTestAlloc&& rhs) noexcept
    {
        m_id = rhs.m_id;
        rhs.m_id = -rhs.m_id;
        return *this;
    }

    /*!
        @brief Allocates memory of a given size in bytes from the pool
         described by this allocator using the tag described by this allocator.

        @param[in] SizeInBytes - Size of memory to allocate in bytes.

        @return On success returns pointer to newly allocated memory. On
         failure nullptr is returned.
    */
    _IRQL_requires_same_ _IRQL_requires_max_(APC_LEVEL)
        _When_(return != nullptr,
                      CS_NO_SAL2(__drv_allocatesMem(
                          Mem))) static inline _Ret_maybenull_ _Check_return_
        void* Allocate(_In_ size_t SizeInBytes)
    {
        CS_PASSIVE_OR_APC_LEVEL();
        ++g_AllocateCallCount;
        if (g_ForceAllocateFailure)
        {
            return nullptr;
        }
        return AllocatePool(TRUE, SizeInBytes, k_PoolTag);
    }

    /*!
        @brief Frees memory back to the pool.

        @param[in] Memory - Memory to free.
    */
    _IRQL_requires_same_ _IRQL_requires_max_(APC_LEVEL) static inline void Free(
        _In_ _Frees_ptr_ void* Memory)
    {
        CS_PASSIVE_OR_APC_LEVEL();
        ++g_FreeCallCount;
        FreePool(Memory, k_PoolTag);
    }
};

bool ListTestAlloc::g_ForceAllocateFailure = false;
int32_t ListTestAlloc::g_AllocateCallCount = 0;
int32_t ListTestAlloc::g_FreeCallCount = 0;

template <typename T, typename Alloc>
void List_equal(const rad::List<T, Alloc>& dlist,
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

template <typename T, typename Alloc, typename U>
void List_val_equal(const rad::List<T, Alloc>& dlist,
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

TEST(List_test, default_construct_is_empty)
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

TEST(List_test, allocator_constructors)
{
    ListTestAlloc::ResetInstrumentation();
    {
        rad::List<int, ListTestAlloc> default_ctor;
        ListTestAlloc default_ctor_alloc = default_ctor.get_allocator();
        EXPECT_EQ(default_ctor_alloc.m_id, 0);
    }
    {
        // allocator constructor
        ListTestAlloc source_alloc;
        source_alloc.m_id = 42;
        rad::List<int, ListTestAlloc> alloc_ctor(source_alloc);
        ListTestAlloc alloc = alloc_ctor.get_allocator();
        EXPECT_EQ(alloc.m_id, 42);

        // regular move constructor needs to steal the original allocator
        rad::List<int, ListTestAlloc> moving_alloc_ctor(std::move(alloc_ctor));
        ListTestAlloc moved_from_alloc = alloc_ctor.get_allocator();
        // Don't care about the moved from state, so long as it isn't the
        // original
        EXPECT_NE(moved_from_alloc.m_id, 42);
        ListTestAlloc moved_to_alloc = moving_alloc_ctor.get_allocator();
        EXPECT_EQ(moved_to_alloc.m_id, 42);

        rad::List<int, ListTestAlloc> move_assigned;
        move_assigned = std::move(moving_alloc_ctor);
        ListTestAlloc assigned_from_alloc = moving_alloc_ctor.get_allocator();
        // Don't care about the moved from state, so long as it isn't the
        // original
        EXPECT_NE(assigned_from_alloc.m_id, 42);
        ListTestAlloc assigned_to_alloc = move_assigned.get_allocator();
        EXPECT_EQ(assigned_to_alloc.m_id, 42);
    }
    ListTestAlloc::ResetInstrumentation();
}

TEST(List_test, test_push_back_failure_recovery)
{
    ListTestAlloc::ResetInstrumentation();
    {
        rad::List<int, ListTestAlloc> list;

        EXPECT_EQ(STATUS_SUCCESS, list.push_back(1));
        EXPECT_EQ(ListTestAlloc::g_AllocateCallCount, 1);
        EXPECT_EQ(ListTestAlloc::g_FreeCallCount, 0);

        ListTestAlloc::g_ForceAllocateFailure = true;
        EXPECT_EQ(STATUS_NO_MEMORY, list.push_back(2));
        EXPECT_EQ(ListTestAlloc::g_AllocateCallCount, 2);
        EXPECT_EQ(ListTestAlloc::g_FreeCallCount, 0);
        List_equal(list, { 1 });

        ListTestAlloc::g_ForceAllocateFailure = false;
        EXPECT_EQ(STATUS_SUCCESS, list.push_back(3));
        EXPECT_EQ(ListTestAlloc::g_AllocateCallCount, 3);
        EXPECT_EQ(ListTestAlloc::g_FreeCallCount, 0);
        List_equal(list, { 1, 3 });
    }
    EXPECT_EQ(ListTestAlloc::g_AllocateCallCount, 3);
    EXPECT_EQ(ListTestAlloc::g_FreeCallCount, 2);
    ListTestAlloc::ResetInstrumentation();
}

TEST(List_test, default_construct_clear)
{
    rad::List<int> i;
    i.clear();
    EXPECT_TRUE(i.empty());
}

struct CopyStruct
{
    int val = 42;
};

TEST(List_test, copy_push_back)
{
    rad::List<CopyStruct> i;
    CopyStruct local;
    local.val = 42;
    EXPECT_EQ(STATUS_SUCCESS, i.push_back(local));
    List_val_equal(i, { 42 });

    local.val = 99;
    (void)i.push_back(local);
    List_val_equal(i, { 42, 99 });

    local.val = 77;
    (void)i.push_back(local);
    List_val_equal(i, { 42, 99, 77 });
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

TEST(List_test, move_push_back)
{
    rad::List<MoveStruct> i;
    MoveStruct local;
    EXPECT_EQ(STATUS_SUCCESS, i.push_back(std::move(local)));
    List_val_equal(i, { 42 });

    local.val = 99;
    (void)i.push_back(std::move(local));
    List_val_equal(i, { 42, 99 });

    local.val = 77;
    (void)i.push_back(std::move(local));
    List_val_equal(i, { 42, 99, 77 });
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

TEST(List_test, immovable_emplace_back)
{
    rad::List<ImmovableStruct> i;
    EXPECT_EQ(STATUS_SUCCESS, i.emplace_back(42));
    List_val_equal(i, { 42 });

    (void)i.emplace_back(99);
    List_val_equal(i, { 42, 99 });

    (void)i.emplace_back(77);
    List_val_equal(i, { 42, 99, 77 });
}

TEST(List_test, move_construct)
{
    {
        rad::List<int> empty;
        rad::List<int> move_from_empty(std::move(empty));
        List_equal(empty, {});
        List_equal(move_from_empty, {});
    }
    {
        rad::List<int> one;
        EXPECT_EQ(STATUS_SUCCESS, one.push_back(1));
        rad::List<int> move_from_one(std::move(one));

        List_equal(one, {});
        List_equal(move_from_one, { 1 });
    }
    {
        rad::List<int> two;
        EXPECT_EQ(STATUS_SUCCESS, two.push_back(1));
        EXPECT_EQ(STATUS_SUCCESS, two.push_back(2));

        rad::List<int> move_from_two(std::move(two));
        List_equal(two, {});
        List_equal(move_from_two, { 1, 2 });
    }
    {
        rad::List<int> three;
        EXPECT_EQ(STATUS_SUCCESS, three.push_back(1));
        EXPECT_EQ(STATUS_SUCCESS, three.push_back(2));
        EXPECT_EQ(STATUS_SUCCESS, three.push_back(3));

        rad::List<int> move_from_three(std::move(three));
        List_equal(three, {});
        List_equal(move_from_three, { 1, 2, 3 });
    }
    {
        rad::List<int> one;
        EXPECT_EQ(STATUS_SUCCESS, one.push_back(1));

        rad::List<int> move_from_one(std::move(one));

        // ensure we can still mutate after moves
        EXPECT_EQ(STATUS_SUCCESS, one.push_back(101));
        EXPECT_EQ(STATUS_SUCCESS, move_from_one.push_back(201));
        List_equal(one, { 101 });
        List_equal(move_from_one, { 1, 201 });
    }
}

TEST(List_test, clear_some)
{
    rad::List<int> i;
    (void)i.push_back(1);
    EXPECT_FALSE(i.empty());
    i.clear();
    EXPECT_TRUE(i.empty());

    (void)i.push_back(2);
    (void)i.push_back(3);
    EXPECT_EQ(i.expensive_size(), 2);
    i.clear();
    EXPECT_TRUE(i.empty());

    (void)i.push_back(4);
    (void)i.push_back(5);
    (void)i.push_back(6);
    EXPECT_EQ(i.expensive_size(), 3);
    i.clear();
    EXPECT_TRUE(i.empty());
}
