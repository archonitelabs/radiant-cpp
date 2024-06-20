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

#include "radiant/Res.h"
#include "radiant/Utility.h"
#include "radiant/Span.h"
#include "radiant/Algorithm.h"

namespace rad
{

namespace detail
{

template <typename T, typename TAllocator>
struct VectorAlloc
{
    ~VectorAlloc()
    {
        if (buffer)
        {
            for (T* p = buffer; p != buffer + size; p++)
            {
                p->~T();
            }

            allocator.Free(buffer);
        }
    }

    explicit VectorAlloc(TAllocator& alloc) noexcept
        : allocator(alloc),
          buffer(nullptr),
          size(0),
          capacity(0)
    {
    }

    bool Alloc(uint32_t count)
    {
        buffer = allocator.Alloc(count);
        if (!buffer)
        {
            return false;
        }

        capacity = count;
        size = 0;

        return true;
    }

    T* Release() noexcept
    {
        T* tmp = buffer;
        buffer = nullptr;
        size = 0;
        return tmp;
    }

    TAllocator& allocator;
    T* buffer;
    uint32_t size;
    uint32_t capacity;
};

template <typename T>
struct VectorManipulation
{
    template <typename U = T, EnIf<IsTrivDtor<U>, int> = 0>
    inline void Dtor(T* item) noexcept
    {
        RAD_UNUSED(item);
    }

    template <typename U = T, EnIf<!IsTrivDtor<U>, int> = 0>
    inline void Dtor(T* item) noexcept
    {
        RAD_S_ASSERT_NOTHROW_DTOR(IsNoThrowDtor<T>);

        item->~T();
    }

    template <typename U = T, EnIf<IsTrivDtor<U>, int> = 0>
    inline void DtorRange(T* start, T* end) noexcept
    {
        RAD_UNUSED(start);
        RAD_UNUSED(end);
    }

    template <typename U = T, EnIf<!IsTrivDtor<U>, int> = 0>
    inline void DtorRange(T* start, T* end) noexcept
    {
        RAD_S_ASSERT_NOTHROW_DTOR(IsNoThrowDtor<T>);

        for (T* p = start; p != end; p++)
        {
            p->~T();
        }
    }

    template <typename U = T, EnIf<IsTrivCtor<U>, int> = 0>
    inline void DefaultCtor(T* dest, uint32_t count) noexcept
    {
        memset(dest, 0, count * sizeof(T));
    }

    template <typename U = T, EnIf<!IsTrivCtor<U>, int> = 0>
    inline void DefaultCtor(T* dest, uint32_t count) noexcept(IsNoThrowCtor<T>)
    {
        for (uint32_t i = 0; i < count; i++)
        {
            new (dest + i) T();
        }
    }

    template <typename Out>
    inline void DefaultCtor(Out& dest, uint32_t count)
    {
        for (uint32_t i = 0; i < count; i++)
        {
            new (dest.buffer + i) T();
            dest.size++;
        }
    }

    template <typename U = T,
              EnIf<IsTrivCopyCtor<U> && (sizeof(T) > sizeof(void*)), int> = 0>
    inline void CopyCtor(T* dest, const T& value) noexcept
    {
        memcpy(dest, AddrOf(value), sizeof(T));
    }

    template <typename U = T,
              EnIf<!IsTrivCopyCtor<U> || (sizeof(T) <= sizeof(void*)), int> = 0>
    inline void CopyCtor(T* dest, const T& value) noexcept(IsNoThrowCopyCtor<T>)
    {
        new (dest) T(value);
    }

    inline void CopyCtor(T* dest, uint32_t count, const T& value)
    {
        for (uint32_t i = 0; i < count; i++)
        {
            CopyCtor(dest + i, value);
        }
    }

    template <typename Out>
    inline void CopyCtor(Out& dest, uint32_t count, const T& value)
    {
        for (uint32_t i = 0; i < count; i++)
        {
            CopyCtor(dest.buffer + i, value);
            dest.size++;
        }
    }

    template <typename U = T, EnIf<IsTrivCopyCtor<U>, int> = 0>
    inline void CopyCtorDtorDest(T* dest,
                                 uint32_t count,
                                 const T& value) noexcept
    {
        for (uint32_t i = 0; i < count; i++)
        {
            memcpy(dest + i, AddrOf(value), sizeof(T));
        }
    }

