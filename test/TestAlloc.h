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

namespace radtest
{

static constexpr uint32_t k_BadState = 0xdeadc0de;

template <typename T>
class Allocator
{
public:

    static constexpr bool NeedsFree = true;
    static constexpr bool HasRealloc = true;
    static constexpr bool HasAllocBytes = true;

    using ThisType = Allocator<T>;
    using ValueType = T;
    using SizeType = uint32_t;
    using DifferenceType = ptrdiff_t;

    ~Allocator() = default;

    constexpr Allocator() noexcept = default;

    constexpr Allocator(const Allocator&) noexcept = default;

    template <typename U>
    constexpr Allocator(const Allocator<U>&) noexcept
    {
    }

    template <typename U>
    struct Rebind
    {
        using Other = Allocator<U>;
    };

    void Free(ValueType* ptr) noexcept
    {
        free(ptr);
    }

    ValueType* Alloc(SizeType count) noexcept
    {
        return (ValueType*)malloc(count * sizeof(T));
    }

    ValueType* Realloc(ValueType* ptr, SizeType count) noexcept
    {
        return (ValueType*)realloc(ptr, count * sizeof(T));
    }

    void FreeBytes(void* ptr) noexcept
    {
        free(ptr);
    }

    void* AllocBytes(SizeType size) noexcept
    {
        return malloc(size);
    }

    void* ReallocBytes(void* ptr, SizeType size) noexcept
    {
        return realloc(ptr, size);
    }
};

template <typename T>
class StatefulAllocator
{
public:

    static constexpr bool NeedsFree = true;
    static constexpr bool HasRealloc = true;
    static constexpr bool HasAllocBytes = true;

    using ThisType = StatefulAllocator<T>;
    using ValueType = T;
    using SizeType = uint32_t;
    using DifferenceType = ptrdiff_t;

    ~StatefulAllocator()
    {
        m_state = k_BadState;
    }

    StatefulAllocator() noexcept
        : m_state(0)
    {
    }

    StatefulAllocator(const StatefulAllocator&) noexcept = default;

    template <typename U>
    StatefulAllocator(const StatefulAllocator<U>& other) noexcept
        : m_state(other.m_state)
    {
    }

    template <typename U>
    struct Rebind
    {
        using Other = StatefulAllocator<U>;
    };

    void Free(ValueType* ptr) noexcept
    {
        EXPECT_NE(m_state, k_BadState) << "Allocator used after destruction";

        free(ptr);
    }

    ValueType* Alloc(SizeType count) noexcept
    {
        EXPECT_NE(m_state, k_BadState) << "Allocator used after destruction";

        return (ValueType*)malloc(count * sizeof(T));
    }

    ValueType* Realloc(ValueType* ptr, SizeType count) noexcept
    {
        EXPECT_NE(m_state, k_BadState) << "Allocator used after destruction";

        return (ValueType*)realloc(ptr, count * sizeof(T));
    }

    void FreeBytes(void* ptr) noexcept
    {
        EXPECT_NE(m_state, k_BadState) << "Allocator used after destruction";

        free(ptr);
    }

    void* AllocBytes(SizeType size) noexcept
    {
        EXPECT_NE(m_state, k_BadState) << "Allocator used after destruction";

        return malloc(size);
    }

    uint32_t m_state;
};

template <typename T>
class FailingAllocator
{
public:

    static constexpr bool NeedsFree = true;
    static constexpr bool HasRealloc = true;
    static constexpr bool HasAllocBytes = true;

    using ThisType = FailingAllocator<T>;
    using ValueType = T;
    using SizeType = uint32_t;
    using DifferenceType = ptrdiff_t;

    ~FailingAllocator() = default;

    constexpr FailingAllocator() noexcept = default;

    constexpr FailingAllocator(const FailingAllocator&) noexcept = default;

    template <typename U>
    constexpr FailingAllocator(const FailingAllocator<U>&) noexcept
    {
    }

    template <typename U>
    struct Rebind
    {
        using Other = FailingAllocator<U>;
    };

