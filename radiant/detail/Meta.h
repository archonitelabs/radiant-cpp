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

#pragma once

#include <cstddef>

namespace rad
{

namespace meta
{

template <typename T, T val>
struct integral_constant
{
    static constexpr T value = val;
    using value_type = T;
    using type = integral_constant;

    constexpr operator value_type() const noexcept
    {
        return value;
    }

    constexpr value_type operator()() const noexcept
    {
        return value;
    }
};

using true_type = integral_constant<bool, true>;
using false_type = integral_constant<bool, false>;

template <typename...>
using VoidT = void;

template <typename L, typename R>
struct is_same : false_type
{
};

template <typename T>
struct is_same<T, T> : true_type
{
};

template <bool Cond, typename T, typename F>
struct conditional
{
    using type = T;
};

template <typename T, typename F>
struct conditional<false, T, F>
{
    using type = F;
};

template <typename T>
struct Identity
{
    using type = T;
};

template <typename...>
struct Or;

template <>
struct Or<> : public false_type
{
};

template <>
struct Or<true_type> : public true_type
{
};

template <>
struct Or<false_type> : public false_type
{
};

template <typename First, typename... Rest>
struct Or<First, Rest...>
    : public conditional<First::value, First, Or<Rest...>>::type
{
};

template <typename...>
struct And;

template <>
struct And<> : public true_type
{
};

template <>
struct And<true_type> : public true_type
{
};

template <>
struct And<false_type> : public false_type
{
};

template <typename First, typename... Rest>
struct And<First, Rest...>
    : public conditional<First::value, And<Rest...>, First>::type
{
};

template <typename T>
struct Not;

template <>
struct Not<true_type> : public false_type
{
};

template <>
struct Not<false_type> : public true_type
{
};

template <typename... Ts>
struct TypeList
{
    static constexpr size_t size() noexcept
    {
        return sizeof...(Ts);
    }
};

template <typename Ts>
struct Front;

template <typename Head, typename... Ts>
struct Front<TypeList<Head, Ts...>>
{
    using Type = Head;
};

template <typename List>
struct PopFront;

template <typename Head, typename... Ts>
struct PopFront<TypeList<Head, Ts...>>
{
    using Type = TypeList<Ts...>;
};

template <typename Ts, typename Item>
struct PushFront;

template <typename... Ts, typename Item>
struct PushFront<TypeList<Ts...>, Item>
{
    using Type = TypeList<Item, Ts...>;
};

template <size_t N, typename Ts>
struct GetAt;

template <size_t N, typename Head, typename... Ts>
struct GetAt<N, TypeList<Head, Ts...>>
{
    using Type = typename GetAt<N - 1, TypeList<Ts...>>::Type;
};

template <typename Head, typename... Ts>
struct GetAt<0, TypeList<Head, Ts...>>
{
    using Type = Head;
};

template <size_t N>
struct GetAt<N, TypeList<>>
{
    RAD_S_ASSERTMSG(N < TypeList<>::size(), "Index out of range");
};

template <typename Item, typename Ts>
struct Contains;

template <typename Item, typename Head, typename... Ts>
struct Contains<Item, TypeList<Head, Ts...>>
{
    using Type = typename conditional<
        is_same<Item, Head>::value,
        true_type,
        typename Contains<Item, TypeList<Ts...>>::Type>::type;
    static constexpr bool value = Type::value;
};

template <typename Item>
struct Contains<Item, TypeList<>>
{
    using Type = false_type;
    static constexpr bool value = Type::value;
};

using Types = TypeList<short, int, int, long>;
using FrontMissing = TypeList<int, int, long>;

} // namespace meta
} // namespace rad