    template <typename U = T, EnIf<!IsTrivCopyCtor<U>, int> = 0>
    inline void CopyCtorDtorDest(T* dest,
                                 uint32_t count,
                                 const T& value) noexcept(IsNoThrowCopyCtor<T>)
    {
        RAD_S_ASSERT_NOTHROW_DTOR(IsNoThrowDtor<T>);

        for (uint32_t i = 0; i < count; i++)
        {
            T* d = dest + i;

            d->~T();
            new (d) T(value);
        }
    }

    template <typename U = T, EnIf<IsTrivCopyCtor<U>, int> = 0>
    inline void CopyCtorDtorDestRange(T* dest, T* src, uint32_t count) noexcept
    {
        memcpy(dest, src, count * sizeof(T));
    }

    template <typename U = T, EnIf<!IsTrivCopyCtor<U>, int> = 0>
    inline void CopyCtorDtorDestRange(T* dest, T* src, uint32_t count) noexcept(
        IsNoThrowCopyCtor<T>)
    {
        RAD_S_ASSERT_NOTHROW_DTOR(IsNoThrowDtor<T>);

        for (uint32_t i = 0; i < count; i++)
        {
            T* d = dest + i;
            T* s = src + i;

            d->~T();
            new (d) T(*s);
        }
    }

    template <typename U = T, EnIf<IsTrivMoveCtor<U>, int> = 0>
    inline void MoveCtorRange(T* dest, T* src, uint32_t count) noexcept
    {
        memmove(dest, src, count * sizeof(T));
    }

    template <typename U = T, EnIf<!IsTrivMoveCtor<U>, int> = 0>
    inline void MoveCtorRange(T* dest,
                              T* src,
                              uint32_t count) noexcept(IsNoThrowMoveCtor<T>)
    {
        RAD_S_ASSERT_NOTHROW_DTOR(IsNoThrowDtor<T>);

        for (uint32_t i = 0; i < count; i++)
        {
            T* s = src + i;
            T* d = dest + i;

            new (d) T(Move(*s));
        }
    }

    template <typename U = T, EnIf<IsTrivMoveCtor<U>, int> = 0>
    inline void MoveCtorDtorSrcRange(T* dest, T* src, uint32_t count) noexcept
    {
        //
        // GCC optimizations gets confused here when inlining a vector with
        // "inline" storage from ShrinkToFit when m_size <= InlineCount.
        //
#if !RAD_DBG && defined(RAD_GCC_VERSION)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif
        memmove(dest, src, count * sizeof(T));
#if !RAD_DBG && defined(RAD_GCC_VERSION)
#pragma GCC diagnostic pop
#endif
    }

    template <typename U = T, EnIf<!IsTrivMoveCtor<U>, int> = 0>
    inline void MoveCtorDtorSrcRange(T* dest, T* src, uint32_t count) noexcept(
        IsNoThrowMoveCtor<T>)
    {
        RAD_S_ASSERT_NOTHROW_DTOR(IsNoThrowDtor<T>);

        for (uint32_t i = 0; i < count; i++)
        {
            T* s = src + i;
            T* d = dest + i;

            new (d) T(Move(*s));
            s->~T();
        }
    }

    template <typename Out, typename U = T, EnIf<IsTrivCopyCtor<U>, int> = 0>
    inline void CopyCtorRange(Out& dest, const T* src, uint32_t count) noexcept
    {
        memcpy(dest.buffer, src, count * sizeof(T));
        dest.size = count;
    }

    template <typename Out, typename U = T, EnIf<!IsTrivCopyCtor<U>, int> = 0>
    inline void CopyCtorRange(Out& dest, const T* src, uint32_t count)
    {
        if (dest.size > 0)
        {
            DtorRange(dest.buffer, dest.buffer + dest.size);
        }

        for (uint32_t i = 0; i < count; i++)
        {
            CopyCtor(dest.buffer + i, src[i]);
            dest.size++;
        }
    }

    template <typename U = T, EnIf<IsTrivCopyCtor<U>, int> = 0>
    inline void CopyCtorRange(T* dest, const T* src, uint32_t count) noexcept
    {
        memcpy(dest, src, count * sizeof(T));
    }

    template <typename U = T, EnIf<!IsTrivCopyCtor<U>, int> = 0>
    inline void CopyCtorRange(T* dest, const T* src, uint32_t count)
    {

        for (uint32_t i = 0; i < count; i++)
        {
            CopyCtor(dest + i, src[i]);
        }
    }

