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

#include "radiant/TotallyRad.h"
#include "radiant/Byte.h"
#include "radiant/Iterator.h"
#include "radiant/TypeTraits.h"

#include <stddef.h>
#include <stdint.h>

namespace rad
{

using SpanSizeType = uint32_t;
RAD_INLINE_VAR constexpr SpanSizeType DynamicExtent = SpanSizeType(-1);

template <typename T, SpanSizeType N>
class Span;

namespace detail
{

template <SpanSizeType From, SpanSizeType To>
struct SpanIsAllowedExtentConvType
    : IntegralConstant<bool, From == To || To == DynamicExtent>
{
};

template <SpanSizeType From, SpanSizeType To>
RAD_INLINE_VAR constexpr bool SpanIsAllowedExtentConv =
    SpanIsAllowedExtentConvType<From, To>::value;

template <typename From, typename To>
RAD_INLINE_VAR constexpr bool SpanIsAllowedElemTypeConv =
    IsConv<From (*)[], To (*)[]>;

template <typename T, SpanSizeType E, SpanSizeType O, SpanSizeType N>
using SpanMakeSubspanType =
    rad::Span<T, N != DynamicExtent ? N : (E != DynamicExtent ? E - O : E)>;

template <SpanSizeType N>
class SpanExtentType
{
public:

    constexpr SpanExtentType() noexcept = default;

    constexpr explicit SpanExtentType(SpanExtentType<DynamicExtent>) noexcept;

    constexpr explicit SpanExtentType(SpanSizeType size) noexcept
    {
        RAD_UNUSED(size);
        RAD_ASSERT(size == N);
    }

    constexpr SpanSizeType Size() const noexcept
    {
        return N;
    }
};

template <>
class SpanExtentType<DynamicExtent>
{
public:

    template <SpanSizeType U>
    constexpr explicit SpanExtentType(SpanExtentType<U> extent) noexcept
        : m_size(extent.Size())
    {
    }

    constexpr explicit SpanExtentType(SpanSizeType size) noexcept
        : m_size(size)
    {
    }

    constexpr SpanSizeType Size() const noexcept
    {
        return m_size;
    }

private:

    SpanSizeType m_size;
};

template <SpanSizeType N>
constexpr SpanExtentType<N>::SpanExtentType(
    SpanExtentType<DynamicExtent> ext) noexcept
{
    RAD_UNUSED(ext);
    RAD_ASSERT(ext.Size() >= N);
}

} // namespace detail

/// @brief Span object that can refer to a contiguous sequence of objects.
/// @details A span can either have a static extent, in which the number of
/// elements in the sequence is known a at compile-time, or a dynamic extent.
/// @tparam T Element type.
/// @tparam N The number of elements in the sequence, or DynamicExtent.
template <typename T, SpanSizeType N = DynamicExtent>
class Span
{
public:

    using ElementType = T;
    using ValueType = RemoveCV<T>;
    using SizeType = SpanSizeType;
    using PointerType = ElementType*;
    using ConstPointerType = const ElementType*;
    using ReferenceType = ElementType&;
    using ConstReferenceType = const ElementType&;
    using DifferenceType = ptrdiff_t;
    using IteratorType = Iterator<PointerType>;
    using ReverseIteratorType = ReverseIterator<IteratorType>;

    static constexpr SizeType Extent = N;

    ~Span() = default;

    constexpr Span& operator=(const Span&) noexcept = default;

    template <typename U>
    constexpr Span(U, SizeType) = delete;

    template <typename U>
    constexpr Span(U, U) = delete;

    template <SizeType E = Extent,
              EnIf<detail::SpanIsAllowedExtentConv<0, E>, int> = 0>
    constexpr Span() noexcept
        : m_storage(nullptr, detail::SpanExtentType<0>())
    {
    }

    template <SizeType E = Extent, EnIf<E != DynamicExtent, int> = 0>
    constexpr explicit Span(PointerType data, SizeType count) noexcept
        : m_storage(data, count)
    {
    }

    template <SizeType E = Extent,
              EnIf<E != DynamicExtent && IsConst<T>, int> = 0>
    constexpr explicit Span(RemoveConst<T>* data, SizeType count) noexcept
        : m_storage(data, count)
    {
    }

