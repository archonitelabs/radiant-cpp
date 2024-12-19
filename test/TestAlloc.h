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

#pragma once

#include "gtest/gtest.h"

#include "radiant/TotallyRad.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

namespace radtest
{

static constexpr uint32_t k_MovedFromState = 0xc001d00d;

class Mallocator
{
public:

    static void* AllocBytes(size_t size)
    {
        return malloc(size);
    }

    static void FreeBytes(void* ptr, size_t size) noexcept
    {
        RAD_UNUSED(size);
        free(ptr);
    }

    static void HandleSizeOverflow()
    {
    }
};

inline void* TaggedAlloc(size_t size, uint32_t tag)
{
    static constexpr size_t kMaxSize = ~uint32_t(0);
    if (size > kMaxSize)
    {
        return nullptr;
    }
    RAD_S_ASSERT(sizeof(max_align_t) >= 2 * sizeof(uint32_t));

    void* mem = malloc(size + sizeof(max_align_t));
    uint32_t* as_num = static_cast<uint32_t*>(mem);
    as_num[0] = tag;
    as_num[1] = static_cast<uint32_t>(size); // range checked above
    return static_cast<max_align_t*>(mem) + 1;
}

inline void TaggedFree(void* ptr, size_t size, uint32_t tag)
{
    if (ptr == nullptr)
    {
        return;
    }
    void* alloc_begin = static_cast<max_align_t*>(ptr) - 1;
    uint32_t* as_num = static_cast<uint32_t*>(alloc_begin);
    EXPECT_EQ(as_num[0], tag);
    EXPECT_EQ(as_num[1], size);
    free(alloc_begin);
}

class StickyTaggedAllocator
{
public:

    // Propagates on false, because this is sticky
    static constexpr bool PropagateOnCopy = false;
    static constexpr bool PropagateOnMoveAssignment = false;
    static constexpr bool PropagateOnSwap = false;
    static constexpr bool IsAlwaysEqual = false;

    explicit StickyTaggedAllocator(uint32_t tag)
        : m_tag(tag)
    {
    }

    void* AllocBytes(size_t size)
    {
        return TaggedAlloc(size, m_tag);
    }

    void FreeBytes(void* ptr, size_t size) noexcept
    {
        TaggedFree(ptr, size, m_tag);
    }

    static void HandleSizeOverflow()
    {
    }

    bool operator==(const StickyTaggedAllocator& rhs) const noexcept
    {
        return m_tag == rhs.m_tag;
    }

    uint32_t m_tag = 0;
};

class StickyDefaultTaggedAllocator : public StickyTaggedAllocator
{
public:

    using StickyTaggedAllocator::StickyTaggedAllocator;

    explicit StickyDefaultTaggedAllocator()
        : StickyTaggedAllocator(1234)
    {
    }
};

class MovingTaggedAllocator
{
public:

    // Propagates on true, because this moves around
    static constexpr bool PropagateOnCopy = true;
    static constexpr bool PropagateOnMoveAssignment = true;
    static constexpr bool PropagateOnSwap = true;
    static constexpr bool IsAlwaysEqual = false;

    explicit MovingTaggedAllocator(uint32_t tag)
        : m_tag(tag)
    {
    }

    void* AllocBytes(size_t size)
    {
        return TaggedAlloc(size, m_tag);
    }

    void FreeBytes(void* ptr, size_t size) noexcept
    {
        TaggedFree(ptr, size, m_tag);
    }

    static void HandleSizeOverflow()
    {
    }

    bool operator==(const MovingTaggedAllocator& rhs) const noexcept
    {
        return m_tag == rhs.m_tag;
    }

    uint32_t m_tag = 0;
};

class StatefulAllocator
{
public:

    static constexpr bool IsAlwaysEqual = false;
    static constexpr bool PropagateOnMoveAssignment = true;
    static constexpr bool PropagateOnCopy = true;
    static constexpr bool PropagateOnSwap = true;

    bool operator==(const StatefulAllocator& other) const
    {
        return m_state == other.m_state;
    }

    static constexpr uint32_t k_BadState = 0xdeadc0de;

    ~StatefulAllocator()
    {
        m_state = k_BadState;
    }

    StatefulAllocator() noexcept
        : m_state(0)
    {
    }

    StatefulAllocator(const StatefulAllocator&) noexcept = default;
    StatefulAllocator& operator=(const StatefulAllocator&) noexcept = default;

