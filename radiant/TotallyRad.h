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

#ifdef __cplusplus

#if __cplusplus >= 202002L // c++20
#define RAD_CPP   20
#define RAD_CPP20 1
#define RAD_CPP17 1
#define RAD_CPP14 1
#define RAD_CPP11 1
#elif __cplusplus >= 201703L // c++17
#define RAD_CPP   17
#define RAD_CPP20 0
#define RAD_CPP17 1
#define RAD_CPP14 1
#define RAD_CPP11 1
#elif __cplusplus >= 201402L // c++14
#define RAD_CPP   14
#define RAD_CPP20 0
#define RAD_CPP17 0
#define RAD_CPP14 1
#define RAD_CPP11 1
#elif __cplusplus >= 201103L // c++11
#define RAD_CPP   11
#define RAD_CPP20 0
#define RAD_CPP17 0
#define RAD_CPP14 0
#define RAD_CPP11 1
#else
#define RAD_CPP   0
#define RAD_CPP20 0
#define RAD_CPP17 0
#define RAD_CPP14 0
#define RAD_CPP11 0
#endif

#if RAD_CPP20
#define RAD_LIKELY(x)   (x) [[likely]]
#define RAD_UNLIKELY(x) (x) [[unlikely]]
#define RAD_EXPLICIT(x) explicit(x)
#else
#define RAD_LIKELY(x)   (x)
#define RAD_UNLIKELY(x) (x)
#define RAD_EXPLICIT(x)
#endif

#if RAD_CPP17
#define RAD_NODISCARD   [[nodiscard]]
#define RAD_FALLTHROUGH [[fallthrough]]
#define RAD_INLINE_VAR  inline
#else
#define RAD_NODISCARD
#define RAD_FALLTHROUGH
#define RAD_INLINE_VAR
#endif

#if _MSC_VER >= 1929 // VS2019 v16.10 and later
// C4848: msvc::no_unique_address in C++17 and earlier is a vendor extension
#define RAD_NO_UNIQUE_ADDRESS                                                  \
    _Pragma("warning(suppress : 4848)") [[msvc::no_unique_address]]
#else
// gcc 9 and clang 9 support [[no_unique_address]] in c++11 modes and newer
#define RAD_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

#endif // __cplusplus

#if defined(__clang__) && __clang__
#define RAD_CLANG_VERSION                                                      \
    (__clang_major__ * 10000 + __clang_minor__ + 100 + __clang_patchlevel__)
#endif

#if !defined(__clang__) && defined(__GNUC__) && __GNUC__
#define RAD_GCC_VERSION                                                        \
    (__GNUC__ * 10000 + __GNUC__MINOR__ + 100 + __GNUC_PATCHLEVEL__)
#endif

#if defined(_MSC_VER) && _MSC_VER
#define RAD_MSC_VERSION _MSC_VER
#endif

#if defined(_MSVC_VER) && _MSVC_VER
#define RAD_MSVC_VERSION _MSVC_VER
#endif

#if !defined(NDEBUG) || defined(_DEBUG)
#define RAD_DBG 1
#else
#define RAD_DBG 0
#endif

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#define RAD_WINDOWS 1
#define RAD_LINUX   0
#define RAD_MACOS   0
#endif

#if defined(unix) || defined(__unix) || defined(__unix__)
#define RAD_WINDOWS 0
#define RAD_LINUX   1
#define RAD_MACOS   0
#endif

#if defined(__APPLE__) || defined(__MACH__)
#define RAD_WINDOWS 0
#define RAD_LINUX   0
#define RAD_MACOS   1
#endif

#if defined(__KERNEL_MODE)
#define RAD_KERNEL_MODE 1
#define RAD_USER_MODE   0
#else
#define RAD_KERNEL_MODE 0
#define RAD_USER_MODE   1
#endif

static_assert(RAD_KERNEL_MODE || RAD_USER_MODE, "env undefined mode");
static_assert(!(RAD_KERNEL_MODE && RAD_USER_MODE), "env invalid mode");
static_assert(RAD_WINDOWS || RAD_LINUX || RAD_MACOS, "env undefined os");
static_assert(!(RAD_WINDOWS && RAD_LINUX && RAD_MACOS), "env invalid os");
static_assert(!(RAD_WINDOWS && RAD_LINUX), "env invalid os");
static_assert(!(RAD_LINUX && RAD_MACOS), "env invalid os");
static_assert(!(RAD_WINDOWS && RAD_MACOS), "env invalid os");