    template <SizeType E = Extent, EnIf<E == DynamicExtent, int> = 0>
    constexpr Span(PointerType data, SizeType count) noexcept
        : m_storage(data, count)
    {
    }

    template <SizeType E = Extent,
              EnIf<E == DynamicExtent && IsConst<T>, int> = 0>
    constexpr Span(RemoveConst<T>* data, SizeType count) noexcept
        : m_storage(data, count)
    {
    }

    template <SizeType E = Extent, EnIf<E != DynamicExtent, int> = 0>
    constexpr explicit Span(PointerType start, PointerType end) noexcept
        : m_storage(start, static_cast<SizeType>(end - start))
    {
    }

    template <SizeType E = Extent,
              EnIf<E != DynamicExtent && IsConst<T>, int> = 0>
    constexpr explicit Span(RemoveConst<T>* start, RemoveConst<T>* end) noexcept
        : m_storage(start, static_cast<SizeType>(end - start))
    {
    }

    template <SizeType E = Extent, EnIf<E == DynamicExtent, int> = 0>
    constexpr Span(PointerType start, PointerType end) noexcept
        : m_storage(start, static_cast<SizeType>(end - start))
    {
    }

    template <SizeType E = Extent,
              EnIf<E == DynamicExtent && IsConst<T>, int> = 0>
    constexpr Span(RemoveConst<T>* start, RemoveConst<T>* end) noexcept
        : m_storage(start, static_cast<SizeType>(end - start))
    {
    }

    template <SizeType C,
              EnIf<detail::SpanIsAllowedExtentConv<C, Extent>, int> = 0>
    constexpr Span(ElementType (&arr)[C]) noexcept
        : m_storage(KnownNotNull{ arr }, detail::SpanExtentType<C>())
    {
    }

    template <typename U,
              SizeType C,
              SizeType E = Extent,
              EnIf<((E == DynamicExtent || E == C) &&
                    detail::SpanIsAllowedElemTypeConv<U, ElementType>),
                   int> = 0>
    constexpr Span(const Span<U, C>& other) noexcept
        : m_storage(other.Data(), detail::SpanExtentType<C>(other.Size()))
    {
    }

    template <typename U,
              SizeType C,
              SizeType E = Extent,
              EnIf<(E != DynamicExtent && C == DynamicExtent &&
                    detail::SpanIsAllowedElemTypeConv<U, ElementType>),
                   int> = 0>
    constexpr explicit Span(const Span<U, C>& other) noexcept
        : m_storage(other.Data(), detail::SpanExtentType<C>(other.Size()))
    {
    }

    constexpr PointerType Data() const noexcept
    {
        return m_storage.Data();
    }

    constexpr SizeType Size() const noexcept
    {
        return m_storage.Size();
    }

    constexpr SizeType SizeBytes() const noexcept
    {
        RAD_ASSERT(Size() < (DynamicExtent / sizeof(ElementType)));
        return static_cast<SizeType>(Size() * sizeof(ElementType));
    }

    constexpr bool Empty() const noexcept
    {
        return (Size() == 0);
    }

    constexpr ReferenceType operator[](SizeType index) const noexcept
    {
        RAD_ASSERT(index < Size());

        return Data()[index];
    }

    constexpr ReferenceType Front() const noexcept
    {
        RAD_ASSERT(!Empty());

        return Data()[0];
    }

    constexpr ReferenceType Back() const noexcept
    {
        RAD_ASSERT(!Empty());

        return Data()[Size() - 1];
    }

    constexpr Span<const Byte, Extent> AsBytes() const noexcept
    {
        return Span<const Byte, Extent>(reinterpret_cast<const Byte*>(Data()),
                                        SizeBytes());
    }

    template <typename U = ElementType, EnIf<!IsConst<U>, int> = 0>
    Span<Byte, Extent> AsBytes() noexcept
    {
        return Span<Byte, Extent>(reinterpret_cast<Byte*>(Data()), SizeBytes());
    }

    template <SizeType O, SizeType C = DynamicExtent>
    constexpr detail::SpanMakeSubspanType<ElementType, Extent, O, C> Subspan()
        const noexcept
    {
        RAD_ASSERT(Size() >= O && (C == DynamicExtent || C <= (Size() - O)));
        using Type = detail::SpanMakeSubspanType<ElementType, Extent, O, C>;
        return Type(Data() + O, C == DynamicExtent ? Size() - O : C);
    }

