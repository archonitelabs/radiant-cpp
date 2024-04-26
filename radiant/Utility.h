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

#include "radiant/TypeTraits.h"

namespace rad
{

template <typename T>
RAD_NODISCARD constexpr inline T&& Forward(RemoveRef<T>& v) noexcept
{
    return static_cast<T&&>(v);
}

template <typename T>
RAD_NODISCARD constexpr inline T&& Forward(RemoveRef<T>&& v) noexcept
{
    RAD_S_ASSERTMSG(!IsLRef<T>, "cannot forward an rvalue as an lvalue.");

    return static_cast<T&&>(v);
}

template <typename T>
RAD_NODISCARD constexpr inline RemoveRef<T>&& Move(T&& v) noexcept
{
    return static_cast<RemoveRef<T>&&>(v);
}

template <typename T>
RAD_NODISCARD constexpr inline //
    Cond<(IsNoThrowMoveCtor<T> && IsNoThrowCopyCtor<T>), const T&, T&&>
    MoveIfNoExcept(T& v) noexcept
{
    return Move(v);
}

template <typename T, uint32_t N>
RAD_NODISCARD constexpr inline uint32_t Size(T (&)[N]) noexcept
{
    return N;
}

template <typename T>
RAD_NODISCARD constexpr inline auto Size(const T& c) noexcept(
    noexcept(c.Size())) -> decltype(c.Size())
{
    RAD_S_ASSERT_NOTHROW(noexcept(c.Size()));

    return c.Size();
}

template <typename T = void>
RAD_NODISCARD constexpr inline T* Add2Ptr(void* p, uint32_t offset) noexcept
{
    return reinterpret_cast<T*>(static_cast<char*>(p) + offset);
}

#if defined(RAD_MSC_VERSION) || defined(RAD_CLANG_VERSION) ||                  \
    defined(RAD_GCC_VERSION)
template <typename T>
RAD_NODISCARD constexpr inline T* AddrOf(T& v) noexcept
{
    return __builtin_addressof(v);
}
#else
template <typename T>
RAD_NODISCARD constexpr inline T* AddrOf(T& v) noexcept
{
    return reinterpret_cast<T*>(
        &const_cast<char&>(reinterpret_cast<const volatile char&>(v)));
}
#endif

template <typename T>
constexpr const T* AddrOf(const T&& v) noexcept = delete;

template <typename T>
RAD_NODISCARD constexpr inline const T& Min(const T& a, const T& b) //
    noexcept(noexcept(a < b))
{
    RAD_S_ASSERT_NOTHROW(noexcept(a < b));

    return (a < b) ? a : b;
}

template <typename T>
RAD_NODISCARD constexpr inline const T& Max(const T& a, const T& b) //
    noexcept(noexcept(a > b))
{
    RAD_S_ASSERT_NOTHROW(noexcept(a > b));

    return (a > b) ? a : b;
}

} // namespace rad