    void Free(ValueType*) noexcept
    {
    }

    ValueType* Alloc(SizeType) noexcept
    {
        return nullptr;
    }

    ValueType* Realloc(ValueType*, SizeType) noexcept
    {
        return nullptr;
    }

    void FreeBytes(void*) noexcept
    {
    }

    void* AllocBytes(SizeType) noexcept
    {
        return nullptr;
    }

    void* ReallocBytes(void*, SizeType) noexcept
    {
        return nullptr;
    }
};

template <typename T>
class OOMAllocator
{
public:

    static constexpr bool NeedsFree = true;
    static constexpr bool HasRealloc = true;
    static constexpr bool HasAllocBytes = true;

    using ThisType = StatefulAllocator<T>;
    using ValueType = T;
    using SizeType = uint32_t;
    using DifferenceType = ptrdiff_t;

    ~OOMAllocator()
    {
    }

    explicit OOMAllocator(int oom) noexcept
        : m_oom(oom)
    {
    }

    OOMAllocator(const OOMAllocator&) noexcept = default;

    template <typename U>
    OOMAllocator(const OOMAllocator<U>& other) noexcept
        : m_oom(other.m_oom)
    {
    }

    template <typename U>
    struct Rebind
    {
        using Other = OOMAllocator<U>;
    };

    void Free(ValueType* ptr) noexcept
    {
        free(ptr);
    }

    ValueType* Alloc(SizeType count) noexcept
    {
        m_oom--;
        if (m_oom < 0)
        {
            return nullptr;
        }

        return (ValueType*)malloc(count * sizeof(T));
    }

    ValueType* Realloc(ValueType* ptr, SizeType count) noexcept
    {
        m_oom--;
        if (m_oom < 0)
        {
            return nullptr;
        }

        return (ValueType*)realloc(ptr, count * sizeof(T));
    }

    void FreeBytes(void* ptr) noexcept
    {
        free(ptr);
    }

    void* AllocBytes(SizeType size) noexcept
    {
        m_oom--;
        if (m_oom < 0)
        {
            return nullptr;
        }

        return malloc(size);
    }

    int m_oom;
};

class CountingAllocatorImpl
{
public:

    static void Free(void* ptr) noexcept;
    static void* Alloc(uint32_t size) noexcept;
    static void* Realloc(void* ptr, uint32_t size) noexcept;
    static void FreeBytes(void* ptr) noexcept;
    static void* AllocBytes(uint32_t size) noexcept;
    static void* ReallocBytes(void* ptr, uint32_t size) noexcept;
    static uint32_t FreeCount() noexcept;
    static uint32_t AllocCount() noexcept;
    static uint32_t ReallocCount() noexcept;
    static uint32_t FreeBytesCount() noexcept;
    static uint32_t AllocBytesCount() noexcept;
    static uint32_t ReallocBytesCount() noexcept;
    static void ResetCounts() noexcept;
    static bool VerifyCounts() noexcept;
    static bool VerifyCounts(uint32_t expectedAllocs,
                             uint32_t expectedFrees) noexcept;

private:

    static uint32_t g_FreeCount;
    static uint32_t g_AllocCount;
    static uint32_t g_ReallocCount;
    static uint32_t g_FreeBytesCount;
    static uint32_t g_AllocBytesCount;
    static uint32_t g_ReallocBytesCount;
};

template <typename T>
class CountingAllocator
{
public:

    static constexpr bool NeedsFree = true;
    static constexpr bool HasRealloc = true;
    static constexpr bool HasAllocBytes = true;

    using ThisType = CountingAllocator<T>;
    using ValueType = T;
    using SizeType = uint32_t;
    using DifferenceType = ptrdiff_t;

    using Impl = CountingAllocatorImpl;

    ~CountingAllocator() = default;

    constexpr CountingAllocator() noexcept = default;

    constexpr CountingAllocator(const CountingAllocator&) noexcept = default;

    template <typename U>
    constexpr CountingAllocator(const CountingAllocator<U>&) noexcept
    {
    }

