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

#include "radiant/Utility.h"

namespace rad
{

/// @brief General purpose scope guard which invokes a callable target when
/// scope is exited as long as it hasn't been released.
template <typename T>
class ScopeExit
{
public:

    RAD_NOT_COPYABLE(ScopeExit);

    ~ScopeExit()
    {
        RAD_S_ASSERTMSG(IsNoThrowDtor<T>, "Destructors should not throw!");
        RAD_S_ASSERTMSG(noexcept(m_fn()), "Destructors should not throw!");

        if (m_call)
        {
            m_fn();
        }
    }

    template <typename Fn>
    explicit ScopeExit(Fn&& fn) noexcept(IsNoThrowCtor<T, Fn&&>)
        : m_fn(Forward<Fn>(fn))
    {
        RAD_S_ASSERT_NOTHROW((IsNoThrowCtor<T, Fn&&>));
    }

    ScopeExit(ScopeExit<T>&& other) noexcept(IsNoThrowMoveCtor<T>)
        : m_fn(Move(other.m_fn))
    {
        RAD_S_ASSERT_NOTHROW(IsNoThrowMoveCtor<T>);
    }

    void Release() noexcept
    {
        m_call = false;
    }

private:

    bool m_call{ true };
    const T m_fn;
};

/// @brief General purpose scope guard which invokes a callable target when
/// scope is exited.
/// @details A scope guard is irrevocable, in contrast to a scope exit which may
/// be revoked by releasing it.
template <typename T>
class ScopeGuard
{
public:

    RAD_NOT_COPYABLE(ScopeGuard);

    ~ScopeGuard()
    {
        RAD_S_ASSERTMSG(IsNoThrowDtor<T>, "Destructors should not throw!");
        RAD_S_ASSERTMSG(noexcept(m_fn()), "Destructors should not throw!");

        m_fn();
    }

    template <typename Fn>
    explicit ScopeGuard(Fn&& fn) noexcept(IsNoThrowCtor<T, Fn&&>)
        : m_fn(Forward<Fn>(fn))
    {
        RAD_S_ASSERT_NOTHROW((IsNoThrowCtor<T, Fn&&>));
    }

    ScopeGuard(ScopeGuard<T>&& other) noexcept(IsNoThrowMoveCtor<T>)
        : m_fn(Move(other.m_fn))
    {
        RAD_S_ASSERT_NOTHROW(IsNoThrowMoveCtor<T>);
    }

private:

    const T m_fn;
};

/// @brief Makes a ScopeExit object.
/// @tparam Fn Callable type to invoke at scope exit.
/// @param fn Callback target to store to be invoked at scope exit.
/// @return ScopeExit object.
template <typename Fn>
ScopeExit<Fn> MakeScopeExit(Fn&& fn)
{
    return ScopeExit<Fn>(Forward<Fn>(fn));
}

/// @brief Makes a ScopeGuard object.
/// @tparam Fn Callable type to invoke at scope exit.
/// @param fn Callback target to store to be invoked at scope exit.
/// @return ScopeGuard object.
template <typename Fn>
ScopeGuard<Fn> MakeScopeGuard(Fn&& fn)
{
    return ScopeGuard<Fn>(Forward<Fn>(fn));
}

} // namespace rad

#define RAD_SCOPE_GUARD_NAME_STRINGIFY(suffix) __scopeGuard##suffix
#define RAD_SCOPE_GUARD_NAME(suffix)           RAD_SCOPE_GUARD_NAME_STRINGIFY(suffix)
#define RAD_SCOPE_GUARD(fn)                                                    \
    const auto RAD_SCOPE_GUARD_NAME(__LINE__) = rad::MakeScopeGuard(fn)