#if defined(_M_AMD64) || defined(__amd64__)
#define RAD_AMD64 1
#define RAD_I386  0
#define RAD_ARM64 0
#define RAD_ARM   0
#elif defined(_M_IX86) || defined(__i386__)
#define RAD_AMD64 0
#define RAD_I386  1
#define RAD_ARM64 0
#define RAD_ARM   0
#elif defined(_M_ARM64) || defined(__aarch64__)
#define RAD_AMD64 0
#define RAD_I386  0
#define RAD_ARM64 1
#define RAD_ARM   0
#elif defined(_M_ARM) || defined(__arm__)
#define RAD_AMD64 0
#define RAD_I386  0
#define RAD_ARM64 0
#define RAD_ARM   1
#else
#error unsupported hardware
#endif

#define RAD_UNUSED(x) ((void)x)

#ifdef RAD_MSC_VERSION
// MSVC does not seem to have something like __has_builtin, but
// also seems to define a superset of what the other compilers do
// in terms of intrinsics
#define RAD_HAS_BUILTIN(v) 1
#else
#define RAD_HAS_BUILTIN(v) __has_builtin(v)
#endif

#if RAD_WINDOWS
#define RAD_YIELD_PROCESSOR() YieldProcessor()
#else
#define RAD_YIELD_PROCESSOR() sched_yield()
#endif

#define RAD_CONCAT_INNER(x, y) x##y
#define RAD_CONCAT(x, y)       RAD_CONCAT_INNER(x, y)

#if RAD_USER_MODE && RAD_DBG
#include <assert.h>

namespace rad
{

#if RAD_WINDOWS
extern "C" void __cdecl _assert(const char* Assertion,
                                const char* File,
                                unsigned Line);
#endif

inline bool DoAssert(const char* Assertion, const char* File, int Line)
{
#if RAD_WINDOWS
    _assert(Assertion, File, Line);
#else
    __assert(Assertion, File, Line);
#endif
    return false;
}

} // namespace rad

#endif

#if RAD_DBG

#if RAD_WINDOWS && RAD_KERNEL_MODE
#define RAD_ASSERT(x)         NT_ASSERT(x)
#define RAD_ASSERTMSG(x, msg) NT_ASSERTMSG(msg, x)
#define RAD_VERIFY(x)         NT_VERIFY(x)
#define RAD_VERIFYMSG(x, msg) NT_VERIFYMSG(x)
#else
#define RAD_VERIFY(x) ((!!(x)) || (::rad::DoAssert(#x, __FILE__, __LINE__)))
#define RAD_VERIFYMSG(x, m)                                                    \
    ((!!(x)) || (::rad::DoAssert(m, __FILE__, __LINE__)))
#define RAD_ASSERT(x)       (void)RAD_VERIFY(x)
#define RAD_ASSERTMSG(x, m) (void)RAD_VERIFY(x, m)
#endif

#else

#define RAD_ASSERT(x)       ((void)0)
#define RAD_ASSERTMSG(x, m) ((void)0)
#define RAD_VERIFY(x)       (x)
#define RAD_VERIFYMSG(x, m) (x)

#endif

#define RAD_S_ASSERT(x)       static_assert(x, #x)
#define RAD_S_ASSERTMSG(x, m) static_assert(x, m)

//
// Enables broad assertions that objects do not throw exceptions.
//
#ifndef RAD_ENABLE_NOTHROW_ASSERTIONS
#define RAD_ENABLE_NOTHROW_ASSERTIONS 1
#endif
#if RAD_ENABLE_NOTHROW_ASSERTIONS
#define RAD_S_ASSERT_NOTHROW(x)       RAD_S_ASSERT(x)
#define RAD_S_ASSERT_NOTHROWMSG(x, m) RAD_S_ASSERTMSG(x, m)
#else
#define RAD_S_ASSERT_NOTHROW(x)       RAD_S_ASSERT(true)
#define RAD_S_ASSERT_NOTHROWMSG(x, m) RAD_S_ASSERT(true)
#endif