    template <typename U>
    struct Rebind
    {
        using Other = CountingAllocator<U>;
    };

    void Free(ValueType* ptr) noexcept
    {
        Impl::Free(ptr);
    }

    ValueType* Alloc(SizeType count) noexcept
    {
        return (ValueType*)Impl::Alloc((sizeof(T) * count));
    }

    ValueType* Realloc(ValueType* ptr, SizeType count) noexcept
    {
        return (ValueType*)Impl::Realloc(ptr, (sizeof(T) * count));
    }

    void FreeBytes(void* ptr) noexcept
    {
        Impl::Free(ptr);
    }

    void* AllocBytes(SizeType size) noexcept
    {
        return (ValueType*)Impl::Alloc(size);
    }

    void* ReallocBytes(void* ptr, SizeType size) noexcept
    {
        return Impl::Realloc(ptr, size);
    }

    uint32_t FreeCount() const noexcept
    {
        return Impl::FreeCount();
    }

    uint32_t AllocCount() const noexcept
    {
        return Impl::AllocCount();
    }

    uint32_t ReallocCount() const noexcept
    {
        return Impl::ReallocCount();
    }

    uint32_t FreeBytesCount() const noexcept
    {
        return Impl::FreeBytesCount();
    }

    uint32_t AllocBytesCount() const noexcept
    {
        return Impl::AllocBytesCount();
    }

    uint32_t ReallocBytesCount() const noexcept
    {
        return Impl::ReallocBytesCount();
    }

    void ResetCounts() noexcept
    {
        Impl::ResetCounts();
    }

    bool VerifyCounts() const noexcept
    {
        return Impl::VerifyCounts();
    }

    bool VerifyCounts(uint32_t expectedAllocs,
                      uint32_t expectedFrees) const noexcept
    {
        return Impl::VerifyCounts(expectedAllocs, expectedFrees);
    }
};

template <typename T>
class StatefulCountingAllocator
{
public:

    static constexpr bool NeedsFree = true;
    static constexpr bool HasRealloc = true;
    static constexpr bool HasAllocBytes = true;

    using ThisType = StatefulCountingAllocator<T>;
    using ValueType = T;
    using SizeType = uint32_t;
    using DifferenceType = ptrdiff_t;

    using Impl = CountingAllocatorImpl;

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

    template <typename U>
    StatefulCountingAllocator(
        const StatefulCountingAllocator<U>& other) noexcept
        : m_state(other.m_state)
    {
    }

    template <typename U>
    struct Rebind
    {
        using Other = StatefulCountingAllocator<U>;
    };

    void Free(ValueType* ptr) noexcept
    {
        EXPECT_NE(m_state, k_BadState) << "Allocator used after destruction";

        Impl::Free(ptr);
    }

    ValueType* Alloc(SizeType count) noexcept
    {
        EXPECT_NE(m_state, k_BadState) << "Allocator used after destruction";

        return (ValueType*)Impl::Alloc((sizeof(T) * count));
    }

    ValueType* Realloc(ValueType* ptr, SizeType count) noexcept
    {
        EXPECT_NE(m_state, k_BadState) << "Allocator used after destruction";

        return (ValueType*)Impl::Realloc(ptr, (sizeof(T) * count));
    }

    void FreeBytes(void* ptr) noexcept
    {
        EXPECT_NE(m_state, k_BadState) << "Allocator used after destruction";

        Impl::Free(ptr);
    }

    void* AllocBytes(SizeType size) noexcept
    {
        EXPECT_NE(m_state, k_BadState) << "Allocator used after destruction";

        return (ValueType*)Impl::Alloc(size);
    }

    void* ReallocBytes(void* ptr, SizeType size) noexcept
    {
        EXPECT_NE(m_state, k_BadState) << "Allocator used after destruction";

        return Impl::Realloc(ptr, size);
    }

    uint32_t FreeCount() const noexcept
    {
        return Impl::FreeCount();
    }

    uint32_t AllocCount() const noexcept
    {
        return Impl::AllocCount();
    }