    constexpr Span<ElementType, DynamicExtent> Subspan(
        SizeType offset, SizeType count = DynamicExtent) const noexcept
    {
        return MakeSubspan(offset, count, SubspanSelector<Extent>());
    }

    template <SizeType C>
    constexpr Span<ElementType, C> First() const noexcept
    {
        RAD_ASSERT(C <= Size());

        return Span<ElementType, C>(Data(), C);
    }

    template <SizeType C>
    constexpr Span<ElementType, C> Last() const noexcept
    {
        RAD_ASSERT(C <= Size());

        return Span<ElementType, C>(Data() + (Size() - C), C);
    }

    constexpr Span<ElementType, DynamicExtent> First(
        SizeType count) const noexcept
    {
        RAD_ASSERT(count <= Size());

        return { Data(), count };
    }

    constexpr Span<ElementType, DynamicExtent> Last(
        SizeType count) const noexcept
    {
        RAD_ASSERT(count <= Size());

        return MakeSubspan(Size() - count,
                           DynamicExtent,
                           SubspanSelector<Extent>());
    }

    constexpr IteratorType begin() const noexcept
    {
        return IteratorType(Data());
    }

    constexpr IteratorType end() const noexcept
    {
        return IteratorType(Data() + Size());
    }

    constexpr ReverseIteratorType rbegin() const noexcept
    {
        return ReverseIteratorType(Data() + Size());
    }

    constexpr ReverseIteratorType rend() const noexcept
    {
        return ReverseIteratorType(Data());
    }

private:

    struct KnownNotNull
    {
        PointerType p;
    };

    template <typename ExtentType>
    class StorageType : public ExtentType
    {
    public:

        template <typename U>
        constexpr StorageType(KnownNotNull data, U extent) noexcept
            : ExtentType(extent),
              m_data(data.p)
        {
        }

        template <typename U>
        constexpr StorageType(PointerType data, U extent) noexcept
            : ExtentType(extent),
              m_data(data)
        {
            RAD_ASSERT(m_data || Size() == 0);
        }

        constexpr PointerType Data() const noexcept
        {
            return m_data;
        }

        using ExtentType::Size;

    private:

        PointerType m_data;
    };

    constexpr Span(KnownNotNull data, SpanSizeType count) noexcept
        : m_storage(data, count)
    {
    }

    template <SizeType C>
    struct SubspanSelector
    {
    };

    template <SizeType C>
    constexpr Span<ElementType, C> MakeSubspan(
        SizeType offset, SizeType count, SubspanSelector<C>) const noexcept
    {
        const Span<ElementType, DynamicExtent> temp(*this);
        return temp.Subspan(offset, count);
    }

    constexpr Span<ElementType, DynamicExtent> MakeSubspan(
        SizeType offset,
        SizeType count,
        SubspanSelector<DynamicExtent>) const noexcept
    {
        RAD_ASSERT(Size() >= offset);

        if (count == DynamicExtent)
        {
            return { KnownNotNull{ Data() + offset }, Size() - offset };
        }

        RAD_ASSERT((Size() - offset) >= count);

        return { KnownNotNull{ Data() + offset }, count };
    }

    StorageType<detail::SpanExtentType<Extent>> m_storage;
};

template <typename T, SpanSizeType N>
constexpr Span<T, N> MakeSpan(T (&arr)[N]) noexcept
{
    return Span<T, N>(arr);
}

template <typename T, SpanSizeType N>
constexpr Span<T, N> MakeSpan(T* data) noexcept
{
    return Span<T, N>(data, N);
}

template <typename T>
constexpr Span<T> MakeSpan(T* data, SpanSizeType count) noexcept
{
    return Span<T>(data, count);
}

template <typename T>
constexpr Span<T> MakeSpan(T* start, T* end) noexcept
{
    return Span<T>(start, end);
}

template <typename T, SpanSizeType TC, typename U, SpanSizeType UC>
constexpr bool SpansOverlap(const Span<T, TC>& left,
                            const Span<U, UC>& right) noexcept
{
    return (((left.begin() >= right.begin()) && (left.begin() < right.end())) ||
            ((right.begin() >= left.begin()) && (right.begin() < left.end())));
}

} // namespace rad
