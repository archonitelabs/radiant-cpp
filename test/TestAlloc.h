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

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

namespace radtest
{

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

    constexpr Allocator() = default;

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

    ~StatefulAllocator() = default;

    StatefulAllocator()
        : value(0)
    {
    }

    StatefulAllocator(const StatefulAllocator&) noexcept = default;

    template <typename U>
    StatefulAllocator(const StatefulAllocator<U>& other)
        : value(other.value)
    {
    }

    template <typename U>
    struct Rebind
    {
        using Other = StatefulAllocator<U>;
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

    uint32_t value;
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

    constexpr FailingAllocator() = default;

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

    ~CountingAllocator() = default;

    constexpr CountingAllocator() = default;

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
        ++freeCount;
        free(ptr);
    }

    ValueType* Alloc(SizeType count) noexcept
    {
        ++allocCount;
        return (ValueType*)malloc(count * sizeof(T));
    }

    ValueType* Realloc(ValueType* ptr, SizeType count) noexcept
    {
        ++reallocCount;
        return (ValueType*)realloc(ptr, count * sizeof(T));
    }

    void FreeBytes(void* ptr) noexcept
    {
        ++freeBytesCount;
        free(ptr);
    }

    void* AllocBytes(SizeType size) noexcept
    {
        ++allocBytesCount;
        return malloc(size);
    }

    void* ReallocBytes(void* ptr, SizeType size) noexcept
    {
        ++reallocBytesCount;
        return realloc(ptr, size);
    }

    uint32_t freeCount;
    uint32_t allocCount;
    uint32_t reallocCount;
    uint32_t freeBytesCount;
    uint32_t allocBytesCount;
    uint32_t reallocBytesCount;
};

struct StaticCountingAllocatorImpl
{
    static void Free(void* ptr) noexcept;
    static void* Alloc(uint32_t size) noexcept;
    static void* Realloc(void* ptr, uint32_t size) noexcept;
    static void FreeBytes(void* ptr) noexcept;
    static void* AllocBytes(uint32_t size) noexcept;
    static void* ReallocBytes(void* ptr, uint32_t size) noexcept;

    static uint32_t freeCount;
    static uint32_t allocCount;
    static uint32_t reallocCount;
    static uint32_t freeBytesCount;
    static uint32_t allocBytesCount;
    static uint32_t reallocBytesCount;
};

template <typename T>
class StaticCountingAllocator
{
public:

    static constexpr bool NeedsFree = true;
    static constexpr bool HasRealloc = true;
    static constexpr bool HasAllocBytes = true;

    using ThisType = StaticCountingAllocator<T>;
    using ValueType = T;
    using SizeType = uint32_t;
    using DifferenceType = ptrdiff_t;

    using Impl = StaticCountingAllocatorImpl;

    ~StaticCountingAllocator() = default;

    StaticCountingAllocator()
        : state(0)
    {
    }

    StaticCountingAllocator(const StaticCountingAllocator&) noexcept = default;

    template <typename U>
    StaticCountingAllocator(const StaticCountingAllocator<U>& other)
        : state(other.state)
    {
    }

    template <typename U>
    struct Rebind
    {
        using Other = StaticCountingAllocator<U>;
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
        return Impl::freeCount;
    }

    uint32_t AllocCount() const noexcept
    {
        return Impl::allocCount;
    }

    uint32_t ReallocCount() const noexcept
    {
        return Impl::reallocCount;
    }

    uint32_t FreeBytesCount() const noexcept
    {
        return Impl::freeBytesCount;
    }

    uint32_t AllocBytesCount() const noexcept
    {
        return Impl::allocBytesCount;
    }

    uint32_t ReallocBytesCount() const noexcept
    {
        return Impl::reallocBytesCount;
    }

    void ResetCounts() noexcept
    {
        Impl::freeCount = 0;
        Impl::allocCount = 0;
        Impl::reallocCount = 0;
        Impl::freeBytesCount = 0;
        Impl::allocBytesCount = 0;
        Impl::reallocBytesCount = 0;
    }

    bool VerifyCounts() const noexcept
    {
        return Impl::allocCount == Impl::freeCount;
    }

    bool VerifyCounts(uint32_t expectedAllocs,
                      uint32_t expectedFrees) const noexcept
    {
        return Impl::allocCount == expectedAllocs &&
               Impl::freeCount == expectedFrees;
    }

    uint32_t state; // make sure stateful
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

    constexpr AllocWrapper(TBase& alloc)
        : base(&alloc)
    {
    }

    constexpr AllocWrapper(const AllocWrapper&) noexcept = default;

    template <typename U>
    constexpr AllocWrapper(const AllocWrapper<U, TBase>& other)
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