    uint32_t ReallocCount() const noexcept
    {
        return Impl::ReallocCount();
    }

    uint32_t FreeBytesCount() const noexcept
    {
        return Impl::FreeBytesCount();
    }

    uint32_t AllocBytesCount() const noexcept
    {
        return Impl::AllocBytesCount();
    }

    uint32_t ReallocBytesCount() const noexcept
    {
        return Impl::ReallocBytesCount();
    }

    void ResetCounts() noexcept
    {
        Impl::ResetCounts();
    }

    bool VerifyCounts() const noexcept
    {
        return Impl::VerifyCounts();
    }

    bool VerifyCounts(uint32_t expectedAllocs,
                      uint32_t expectedFrees) const noexcept
    {
        return Impl::VerifyCounts(expectedAllocs, expectedFrees);
    }

    uint32_t m_state;
};

struct HeapAllocator
{
    void Free(void* ptr)
    {
        freeCount++;
        free(ptr);
    }

    void* Alloc(uint32_t count)
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
        return malloc(count);
    }

    void* Realloc(void* ptr, uint32_t count)
    {
        if (forceReallocFails > 0)
        {
            forceReallocFails--;
            return nullptr;
        }

        reallocCount++;
        return realloc(ptr, count);
    }

    void FreeBytes(void* ptr)
    {
        freeBytesCount++;
        free(ptr);
    }

    void* AllocBytes(uint32_t count)
    {
        if (forceAllocBytesFails > 0)
        {
            forceAllocBytesFails--;
            return nullptr;
        }

        allocBytesCount++;
        return malloc(count);
    }

    void* ReallocBytes(void* ptr, uint32_t count)
    {
        if (forceReallocBytesFails > 0)
        {
            forceReallocBytesFails--;
            return nullptr;
        }

        reallocBytesCount++;
        return realloc(ptr, count);
    }

    int32_t freeCount{ 0 };

    int32_t forceFutureAllocFail{ 0 };
    int32_t forceAllocFails{ 0 };
    int32_t allocCount{ 0 };

    uint32_t forceReallocFails{ 0 };
    uint32_t reallocCount{ 0 };

    uint32_t freeBytesCount{ 0 };

    uint32_t forceAllocBytesFails{ 0 };
    uint32_t allocBytesCount{ 0 };

    uint32_t forceReallocBytesFails{ 0 };
    uint32_t reallocBytesCount{ 0 };
};

template <typename T, typename TBase>
class AllocWrapper
{
public:

    static constexpr bool NeedsFree = true;
    static constexpr bool HasRealloc = true;
    static constexpr bool HasAllocBytes = true;

    using ThisType = AllocWrapper<T, TBase>;
    using ValueType = T;
    using SizeType = uint32_t;
    using DifferenceType = ptrdiff_t;

    ~AllocWrapper() = default;

    constexpr AllocWrapper(TBase& alloc) noexcept
        : base(&alloc)
    {
    }

    constexpr AllocWrapper(const AllocWrapper&) noexcept = default;

    template <typename U>
    constexpr AllocWrapper(const AllocWrapper<U, TBase>& other) noexcept
        : base(other.base)
    {
    }

    template <typename U>
    struct Rebind
    {
        using Other = AllocWrapper<U, TBase>;
    };

    void Free(ValueType* ptr) noexcept
    {
        base->Free(ptr);
    }

    ValueType* Alloc(SizeType count) noexcept
    {
        return (ValueType*)base->Alloc(count * sizeof(T));
    }

    ValueType* Realloc(ValueType* ptr, SizeType count) noexcept
    {
        return (ValueType*)base->Realloc(ptr, count * sizeof(T));
    }

    void FreeBytes(void* ptr) noexcept
    {
        base->FreeBytes(ptr);
    }

    void* AllocBytes(SizeType size) noexcept
    {
        return base->AllocBytes(size);
    }

    void* ReallocBytes(void* ptr, SizeType size) noexcept
    {
        return base->ReallocBytes(ptr, size);
    }

    TBase* base;
};

} // namespace radtest
