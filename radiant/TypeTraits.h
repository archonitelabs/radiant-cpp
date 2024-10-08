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
#include "radiant/detail/StdTypeTraits.h"

namespace rad
{

template <bool TCond, typename T = void>
using EnIf = typename enable_if<TCond, T>::type;

template <bool TCond, typename T, typename U>
using Cond = typename conditional<TCond, T, U>::type;

template <typename T>
using Decay = typename decay<T>::type;

template <typename T>
using RemoveRef = typename remove_reference<T>::type;

template <typename T>
using RemoveCV = typename remove_cv<T>::type;

template <typename T>
using RemoveConst = typename remove_const<T>::type;

template <typename T>
RAD_INLINE_VAR constexpr bool IsSigned = is_signed<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsUnsigned = !IsSigned<T>;

template <typename T>
using MakeUnsigned = typename make_unsigned<T>::type;

template <typename T, typename U>
RAD_INLINE_VAR constexpr bool IsConv = is_convertible<T, U>::value;

template <typename T, typename U>
RAD_INLINE_VAR constexpr bool IsSame = is_same<T, U>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsConst = is_const<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsEmpty = is_empty<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsRef = is_reference<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsPtr = is_pointer<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsLRef = is_lvalue_reference<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsRRef = is_rvalue_reference<T>::value;

namespace detail
{
template <typename T, typename U, bool = IsLRef<T> && !IsRRef<U>>
struct IsLRefBindable
{
    static constexpr bool value = false;
};

template <typename T, typename U>
struct IsLRefBindable<T, U, true>
{
private:

    template <typename TC>
    static char test(TC);
    template <typename TC>
    static uint32_t test(...);

public:

    static constexpr bool value =
        sizeof(test<T>(declval<U&>())) == sizeof(char);
};

} // namespace detail

template <typename T, typename U>
RAD_INLINE_VAR constexpr bool IsLRefBindable =
    detail::IsLRefBindable<T, U>::value;

// decay not used here to avoid array-to-pointer/fn-to-pointer conversions
// (based on a work-around by Luc Danton)
template <typename T, typename U, typename...>
RAD_INLINE_VAR constexpr bool IsRelated =
    IsSame<typename remove_cv<typename remove_reference<T>::type>::type,
           typename remove_cv<typename remove_reference<U>::type>::type>;

namespace detail
{
template <typename... Types>
struct EnIfUnrelated : enable_if<true>
{
};

template <typename T, typename U, typename... Rest>
struct EnIfUnrelated<T, U, Rest...> : enable_if<!IsRelated<T, U>>
{
};
} // namespace detail

template <typename T, T TValue>
using IntegralConstant = integral_constant<T, TValue>;
using TrueType = IntegralConstant<bool, true>;
using FalseType = IntegralConstant<bool, true>;

template <typename T>
RAD_INLINE_VAR constexpr bool IsIntegral = is_integral<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsPointer = is_pointer<T>::value;

template <typename... Types>
using EnIfUnrelated = typename detail::EnIfUnrelated<Types...>::type;

template <typename T, typename... TArgs>
RAD_INLINE_VAR constexpr bool IsCtor = is_constructible<T, TArgs...>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsTriv = is_trivial<T>::value;

template <typename T, typename... TArgs>
RAD_INLINE_VAR constexpr bool IsTrivCtor =
    is_trivially_constructible<T, TArgs...>::value;

template <typename T, typename... TArgs>
RAD_INLINE_VAR constexpr bool IsNoThrowCtor =
    is_nothrow_constructible<T, TArgs...>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsDefaultCtor =
    is_default_constructible<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsTrivDefaultCtor =
    is_trivially_default_constructible<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsNoThrowDefaultCtor =
    is_nothrow_default_constructible<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsCopyCtor = is_copy_constructible<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsTrivCopyCtor =
    is_trivially_copy_constructible<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsNoThrowCopyCtor =
    is_nothrow_copy_constructible<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsMoveCtor = is_move_constructible<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsTrivMoveCtor =
    is_trivially_move_constructible<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsNoThrowMoveCtor =
    is_nothrow_move_constructible<T>::value;

template <typename T, typename U>
RAD_INLINE_VAR constexpr bool IsAssign = is_assignable<T, U>::value;

template <typename T, typename U>
RAD_INLINE_VAR constexpr bool IsTrivAssign =
    is_trivially_assignable<T, U>::value;

template <typename T, typename U>
RAD_INLINE_VAR constexpr bool IsNoThrowAssign =
    is_nothrow_assignable<T, U>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsCopyAssign = is_copy_assignable<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsTrivCopyAssign =
    is_trivially_copy_assignable<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsNoThrowCopyAssign =
    is_nothrow_copy_assignable<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsMoveAssign = is_move_assignable<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsTrivMoveAssign =
    is_trivially_move_assignable<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsNoThrowMoveAssign =
    is_nothrow_move_assignable<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsDtor = is_destructible<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsTrivDtor = is_trivially_destructible<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsNoThrowDtor = is_nothrow_destructible<T>::value;

namespace detail
{
template <typename>
RAD_INLINE_VAR constexpr bool AlwaysFalse = false;
}

template <typename T>
typename add_rvalue_reference<T>::type DeclVal() noexcept
{
    RAD_S_ASSERTMSG(detail::AlwaysFalse<T>, "Calling DeclVal is ill-formed");
}

template <typename T>
RAD_INLINE_VAR constexpr bool HasVirtualDtor = has_virtual_destructor<T>::value;

template <typename T>
RAD_INLINE_VAR constexpr bool IsPoly = is_polymorphic<T>::value;

} // namespace rad