    template <typename U = T, EnIf<IsTrivMoveAssign<U>, int> = 0>
    inline void MoveAssignRange(T* dest, T* src, uint32_t count) noexcept
    {
        memmove(dest, src, count * sizeof(T));
    }

    template <typename U = T, EnIf<!IsTrivMoveAssign<U>, int> = 0>
    inline void MoveAssignRange(T* dest,
                                T* src,
                                uint32_t count) noexcept(IsNoThrowMoveAssign<T>)
    {
        for (uint32_t i = 0; i < count; i++)
        {
            T* s = src + i;
            T* d = dest + i;

            *d = Move(*s);
        }
    }
};

template <typename T, uint16_t TInlineCount, bool = (TInlineCount > 0)>
struct VectorStorage;

template <typename T, uint16_t TInlineCount>
struct VectorStorage<T, TInlineCount, false>
{
    using ThisType = VectorStorage<T, TInlineCount>;
    using ValueType = T;
    using SizeType = uint32_t;
    static constexpr uint16_t InlineCount = TInlineCount;
    using ManipType = VectorManipulation<T>;

    ~VectorStorage() = default;

    VectorStorage() noexcept
        : m_data(nullptr),
          m_size(0),
          m_capacity(0)
    {
    }

    bool IsInline() const noexcept
    {
        return false;
    }

    ValueType* Data() noexcept
    {
        return m_data;
    }

    const ValueType* Data() const noexcept
    {
        return m_data;
    }

    template <typename TAllocator>
    void Free(TAllocator& alloc) noexcept
    {
        if (m_data)
        {
            alloc.Free(m_data);
        }
    }

    template <typename TAllocator>
    Err ShrinkToFit(TAllocator& alloc) noexcept
    {
        if (m_size == m_capacity)
        {
            return NoError;
        }

        if (m_size == 0)
        {
            Free(alloc);
            m_data = nullptr;
            m_capacity = 0;

            return NoError;
        }

        VectorAlloc<ValueType, TAllocator> vec(alloc);
        vec.buffer = vec.allocator.Alloc(m_size);
        if (!vec.buffer)
        {
            return Error::NoMemory;
        }

        ManipType().MoveCtorDtorSrcRange(vec.buffer, m_data, m_size);

        Free(alloc);

        m_data = vec.Release();
        m_capacity = m_size;

        return NoError;
    }

    void Swap(ThisType& other) noexcept
    {
        if (this == &other)
        {
            return;
        }

        rad::Swap(m_data, other.m_data);
        rad::Swap(m_size, other.m_size);
        rad::Swap(m_capacity, other.m_capacity);
    }

    template <typename TAllocator>
    void Swap(VectorAlloc<ValueType, TAllocator>& other) noexcept
    {
        rad::Swap(m_data, other.buffer);
        rad::Swap(m_size, other.size);
        rad::Swap(m_capacity, other.capacity);
    }

    ValueType* m_data;
    SizeType m_size;
    SizeType m_capacity;
};

template <typename T, uint16_t TInlineCount>
struct VectorStorage<T, TInlineCount, true>
{
    using ThisType = VectorStorage<T, TInlineCount>;
    using ValueType = T;
    using SizeType = uint32_t;
    static constexpr uint16_t InlineCount = TInlineCount;
    using ManipType = VectorManipulation<T>;

    ~VectorStorage()
    {
    }

    VectorStorage() noexcept
        : m_data(nullptr),
          m_size(0),
          m_capacity(InlineCount)
    {
    }

    bool IsInline() const noexcept
    {
        return (m_capacity <= InlineCount);
    }

    ValueType* Data() noexcept
    {
        return IsInline() ? m_inline : m_data;
    }

    const ValueType* Data() const noexcept
    {
        return IsInline() ? m_inline : m_data;
    }

    template <typename TAllocator>
    void Free(TAllocator& alloc) noexcept
    {
        if (!IsInline() && m_data)
        {
            alloc.Free(m_data);
        }
    }