//
// Enables requiring explicit atomic ordering.
//
#ifndef RAD_REQUIRE_EXPLICIT_ATOMIC_ORDERING
#define RAD_REQUIRE_EXPLICIT_ATOMIC_ORDERING 1
#endif

//
// Enables assertions that destructors do not throw exceptions.
//
// Core Guideline: A destructor must not fail. If a destructor tries to exit
// with an exception, it’s a bad design error and the program had better
// terminate.
//
#ifndef RAD_ENABLE_NOTHROW_DTOR_ASSERTIONS
#define RAD_ENABLE_NOTHROW_DTOR_ASSERTIONS 1
#endif
#if RAD_ENABLE_NOTHROW_DTOR_ASSERTIONS
#define RAD_S_ASSERT_NOTHROW_DTOR(x)                                           \
    RAD_S_ASSERTMSG(x, "destructors should not throw")
#define RAD_S_ASSERT_NOTHROW_DTOR_T(x)                                         \
    RAD_S_ASSERT_NOTHROW_DTOR(::rad::IsNoThrowDtor<x>)
#else
#define RAD_S_ASSERT_NOTHROW_DTOR(x)   RAD_S_ASSERT(true)
#define RAD_S_ASSERT_NOTHROW_DTOR_T(x) RAD_S_ASSERT(true)
#endif

//
// Enables assertions that move operations do not throw exceptions.
//
// Core Guideline: A throwing move violates most people’s reasonable
// assumptions. A non-throwing move will be used more efficiently by
// standard-library and language facilities.
//
#ifndef RAD_ENABLE_NOTHROW_MOVE_ASSERTIONS
#define RAD_ENABLE_NOTHROW_MOVE_ASSERTIONS 1
#endif
#if RAD_ENABLE_NOTHROW_MOVE_ASSERTIONS
#define RAD_S_ASSERT_NOTHROW_MOVE(x)                                           \
    RAD_S_ASSERTMSG(x, "move operations should not throw")
#define RAD_S_ASSERT_NOTHROW_MOVE_T(x)                                         \
    RAD_S_ASSERT_NOTHROW_MOVE(                                                 \
        ::rad::IsNoThrowMoveCtor<x>&& ::rad::IsNoThrowMoveAssign<x>)
#else
#define RAD_S_ASSERT_NOTHROW_MOVE(x)   RAD_S_ASSERT(true)
#define RAD_S_ASSERT_NOTHROW_MOVE_T(x) RAD_S_ASSERT(true)
#endif

//
// Enables assertions that allocators meet the Radiant allocator concept
// requirements.
//
// See: rad::AllocatorRequires
//
#ifndef RAD_ENABLE_ALLOCATOR_REQUIRES_ASSERTIONS
#define RAD_ENABLE_ALLOCATOR_REQUIRES_ASSERTIONS 1
#endif
#if RAD_ENABLE_ALLOCATOR_REQUIRES_ASSERTIONS
#define RAD_S_ASSERT_ALLOCATOR_REQUIRES(x)                                     \
    RAD_S_ASSERTMSG(x, "allocator requirements not met")
#define RAD_S_ASSERT_ALLOCATOR_REQUIRES_T(x)                                   \
    RAD_S_ASSERT_ALLOCATOR_REQUIRES(::rad::AllocatorRequires<x>)
#else
#define RAD_S_ASSERT_ALLOCATOR_REQUIRES(x)   RAD_S_ASSERT(true)
#define RAD_S_ASSERT_ALLOCATOR_REQUIRES_T(x) RAD_S_ASSERT(true)
#endif

#define RAD_NOT_COPYABLE(x)                                                    \
    x(x const&) = delete;                                                      \
    x& operator=(x const&) = delete

#ifdef RAD_NO_STD
#define RAD_ENABLE_STD 0
#define RAD_NO_STD     1
#else
#define RAD_ENABLE_STD 1
#define RAD_NO_STD     0
#endif

namespace rad
{
using nullptr_t = decltype(nullptr);
} // namespace rad
