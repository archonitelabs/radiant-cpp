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
#include "radiant/Utility.h"

namespace rad
{

template <typename T>
struct IteratorTraits
{
    using ValueType = typename T::ValueType;
    using DifferenceType = typename T::DifferenceType;
    using PointerType = typename T::PointerType;
    using ReferenceType = typename T::ReferenceType;
};

template <typename T>
struct IteratorTraits<T*>
{
    using ValueType = T;
    using DifferenceType = ptrdiff_t;
    using PointerType = T*;
    using ReferenceType = T&;
};

template <typename T>
struct IteratorTraits<const T*>
{
    using ValueType = T;
    using DifferenceType = ptrdiff_t;
    using PointerType = const T*;
    using ReferenceType = const T&;
};

template <typename T>
class Iterator
{
public:

    using ThisType = Iterator<T>;
    using TraitsType = IteratorTraits<T>;
    using ValueType = typename TraitsType::ValueType;
    using DifferenceType = typename TraitsType::DifferenceType;
    using PointerType = typename TraitsType::PointerType;
    using ReferenceType = typename TraitsType::ReferenceType;

    ~Iterator() = default;

    constexpr Iterator() noexcept
        : m_current(T())
    {
    }

    constexpr Iterator(T current) noexcept
        : m_current(current)
    {
    }

    constexpr Iterator(const ThisType& other) noexcept
        : m_current(other.m_current)
    {
    }

    constexpr ThisType& operator=(const ThisType& other) noexcept
    {
        m_current = other.m_current;
        return *this;
    }

    template <typename U = T, EnIf<IsPtr<U>, int> = 0>
    constexpr PointerType operator->() const noexcept
    {
        T tmp = m_current;
        return tmp;
    }

    template <typename U = T, EnIf<!IsPtr<U>, int> = 0>
    constexpr PointerType operator->() const noexcept
    {
        T tmp = m_current;
        return tmp.operator->();
    }

    constexpr ReferenceType operator*() const noexcept
    {
        T tmp = m_current;
        return *tmp;
    }

    constexpr ReferenceType operator[](DifferenceType Diff) const noexcept
    {
        return m_current[Diff];
    }

    constexpr ThisType& operator++() noexcept
    {
        ++m_current;
        return *this;
    }

    constexpr ThisType operator++(int) noexcept
    {
        return ThisType(m_current++);
    }

    constexpr ThisType& operator--() noexcept
    {
        --m_current;
        return *this;
    }

    constexpr ThisType operator--(int) noexcept
    {
        return ThisType(m_current--);
    }

    constexpr ThisType& operator+=(DifferenceType diff) noexcept
    {
        m_current += diff;
        return *this;
    }

    constexpr ThisType& operator-=(DifferenceType diff) noexcept
    {
        m_current -= diff;
        return *this;
    }

    constexpr ThisType operator+(DifferenceType diff) const noexcept
    {
        return ThisType(m_current + diff);
    }

    constexpr ThisType operator-(DifferenceType diff) const noexcept
    {
        return ThisType(m_current - diff);
    }

    const T& Base() const noexcept
    {
        return m_current;
    }

    const T& base() const noexcept
    {
        return Base();
    }

protected:

    T m_current;
};

template <typename TLeft, typename TRight>
constexpr inline bool operator==(const Iterator<TLeft>& left,
                                 const Iterator<TRight>& right) noexcept
{
    return (left.Base() == right.Base());
}

template <typename TLeft, typename TRight>
constexpr inline bool operator!=(const Iterator<TLeft>& left,
                                 const Iterator<TRight>& right) noexcept
{
    return (left.Base() != right.Base());
}

template <typename TLeft, typename TRight>
constexpr inline bool operator<(const Iterator<TLeft>& left,
                                const Iterator<TRight>& right) noexcept
{
    return (left.Base() < right.Base());
}

template <typename TLeft, typename TRight>
constexpr inline bool operator>(const Iterator<TLeft>& left,
                                const Iterator<TRight>& right) noexcept
{
    return (right.Base() > left.Base());
}

template <typename TLeft, typename TRight>
constexpr inline bool operator<=(const Iterator<TLeft>& left,
                                 const Iterator<TRight>& right) noexcept
{
    return (left.Base() <= right.Base());
}

template <typename TLeft, typename TRight>
constexpr inline bool operator>=(const Iterator<TLeft>& left,
                                 const Iterator<TRight>& right) noexcept
{
    return (left.Base() >= right.Base());
}

template <typename T>
constexpr inline typename Iterator<T>::DifferenceType operator-(
    const Iterator<T>& left, const Iterator<T>& right) noexcept
{
    return (left.Base() - right.Base());
}

template <typename T>
constexpr inline typename Iterator<T>::DifferenceType operator+(
    const Iterator<T>& left, const Iterator<T>& right) noexcept
{
    return (left.Base() + right.Base());
}

template <typename T>
class ReverseIterator
{
public:

