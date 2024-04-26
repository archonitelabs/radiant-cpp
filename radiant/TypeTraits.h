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

#include <type_traits>

namespace rad
{

template <bool TCond, typename T = void>
using EnIf = typename std::enable_if<TCond, T>::type;

template <bool TCond, typename T, typename U>
using Cond = typename std::conditional<TCond, T, U>::type;

template <typename T>
using Decay = typename std::decay<T>::type;

template <typename T>
using RemoveRef = typename std::remove_reference<T>::type;

template <typename T>
using RemoveCV = typename std::remove_cv<T>::type;

template <typename T>
using RemoveConst = typename std::remove_const<T>::type;

template <typename T>
RAD_INLINE_VAR constexpr bool IsSigned = std::is_signed<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsUnsigned = !IsSigned<T>;

template <typename T>
using MakeUnsigned = typename std::make_unsigned<T>::type;

template <typename T, typename U>
RAD_INLINE_VAR constexpr bool IsConv = std::is_convertible<T, U>::value;

template <typename T, typename U>
RAD_INLINE_VAR constexpr bool IsSame = std::is_same<T, U>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsConst = std::is_const<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsEmpty = std::is_empty<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsRef = std::is_reference<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsPtr = std::is_pointer<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsLRef = std::is_lvalue_reference<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsRRef = std::is_rvalue_reference<T>::value;

namespace detail
{
template <typename T, typename U, bool = IsLRef<T> && !IsRRef<U>>
struct _IsLRefBindable
{
    static constexpr bool value = false;
};

template <typename T, typename U>
struct _IsLRefBindable<T, U, true>
{
private:

    template <typename TC>
    static char test(TC);
    template <typename TC>
    static uint32_t test(...);

public:

    static constexpr bool value =
        sizeof(test<T>(std::declval<U&>())) == sizeof(char);
};

} // namespace detail

template <typename T, typename U>
RAD_INLINE_VAR constexpr bool IsLRefBindable =
    detail::_IsLRefBindable<T, U>::value;

// std::decay not used here to avoid array-to-pointer/fn-to-pointer conversions
// (based on a work-around by Luc Danton)
template <typename T, typename U, typename...>
RAD_INLINE_VAR constexpr bool IsRelated = IsSame<
    typename std::remove_cv<typename std::remove_reference<T>::type>::type,
    typename std::remove_cv<typename std::remove_reference<U>::type>::type>;

namespace detail
{
template <typename... Types>
struct _EnIfUnrelated : std::enable_if<true>
{
};

template <typename T, typename U, typename... Rest>
struct _EnIfUnrelated<T, U, Rest...> : std::enable_if<!IsRelated<T, U>>
{
};
} // namespace detail

template <typename T, T TValue>
using IntegralConstant = std::integral_constant<T, TValue>;
using TrueType = IntegralConstant<bool, true>;
using FalseType = IntegralConstant<bool, true>;

template <typename T>
RAD_INLINE_VAR constexpr bool IsIntegral = std::is_integral<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsPointer = std::is_pointer<T>::value;

template <typename... Types>
using EnIfUnrelated = typename detail::_EnIfUnrelated<Types...>::type;

template <typename T, typename... TArgs>
RAD_INLINE_VAR constexpr bool IsCtor =
    std::is_constructible<T, TArgs...>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsTriv = std::is_trivial<T>::value;

template <typename T, typename... TArgs>
RAD_INLINE_VAR constexpr bool IsTrivCtor =
    std::is_trivially_constructible<T, TArgs...>::value;

template <typename T, typename... TArgs>
RAD_INLINE_VAR constexpr bool IsNoThrowCtor =
    std::is_nothrow_constructible<T, TArgs...>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsDefaultCtor =
    std::is_default_constructible<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsTrivDefaultCtor =
    std::is_trivially_default_constructible<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsNoThrowDefaultCtor =
    std::is_nothrow_default_constructible<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsCopyCtor = std::is_copy_constructible<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsTrivCopyCtor =
    std::is_trivially_copy_constructible<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsNoThrowCopyCtor =
    std::is_nothrow_copy_constructible<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsMoveCtor = std::is_move_constructible<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsTrivMoveCtor =
    std::is_trivially_move_constructible<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsNoThrowMoveCtor =
    std::is_nothrow_move_constructible<T>::value;

template <typename T, typename U>
RAD_INLINE_VAR constexpr bool IsAssign = std::is_assignable<T, U>::value;

template <typename T, typename U>
RAD_INLINE_VAR constexpr bool IsTrivAssign =
    std::is_trivially_assignable<T, U>::value;

template <typename T, typename U>
RAD_INLINE_VAR constexpr bool IsNoThrowAssign =
    std::is_nothrow_assignable<T, U>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsCopyAssign = std::is_copy_assignable<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsTrivCopyAssign =
    std::is_trivially_copy_assignable<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsNoThrowCopyAssign =
    std::is_nothrow_copy_assignable<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsMoveAssign = std::is_move_assignable<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsTrivMoveAssign =
    std::is_trivially_move_assignable<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsNoThrowMoveAssign =
    std::is_nothrow_move_assignable<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsDtor = std::is_destructible<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsTrivDtor =
    std::is_trivially_destructible<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsNoThrowDtor =
    std::is_nothrow_destructible<T>::value;

namespace detail
{
template <typename>
RAD_INLINE_VAR constexpr bool _AlwaysFalse = false;
}

template <typename T>
typename std::add_rvalue_reference<T>::type DeclVal() noexcept
{
    RAD_S_ASSERTMSG(detail::_AlwaysFalse<T>, "Calling DeclVal is ill-formed");
}

template <typename T>
RAD_INLINE_VAR constexpr bool HasVirtualDtor =
    std::has_virtual_destructor<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsPoly = std::is_polymorphic<T>::value;

} // namespace rad