    StatefulAllocator(StatefulAllocator&& other) noexcept
        : m_state(other.m_state)
    {
        if (other.m_state != k_BadState)
        {
            other.m_state = k_MovedFromState;
        }
    }

    StatefulAllocator& operator=(StatefulAllocator&& other) noexcept
    {
        m_state = other.m_state;
        if (other.m_state != k_BadState)
        {
            other.m_state = k_MovedFromState;
        }
        return *this;
    }

    void FreeBytes(void* ptr, size_t byte_count) noexcept
    {
        EXPECT_NE(m_state, k_BadState) << "Allocator used after destruction";

        RAD_UNUSED(byte_count);
        free(ptr);
    }

    void* AllocBytes(size_t byte_count) noexcept
    {
        EXPECT_NE(m_state, k_BadState) << "Allocator used after destruction";

        return malloc(byte_count);
    }

    uint32_t m_state;
};

class FailingAllocator
{
public:

    void FreeBytes(void* mem, size_t byte_count) noexcept
    {
        RAD_UNUSED(mem);
        RAD_UNUSED(byte_count);
    }

    void* AllocBytes(size_t byte_count) noexcept
    {
        RAD_UNUSED(byte_count);
        return nullptr;
    }

    static void HandleSizeOverflow()
    {
    }
};

class OOMAllocator
{
public:

    static constexpr bool IsAlwaysEqual = true;

    explicit OOMAllocator(int oom) noexcept
        : m_oom(oom)
    {
    }

    void FreeBytes(void* ptr, size_t byte_count) noexcept
    {
        RAD_UNUSED(byte_count);

        free(ptr);
    }

    void* AllocBytes(size_t byte_count) noexcept
    {
        m_oom--;
        if (m_oom < 0)
        {
            return nullptr;
        }

        return malloc(byte_count);
    }

    static void HandleSizeOverflow()
    {
    }

    int m_oom;
};

class MoveOOMAllocator : public OOMAllocator
{
public:

    static constexpr bool IsAlwaysEqual = false;
    static constexpr bool PropagateOnCopy = true;
    static constexpr bool PropagateOnMoveAssignment = true;
    static constexpr bool PropagateOnSwap = true;

    explicit MoveOOMAllocator(int oom, int id) noexcept
        : OOMAllocator(oom),
          m_id(id)
    {
    }

    bool operator==(const MoveOOMAllocator& other) const noexcept
    {
        return m_id == other.m_id;
    }

    int m_id;
};

class CountingAllocator
{
public:

    static uint32_t g_FreeCount;
    static uint32_t g_AllocCount;
    static size_t g_FreeBytesCount;
    static size_t g_AllocBytesCount;

    void FreeBytes(void* ptr, size_t byte_count) noexcept
    {
        RAD_UNUSED(byte_count);

        if (ptr != nullptr)
        {
            ++g_FreeCount;
            g_FreeBytesCount += byte_count;
        }
        free(ptr);
    }

    void* AllocBytes(size_t byte_count) noexcept
    {
        ++g_AllocCount;
        g_AllocBytesCount += byte_count;
        return malloc(byte_count);
    }

    static void HandleSizeOverflow()
    {
    }

    uint32_t FreeCount() const noexcept
    {
        return g_FreeCount;
    }

    uint32_t AllocCount() const noexcept
    {
        return g_AllocCount;
    }

    void ResetCounts() noexcept
    {
        g_AllocBytesCount = g_FreeBytesCount = g_FreeCount = g_AllocCount = 0;
    }

    void VerifyCounts() const noexcept
    {
        EXPECT_EQ(g_AllocCount, g_FreeCount);
        EXPECT_EQ(g_AllocBytesCount, g_FreeBytesCount);
    }

    void VerifyCounts(uint32_t expectedAllocs,
                      uint32_t expectedFrees) const noexcept
    {
        EXPECT_EQ(g_AllocCount, expectedAllocs);
        EXPECT_EQ(g_FreeCount, expectedFrees);
    }
};

class StatefulCountingAllocator
{
public:

    static constexpr bool IsAlwaysEqual = false;
    static constexpr bool PropagateOnMoveAssignment = true;
    static constexpr bool PropagateOnCopy = true;
    static constexpr bool PropagateOnSwap = true;