    template <typename TAllocator>
    Err ShrinkToFit(TAllocator& alloc) noexcept
    {
        if (IsInline())
        {
            return NoError;
        }

        if (m_size == m_capacity)
        {
            return NoError;
        }

        if (m_size == 0)
        {
            Free(alloc);
            m_data = nullptr;
            m_capacity = InlineCount;
            return NoError;
        }

        if (m_size <= InlineCount)
        {
            //
            // Shrinking the vector will fit inline, make it so.
            //
            auto data = m_data;
            ManipType().MoveCtorDtorSrcRange(m_inline, data, m_size);
            if (data)
            {
                alloc.Free(data);
            }
            m_capacity = InlineCount;
        }
        else
        {
            VectorAlloc<ValueType, TAllocator> vec(alloc);
            vec.buffer = vec.allocator.Alloc(m_size);
            if (!vec.buffer)
            {
                return Error::NoMemory;
            }

            ManipType().MoveCtorDtorSrcRange(vec.buffer, m_data, m_size);

            Free(alloc);

            m_data = vec.Release();
            m_capacity = m_size;
        }

        return NoError;
    }

    void Swap(ThisType& other) noexcept
    {
        if (this == &other)
        {
            return;
        }

        if (!IsInline() && !other.IsInline())
        {
            //
            // Neither are inline, just swap the pointers.
            //
            rad::Swap(m_data, other.m_data);
        }
        else if (IsInline() && other.IsInline())
        {
            //
            // Both are inline, move the data between the inline buffers
            // by using some stack.
            //
            uint8_t storage[sizeof(m_inline)];
            auto buffer = reinterpret_cast<ValueType*>(storage);
            ManipType().MoveCtorDtorSrcRange(buffer, m_inline, m_size);
            ManipType().MoveCtorDtorSrcRange(m_inline,
                                             other.m_inline,
                                             other.m_size);
            ManipType().MoveCtorDtorSrcRange(other.m_inline, buffer, m_size);
        }
        else
        {
            //
            // One is inline and the other isn't. We guarantee there is enough
            // space in the inline data to store the other's information.
            //   1. Move the inline data.
            //   2. Store the allocated buffer in the object we moved data from.
            //   3. Point the data at the inline data.
            //

            if (IsInline())
            {
                //
                // This is inline, move this data to the other's inline.
                //
                auto data = other.m_data;
                ManipType().MoveCtorDtorSrcRange(other.m_inline,
                                                 m_inline,
                                                 m_size);
                m_data = data;
            }
            else
            {
                //
                // Other is inline, move the other's data to this inline.
                //
                auto data = m_data;
                ManipType().MoveCtorDtorSrcRange(m_inline,
                                                 other.m_inline,
                                                 other.m_size);
                other.m_data = data;
            }
        }

        rad::Swap(m_size, other.m_size);
        rad::Swap(m_capacity, other.m_capacity);
    }

    union
    {
        ValueType* m_data;
        ValueType m_inline[InlineCount];
    };

    SizeType m_size;
    SizeType m_capacity;
};

template <typename T, uint16_t TInlineCount>
struct VectorOperations : public VectorStorage<T, TInlineCount>
{
    using ThisType = VectorOperations<T, TInlineCount>;
    using ValueType = T;
    using SizeType = uint32_t;
    static constexpr uint16_t InlineCount = TInlineCount;
    using BaseType = VectorStorage<T, TInlineCount>;
    using ManipType = VectorManipulation<T>;
    using ConstIteratorType = Iterator<const T*>;

    SizeType GrowthFor(SizeType size) noexcept
    {
        if (m_capacity > UINT32_MAX - m_capacity / 2)
        {
            return UINT32_MAX;
        }

        SizeType growth = m_capacity + m_capacity / 2;
        if (growth < size)
        {
            return size;
        }

        return growth;
    }

    void Clear() noexcept
    {
        ManipType().DtorRange(Data(), Data() + m_size);
        m_size = 0;
    }

    template <typename TAllocator>
    Err Reserve(TAllocator& alloc, SizeType capacity) noexcept
    {
        if (capacity <= m_capacity)
        {
            return NoError;
        }

        VectorAlloc<ValueType, TAllocator> vec(alloc);
        if (!vec.Alloc(capacity))
        {
            return Error::NoMemory;
        }

        ManipType().MoveCtorDtorSrcRange(vec.buffer, Data(), m_size);

        Free(alloc);

        m_data = vec.Release();
        m_capacity = capacity;

        return NoError;
    }

    template <typename TAllocator>
    void Move(TAllocator& alloc, ThisType& to) noexcept
    {
        if RAD_UNLIKELY (this == &to)
        {
            return;
        }

        Swap(to);
        Clear();
        RAD_VERIFY(ShrinkToFit(alloc).IsOk());
    }

