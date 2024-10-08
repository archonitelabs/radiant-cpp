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

/// \brief Pair implementing the "empty base optimization" technique.
/// \details With this technique, it is possible to create zero-sized storage
/// for stateless objects when paired with another type. A common use-case is
/// allocators. Sometimes allocators are stateless and don't require storage,
/// and sometimes they are stateful. With this technique, stateless allocators
/// require no storage.
/// \tparam TEmptyOptimized Type that can optionally be empty (stateless)
/// \tparam T2 Type which will always use storage
template <typename TEmptyOptimized,
          typename T2,
          bool = IsEmpty<TEmptyOptimized>>
class EmptyOptimizedPair final : private TEmptyOptimized
{
public:

    using FirstType = TEmptyOptimized;
    using SecondType = T2;

    constexpr EmptyOptimizedPair() noexcept(IsNoThrowDefaultCtor<FirstType> &&
                                            IsNoThrowDefaultCtor<SecondType>)
        : FirstType(),
          m_second()
    {
    }

    template <typename... Args>
    constexpr explicit EmptyOptimizedPair(Args&&... args) noexcept(
        IsNoThrowDefaultCtor<FirstType> && IsNoThrowCtor<SecondType, Args...>)
        : FirstType(),
          m_second(Forward<Args>(args)...)
    {
    }

    template <typename... Args>
    constexpr explicit EmptyOptimizedPair(
        FirstType& first,
        Args&&... args) noexcept(noexcept(FirstType(first)) &&
                                 IsNoThrowCtor<SecondType, Args...>)
        : FirstType(first),
          m_second(Forward<Args>(args)...)
    {
    }

    template <typename... Args>
    constexpr explicit EmptyOptimizedPair(
        const FirstType& first,
        Args&&... args) noexcept(noexcept(FirstType(first)) &&
                                 IsNoThrowCtor<SecondType, Args...>)
        : FirstType(first),
          m_second(Forward<Args>(args)...)
    {
    }

    template <typename... Args>
    constexpr explicit EmptyOptimizedPair(
        FirstType&& first,
        Args&&... args) noexcept(noexcept(FirstType(Move(first))) &&
                                 IsNoThrowCtor<SecondType, Args...>)
        : FirstType(Move(first)),
          m_second(Forward<Args>(args)...)
    {
    }

    constexpr explicit EmptyOptimizedPair(EmptyOptimizedPair& r) noexcept(
        IsNoThrowCopyCtor<FirstType> && IsNoThrowCopyCtor<SecondType>)
        : FirstType(r.First()),
          m_second(r.m_second)
    {
    }

    constexpr explicit EmptyOptimizedPair(EmptyOptimizedPair const& r) noexcept(
        IsNoThrowCopyCtor<FirstType> && IsNoThrowCopyCtor<SecondType>)
        : FirstType(r.First()),
          m_second(r.m_second)
    {
    }

    constexpr explicit EmptyOptimizedPair(EmptyOptimizedPair&& r) noexcept(
        IsNoThrowMoveCtor<FirstType> && IsNoThrowMoveCtor<SecondType>)
        : FirstType(Move(r)),
          m_second(Move(r.m_second))
    {
    }

    constexpr FirstType& First() noexcept
    {
        return *this;
    }

    constexpr const FirstType& First() const noexcept
    {
        return *this;
    }

    constexpr SecondType& Second() noexcept
    {
        return m_second;
    }

    constexpr const SecondType& Second() const noexcept
    {
        return m_second;
    }

private:

    SecondType m_second;
};

template <typename TEmptyOptimized, typename T2>
class EmptyOptimizedPair<TEmptyOptimized, T2, false> final
{
public:

    using FirstType = TEmptyOptimized;
    using SecondType = T2;

    constexpr EmptyOptimizedPair() noexcept(IsNoThrowDefaultCtor<FirstType> &&
                                            IsNoThrowDefaultCtor<SecondType>)
        : m_first(),
          m_second()
    {
    }

    template <typename... Args>
    constexpr explicit EmptyOptimizedPair(
        const TEmptyOptimized& first,
        Args&&... args) noexcept(IsNoThrowCtor<FirstType, const FirstType&> &&
                                 IsNoThrowCtor<SecondType, Args...>)
        : m_first(first),
          m_second(Forward<Args>(args)...)
    {
    }

    template <typename... Args>
    constexpr explicit EmptyOptimizedPair(
        TEmptyOptimized&& first,
        Args&&... args) noexcept(IsNoThrowCtor<FirstType, FirstType&&> &&
                                 IsNoThrowCtor<SecondType, Args...>)
        : m_first(Forward<FirstType>(first)),
          m_second(Forward<Args>(args)...)
    {
    }

    constexpr EmptyOptimizedPair(EmptyOptimizedPair& r) noexcept(
        IsNoThrowCopyCtor<FirstType> && IsNoThrowCopyCtor<SecondType>)
        : m_first(r.m_first),
          m_second(r.m_second)
    {
    }

    constexpr EmptyOptimizedPair(EmptyOptimizedPair const& r) noexcept(
        IsNoThrowCopyCtor<FirstType> && IsNoThrowCopyCtor<SecondType>)
        : m_first(r.m_first),
          m_second(r.m_second)
    {
    }

    constexpr EmptyOptimizedPair(EmptyOptimizedPair&& r) noexcept(
        IsNoThrowMoveCtor<FirstType> && IsNoThrowMoveCtor<SecondType>)
        : m_first(Move(r.m_first)),
          m_second(Move(r.m_second))
    {
    }

    constexpr FirstType& First() noexcept
    {
        return m_first;
    }

    constexpr const FirstType& First() const noexcept
    {
        return m_first;
    }

    constexpr SecondType& Second() noexcept
    {
        return m_second;
    }

    constexpr const SecondType& Second() const noexcept
    {
        return m_second;
    }

private:

    FirstType m_first;
    SecondType m_second;
};

} // namespace rad