    bool operator==(const StatefulCountingAllocator& other) const
    {
        return m_state == other.m_state;
    }

    static uint32_t g_FreeCount;
    static uint32_t g_AllocCount;
    static size_t g_AllocBytesCount;
    static size_t g_FreeBytesCount;

    static constexpr uint32_t k_BadState = 0xdeadc0de;

    ~StatefulCountingAllocator()
    {
        m_state = k_BadState;
    }

    StatefulCountingAllocator() noexcept
        : m_state(0)
    {
    }

    StatefulCountingAllocator(const StatefulCountingAllocator&) noexcept =
        default;

    void FreeBytes(void* ptr, size_t byte_count) noexcept
    {
        RAD_UNUSED(byte_count);

        EXPECT_NE(m_state, k_BadState) << "Allocator used after destruction";
        if (ptr != nullptr)
        {
            ++g_FreeCount;
            g_FreeBytesCount += byte_count;
        }
        free(ptr);
    }

    void* AllocBytes(size_t byte_count) noexcept
    {
        EXPECT_NE(m_state, k_BadState) << "Allocator used after destruction";

        ++g_AllocCount;
        g_AllocBytesCount += byte_count;
        return malloc(byte_count);
    }

    static void HandleSizeOverflow()
    {
    }

    uint32_t FreeCount() const noexcept
    {
        return g_FreeCount;
    }

    uint32_t AllocCount() const noexcept
    {
        return g_AllocCount;
    }

    void ResetCounts() noexcept
    {
        g_AllocBytesCount = g_FreeBytesCount = g_FreeCount = g_AllocCount = 0;
    }

    void VerifyCounts() const noexcept
    {
        EXPECT_EQ(g_AllocCount, g_FreeCount);
        EXPECT_EQ(g_AllocBytesCount, g_FreeBytesCount);
    }

    void VerifyCounts(uint32_t expectedAllocs,
                      uint32_t expectedFrees) const noexcept
    {
        EXPECT_EQ(g_AllocCount, expectedAllocs);
        EXPECT_EQ(g_FreeCount, expectedFrees);
    }

    uint32_t m_state;
};

struct HeapResource
{
    void FreeBytes(void* ptr, size_t byte_count)
    {
        RAD_UNUSED(byte_count);

        if (ptr != nullptr)
        {
            freeCount++;
        }
        free(ptr);
    }

    void* AllocBytes(size_t byte_count)
    {
        if (forceAllocFails > 0)
        {
            forceAllocFails--;
            return nullptr;
        }
        if (forceFutureAllocFail > 0)
        {
            forceFutureAllocFail--;
            if (forceFutureAllocFail == 0)
            {
                return nullptr;
            }
        }

        allocCount++;
        return malloc(byte_count);
    }

    int32_t freeCount{ 0 };

    int32_t forceFutureAllocFail{ 0 };
    int32_t forceAllocFails{ 0 };
    int32_t allocCount{ 0 };
};

template <typename Res>
class ResourceAllocator
{
public:

    static constexpr bool IsAlwaysEqual = false;
    static constexpr bool PropagateOnMoveAssignment = true;
    static constexpr bool PropagateOnCopy = true;
    static constexpr bool PropagateOnSwap = true;

    bool operator==(const ResourceAllocator& other) const noexcept
    {
        return m_res == other.m_res;
    }

    constexpr ResourceAllocator(Res& res) noexcept
        : m_res(&res)
    {
    }

    void FreeBytes(void* ptr, size_t byte_count) noexcept
    {
        m_res->FreeBytes(ptr, byte_count);
    }

    void* AllocBytes(size_t byte_count) noexcept
    {
        return m_res->AllocBytes(byte_count);
    }

    static void HandleSizeOverflow()
    {
    }

    Res* m_res;
};

class TypedAllocator
{
public:

    static constexpr bool IsAlwaysEqual = true;
    static constexpr bool HasTypedAllocations = true;

    template <typename T>
    static constexpr T* Alloc(size_t count)
    {
        if (count > UINT32_MAX / sizeof(T))
        {
            return nullptr;
        }

        void* raw = malloc(count * sizeof(T));
        return static_cast<T*>(raw);
    }

    template <typename T>
    static constexpr void Free(T* ptr, size_t count) noexcept
    {
        RAD_UNUSED(count);
        free(ptr);
    }
};

} // namespace radtest