    template <typename TAllocator,
              typename U = T,
              EnIf<!IsNoThrowCopyCtor<U>, int> = 0>
    Err Copy(TAllocator& alloc, ThisType& to)
    {
        if RAD_UNLIKELY (this == &to)
        {
            return NoError;
        }

        // Here we want to provide the strong guarantee, but if the copy
        // constructor can throw we cannot just overwrite the memory in the
        // old vector because if an exception happens we will wind up with a
        // half edited vector.  In the worst case the exception happens copying
        // the last item.  So, we have to maintain a copy of the entire old
        // vector somewhere until the copy is complete.  The amount of memory
        // we need to have is this->Size + to.Size().  For now we have
        // implemented the basic thing, but should/will optimize further.  Two
        // easy things we could are to use unused space in the vectors and/or
        // some stack space to avoid expensive memory allocations when possible.
        VectorAlloc<ValueType, TAllocator> vec(alloc);
        if (!vec.Alloc(m_size))
        {
            return Error::NoMemory;
        }

        ManipType().CopyCtorRange(vec, Data(), m_size);
        to.Swap(vec);

        return NoError;
    }

    template <typename TAllocator,
              typename U = T,
              EnIf<IsNoThrowCopyCtor<U>, int> = 0>
    Err Copy(TAllocator& alloc, ThisType& to) noexcept
    {
        if RAD_UNLIKELY (this == &to)
        {
            return NoError;
        }

        Err res = to.Reserve(alloc, m_size);
        if (!res.IsOk())
        {
            return res.Err();
        }

        if (to.m_size >= m_size)
        {
            ManipType().CopyCtorDtorDestRange(to.Data(), Data(), m_size);

            if (to.m_size > m_size)
            {
                ManipType().DtorRange(to.Data() + m_size,
                                      to.Data() + to.m_size);
            }
        }
        else
        {
            ManipType().DtorRange(to.Data(), to.Data() + to.m_size);
            ManipType().CopyCtorRange(to.Data(), Data(), m_size);
        }

        to.m_size = m_size;

        return NoError;
    }

    void PopBack() noexcept
    {
        if RAD_LIKELY (m_size > 0)
        {
            --m_size;
            ManipType().Dtor(Data() + m_size);
        }
    }

    template <typename TAllocator,
              typename U = T,
              EnIf<!IsNoThrowCopyCtor<U> || !IsNoThrowDefaultCtor<U>, int> = 0>
    Err Resize(TAllocator& alloc,
               SizeType count,
               const ValueType& value = ValueType())
    {
        if RAD_UNLIKELY (count == m_size)
        {
            return NoError;
        }

        if (count < m_size)
        {
            ManipType().DtorRange(Data() + count, Data() + m_size);
        }
        else
        {
            if (count > m_capacity)
            {
                Err res = Reserve(alloc, GrowthFor(count));
                if (!res.IsOk())
                {
                    return res.Err();
                }
            }

            // TODO: Optimize out this allocation when possible.  See, comment
            // in the copy method for further explanation.
            VectorAlloc<ValueType, TAllocator> vec(alloc);
            if (!vec.Alloc(count - m_size))
            {
                return Error::NoMemory;
            }
            ManipType().CopyCtor(vec, count - m_size, value);
            ManipType().MoveCtorRange(Data() + m_size, vec.buffer, vec.size);
        }

        m_size = count;

        return NoError;
    }

    template <typename TAllocator,
              typename U = T,
              EnIf<IsNoThrowCopyCtor<U> && IsNoThrowDefaultCtor<U>, int> = 0>
    Err Resize(TAllocator& alloc,
               SizeType count,
               const ValueType& value = ValueType()) noexcept
    {
        if RAD_UNLIKELY (count == m_size)
        {
            return NoError;
        }

        if (count < m_size)
        {
            ManipType().DtorRange(Data() + count, Data() + m_size);
        }
        else
        {
            if (count > m_capacity)
            {
                Err res = Reserve(alloc, GrowthFor(count));
                if (!res.IsOk())
                {
                    return res.Err();
                }
            }

            ManipType().CopyCtor(Data() + m_size, count - m_size, value);
        }

        m_size = count;

        return NoError;
    }