    using ThisType = ReverseIterator<T>;
    using TraitsType = IteratorTraits<T>;
    using ValueType = typename TraitsType::ValueType;
    using DifferenceType = typename TraitsType::DifferenceType;
    using PointerType = typename TraitsType::PointerType;
    using ReferenceType = typename TraitsType::ReferenceType;

    ~ReverseIterator() = default;

    constexpr ReverseIterator() noexcept = default;

    constexpr explicit ReverseIterator(T other) noexcept
        : m_current(Move(other))
    {
    }

    template <typename U>
    constexpr ReverseIterator(const ReverseIterator<U>& other) noexcept
        : m_current(other.m_current)
    {
    }

    template <typename U>
    constexpr ThisType& operator=(const ReverseIterator<U>& other) noexcept
    {
        m_current = other.m_current;
        return *this;
    }

    template <typename U = T, EnIf<IsPtr<U>, int> = 0>
    constexpr PointerType operator->() const noexcept
    {
        T tmp = m_current;
        return tmp;
    }

    template <typename U = T, EnIf<!IsPtr<U>, int> = 0>
    constexpr PointerType operator->() const noexcept
    {
        T tmp = m_current;
        return tmp.operator->();
    }

    constexpr ReferenceType operator*() const noexcept
    {
        T tmp = m_current;
        return *tmp;
    }

    constexpr ReferenceType operator[](DifferenceType diff) const noexcept
    {
        return m_current[static_cast<DifferenceType>(-diff)];
    }

    constexpr ThisType& operator++() noexcept
    {
        --m_current;
        return *this;
    }

    constexpr ThisType operator++(int) noexcept
    {
        return ThisType(m_current--);
    }

    constexpr ThisType& operator--() noexcept
    {
        ++m_current;
        return *this;
    }

    constexpr ThisType operator--(int) noexcept
    {
        return ThisType(m_current++);
    }

    constexpr ThisType& operator+=(DifferenceType diff) noexcept
    {
        m_current -= diff;
        return *this;
    }

    constexpr ThisType& operator-=(DifferenceType diff) noexcept
    {
        m_current += diff;
        return *this;
    }

    constexpr ThisType operator+(DifferenceType diff) const noexcept
    {
        return ThisType(m_current - diff);
    }

    constexpr ThisType operator-(DifferenceType diff) const noexcept
    {
        return ThisType(m_current + diff);
    }

    const T& Base() const noexcept
    {
        return m_current;
    }

    const T& base() const noexcept
    {
        return Base();
    }

protected:

    T m_current;
};

template <typename TLeft, typename TRight>
constexpr inline bool operator==(const ReverseIterator<TLeft>& left,
                                 const ReverseIterator<TRight>& right) noexcept
{
    return (left.Base() == right.Base());
}

template <typename TLeft, typename TRight>
constexpr inline bool operator!=(const ReverseIterator<TLeft>& left,
                                 const ReverseIterator<TRight>& right) noexcept
{
    return (left.Base() != right.Base());
}

template <typename TLeft, typename TRight>
constexpr inline bool operator<(const ReverseIterator<TLeft>& left,
                                const ReverseIterator<TRight>& right) noexcept
{
    return (left.Base() < right.Base());
}

template <typename TLeft, typename TRight>
constexpr inline bool operator>(const ReverseIterator<TLeft>& left,
                                const ReverseIterator<TRight>& right) noexcept
{
    return (right.Base() > left.Base());
}

template <typename TLeft, typename TRight>
constexpr inline bool operator<=(const ReverseIterator<TLeft>& left,
                                 const ReverseIterator<TRight>& right) noexcept
{
    return (left.Base() <= right.Base());
}

template <typename TLeft, typename TRight>
constexpr inline bool operator>=(const ReverseIterator<TLeft>& left,
                                 const ReverseIterator<TRight>& right) noexcept
{
    return (left.Base() >= right.Base());
}

template <typename T>
constexpr inline typename ReverseIterator<T>::DifferenceType operator-(
    const ReverseIterator<T>& left, const ReverseIterator<T>& right) noexcept
{
    return (left.Base() + right.Base());
}

template <typename T>
constexpr inline typename ReverseIterator<T>::DifferenceType operator+(
    const ReverseIterator<T>& left, const ReverseIterator<T>& right) noexcept
{
    return (left.Base() - right.Base());
}

} // namespace rad
