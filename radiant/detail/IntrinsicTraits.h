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

#ifdef RAD_MSC_VERSION
// MSVC does not seem to have something like __has_builtin, but
// also seems to define a superset of what the other compilers do
// in terms of intrinsics
#define __has_builtin(v) 1
#endif

#if __has_builtin(__is_enum)
#define INTRINSIC_IS_ENUM(T) __is_enum(T)
#else
RAD_S_ASSERTMSG(false, "compiler does not support intrinsic __is_enum")
#endif

#if __has_builtin(__is_base_of)
#define INTRINSIC_IS_BASE_OF(B, D) __is_base_of(B, D)
#else
RAD_S_ASSERTMSG(false, "compiler does not support intrinsic __is_base_of")
#endif

#if __has_builtin(__is_empty)
#define INTRINSIC_IS_EMPTY(T) __is_empty(T)
#else
RAD_S_ASSERTMSG(false, "compiler does not support intrinsic __is_empty")
#endif

#if __has_builtin(__is_polymorphic)
#define INTRINSIC_IS_POLYMORPHIC(T) __is_polymorphic(T)
#else
RAD_S_ASSERTMSG(false, "compiler does not support intrinsic __is_polymorphic")
#endif

#if __has_builtin(__is_trivially_destructible)
#define INTRINSIC_IS_TRIVIALLY_DESTRUCTIBLE(T) __is_trivially_destructible(T)
#elif __has_builtin(__has_trivial_destructor)
#define INTRINSIC_IS_TRIVIALLY_DESTRUCTIBLE(T) __has_trivial_destructor(T)
#else
RAD_S_ASSERTMSG(false,
                "compiler does not support intrinsic for either "
                "__is_trivially_destructible or __has_trivial_destructor");
#endif

#if __has_builtin(__is_assignable)
#define INTRINSIC_IS_ASSIGNABLE(L, R) __is_assignable(L, R)
#else
RAD_S_ASSERTMSG(false, "compiler does not support intrinsic __is_assignable");
#endif

#if __has_builtin(__is_constructible)
#define INTRINSIC_IS_CONSTRUCTIBLE(...) __is_constructible(__VA_ARGS__)
#else
RAD_S_ASSERTMSG(false,
                "compiler does not support intrinsic __is_constructible");
#endif

#if __has_builtin(__is_trivially_assignable)
#define INTRINSIC_IS_TRIVIALLY_ASSIGNABLE(L, R) __is_trivially_assignable(L, R)
#else
RAD_S_ASSERTMSG(
    false, "compiler does not support intrinsic __is_trivially_assignable");
#endif

#if __has_builtin(__is_trivially_constructible)
#define INTRINSIC_IS_TRIVIALLY_CONSTRUCTIBLE(...)                              \
    __is_trivially_constructible(__VA_ARGS__)
#else
RAD_S_ASSERTMSG(
    false, "compiler does not support intrinsic __is_trivially_constructible");
#endif

#if __has_builtin(__is_nothrow_assignable) || defined(RAD_GCC_VERSION)
#define INTRINSIC_IS_NOTHROW_ASSIGNABLE(L, R) __is_nothrow_assignable(L, R)
#else
RAD_S_ASSERTMSG(false,
                "compiler does not support intrinsic __is_nothrow_assignable");
#endif

#if __has_builtin(__is_nothrow_constructible) || defined(RAD_GCC_VERSION)
#define INTRINSIC_IS_NOTHROW_CONSTRUCTIBLE(...)                                \
    __is_nothrow_constructible(__VA_ARGS__)
#else
RAD_S_ASSERTMSG(
    false, "compiler does not support intrinsic __is_nothrow_constructible");
#endif

#if __has_builtin(__is_trivial)
#define IS_TRIVIAL_IMPL(T) __is_trivial(T)
#else
#define IS_TRIVIAL_IMPL(T)                                                     \
    (is_trivially_copyable<T>::value &&                                        \
     is_trivially_default_constructible<T>::value)
#endif

#if __has_builtin(__has_virtual_destructor)
#define INTRINSIC_HAS_VIRTUAL_DESTRUCTOR(T) __has_virtual_destructor(T)
#else
RAD_S_ASSERTMSG(false,
                "compiler does not support intrinsic __has_virtual_destructor");
#endif