    template <typename TAllocator,
              typename U = T,
              EnIf<!IsNoThrowCopyCtor<U>, int> = 0>
    Err Assign(TAllocator& alloc, SizeType count, const ValueType& value)
    {
        // TODO: Optimize out this allocation when possible.  See, comment
        // in the copy method for further explanation
        VectorAlloc<ValueType, TAllocator> vec(alloc);
        if (!vec.Alloc(count))
        {
            return Error::NoMemory;
        }

        ManipType().CopyCtor(vec, count, value);
        ManipType().DtorRange(Data(), Data() + m_size);

        if (count <= m_capacity)
        {
            // As an optimization we could decide to use the inline storage for
            // small values of count and avoid the memory allocation.
            ManipType().MoveCtorRange(Data(), vec.buffer, count);
        }
        else
        {
            m_data = vec.Release();
            m_capacity = count;
        }

        m_size = count;

        return NoError;
    }

    template <typename TAllocator,
              typename U = T,
              EnIf<IsNoThrowCopyCtor<U>, int> = 0>
    Err Assign(TAllocator& alloc,
               SizeType count,
               const ValueType& value) noexcept
    {
        if (count <= m_size)
        {
            ManipType().CopyCtorDtorDest(Data(), count, value);
            ManipType().DtorRange(Data() + count, Data() + m_size);
        }
        else if (count <= m_capacity)
        {
            ManipType().CopyCtorDtorDest(Data(), m_size, value);
            ManipType().CopyCtor(Data() + m_size, count - m_size, value);
        }
        else
        {
            VectorAlloc<ValueType, TAllocator> vec(alloc);
            vec.buffer = vec.allocator.Alloc(count);
            if (!vec.buffer)
            {
                return Error::NoMemory;
            }

            ManipType().CopyCtor(vec.buffer, count, value);
            ManipType().DtorRange(Data(), Data() + m_size);

            m_data = vec.Release();
            m_capacity = count;
        }

        m_size = count;

        return NoError;
    }

    template <typename TAllocator,
              typename U = T,
              EnIf<!IsNoThrowCopyCtor<U>, int> = 0>
    Err Assign(TAllocator& alloc, Span<const ValueType> span)
    {
        if (SpansOverlap(span, Span<const ValueType>(Data(), m_size)))
        {
            return Error::InvalidAddress;
        }

        // TODO: Optimize out this allocation when possible.  See, comment
        // in the copy method for further explanation
        VectorAlloc<ValueType, TAllocator> vec(alloc);
        if (!vec.Alloc(span.Size()))
        {
            return Error::NoMemory;
        }

        ManipType().CopyCtorRange(vec, span.Data(), span.Size());
        ManipType().DtorRange(Data(), Data() + m_size);

        if (span.Size() <= m_capacity)
        {
            // As an optimization we could decide to use the inline storage for
            // small values of count and avoid the memory allocation.
            ManipType().MoveCtorRange(Data(), vec.buffer, vec.size);
        }
        else
        {
            m_data = vec.Release();
            m_capacity = span.Size();
        }

        m_size = span.Size();

        return NoError;
    }

    template <typename TAllocator,
              typename U = T,
              EnIf<IsNoThrowCopyCtor<U>, int> = 0>
    Err Assign(TAllocator& Allocator, Span<const ValueType> span) noexcept
    {
        if (SpansOverlap(span, Span<const ValueType>(Data(), m_size)))
        {
            return Error::InvalidAddress;
        }

        Err res = Reserve(Allocator, span.Size());
        if (!res.IsOk())
        {
            return res.Err();
        }

        ValueType* data = Data();
        ValueType* end = Data() + m_size;
        for (const auto& entry : span)
        {
            if (data < end)
            {
                ManipType().Dtor(data);
            }

            ManipType().CopyCtor(data, entry);

            data++;
        }

        if (data < end)
        {
            ManipType().DtorRange(data, end);
        }

        m_size = span.Size();

        return NoError;
    }

    template <typename TAllocator, typename... TArgs>
    Err EmplaceBack(TAllocator& alloc,
                    TArgs&&... args) noexcept(IsNoThrowCtor<T, TArgs...>)
    {
        if RAD_LIKELY (RAD_VERIFY(m_size < UINT32_MAX))
        {
            if (m_size + 1 > m_capacity)
            {
                Err res = Reserve(alloc, GrowthFor(m_size + 1));
                if (!res.IsOk())
                {
                    return res;
                }
            }

            new (AddrOf(Data()[m_size])) ValueType(Forward<TArgs>(args)...);
            m_size++;

            return NoError;
        }

        return Error::IntegerOverflow;
    }

    using BaseType::Data;
    using BaseType::Free;
    using BaseType::Swap;
    using BaseType::ShrinkToFit;

    using BaseType::m_data;
    using BaseType::m_size;
    using BaseType::m_capacity;
};

} // namespace detail

} // namespace rad
