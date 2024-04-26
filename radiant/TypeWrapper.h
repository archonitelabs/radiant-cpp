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

#if RAD_ENABLE_STD
#include <initializer_list>
#endif

namespace rad
{

template <typename T, bool = IsRef<T>>
class TypeWrapper;

template <typename T>
class TypeWrapper<T, false>
{
public:

    using Type = T;

    template <typename U = T, EnIf<IsDefaultCtor<U>, int> = 0>
    constexpr TypeWrapper() noexcept(IsNoThrowDefaultCtor<T>)
        : m_value()
    {
        RAD_S_ASSERT_NOTHROW((IsNoThrowDefaultCtor<T>));
    }

    // wrapped-type move ctor
    constexpr explicit TypeWrapper(T&& value) noexcept(IsNoThrowMoveCtor<T>)
        : m_value(Forward<T>(value))
    {
        RAD_S_ASSERT_NOTHROW((IsNoThrowMoveCtor<T>));
    }

#if RAD_ENABLE_STD
    // wrapped-type initializer_list ctor
    template <typename U, EnIf<IsCtor<T, std::initializer_list<U>>, int> = 0>
    constexpr explicit TypeWrapper(std::initializer_list<U> init) noexcept(
        IsNoThrowCtor<T, std::initializer_list<U>>)
        : m_value(init)
    {
        RAD_S_ASSERT_NOTHROW((IsNoThrowCtor<T, std::initializer_list<U>>));
    }

    template <typename U>
    constexpr TypeWrapper(std::initializer_list<TypeWrapper<U>>) = delete;
#endif

    // wrapper convertible ctor
    template <typename U, EnIf<IsCtor<T, const U&>, int> = 0>
    constexpr explicit TypeWrapper(const TypeWrapper<U>& container) noexcept(
        IsNoThrowCtor<T, const U&>)
        : m_value(container.Get())
    {
        RAD_S_ASSERT_NOTHROW((IsNoThrowCtor<T, const U&>));
    }

    // wrapper convertible move ctor
    template <typename U, EnIf<IsCtor<T, U&&>, int> = 0>
    constexpr explicit TypeWrapper(TypeWrapper<U>&& container) noexcept(
        IsNoThrowCtor<T, U&&>)
        : m_value(Forward<TypeWrapper<U>>(container).Get())
    {
        RAD_S_ASSERT_NOTHROW((IsNoThrowCtor<T, U&&>));
    }

    // arg forwarding ctor (without wrapper copy/move ctor conflict)
    template <typename... TArgs,
              typename = EnIf<!IsRelated<TypeWrapper, TArgs...> &&
                              IsCtor<T, TArgs...>>>
    constexpr TypeWrapper(TArgs&&... args) noexcept(
        IsNoThrowCtor<T, TArgs&&...>)
        : m_value(Forward<TArgs>(args)...)
    {
        RAD_S_ASSERT_NOTHROW((IsNoThrowCtor<T, TArgs&&...>));
    }

    // type assign
    template <typename U = T>
    constexpr TypeWrapper& operator=(const U& r) noexcept(
        noexcept(DeclVal<TypeWrapper*>()->m_value = r))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(m_value = r));
        m_value = r;
        return *this;
    }

    // type move assign
    constexpr TypeWrapper& operator=(T&& r) noexcept(
        noexcept(DeclVal<TypeWrapper*>()->m_value = Forward<T>(r)))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(m_value = Forward<T>(r)));
        m_value = Forward<T>(r);
        return *this;
    }

    // wrapper copy assign
    template <typename U = T>
    constexpr TypeWrapper& operator=(const TypeWrapper<U>& r) noexcept(
        noexcept(DeclVal<TypeWrapper*>()->m_value = r.Get()))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(m_value = r.Get()));
        m_value = r.Get();
        return *this;
    }

    // wrapper move assign
    template <typename U = T>
    constexpr TypeWrapper& operator=(TypeWrapper<U>&& r) noexcept(noexcept(
        DeclVal<TypeWrapper*>()->m_value = Forward<TypeWrapper<U>>(r).Get()))
    {
        RAD_S_ASSERT_NOTHROW(
            noexcept(m_value = Forward<TypeWrapper<U>>(r).Get()));
        m_value = Forward<TypeWrapper<U>>(r).Get();
        return *this;
    }

    constexpr T& Get() & noexcept
    {
        return m_value;
    }

    constexpr const T& Get() const& noexcept
    {
        return m_value;
    }

    constexpr T&& Get() && noexcept
    {
        return Move(m_value);
    }

private:

    T m_value;
};

template <typename T>
class TypeWrapper<T, true>
{
public:

    using Type = T;

    constexpr TypeWrapper() = delete;

    template <typename U, EnIf<IsCtor<T, U>, int> = 0>
    constexpr explicit TypeWrapper(U&& val) noexcept
        : m_ref(Forward<U>(val))
    {
    }

    template <typename U, EnIf<IsLRefBindable<T, U&>, int> = 0>
    constexpr explicit TypeWrapper(TypeWrapper<U>& r) noexcept
        : m_ref(r.Get())
    {
    }

    template <typename U, EnIf<IsLRefBindable<T, const U&>, int> = 0>
    constexpr explicit TypeWrapper(const TypeWrapper<U>& r) noexcept
        : m_ref(r.Get())
    {
    }

    template <typename U, EnIf<IsLRefBindable<T, U&>, int> = 0>
    constexpr explicit TypeWrapper(TypeWrapper<U>&& r) noexcept
        : m_ref(r.Get())
    {
    }

    template <typename U, EnIf<IsCtor<T, U>, int> = 0>
    constexpr TypeWrapper& operator=(U&& val) noexcept
    {
        new (this) TypeWrapper(Forward<U>(val));
        return *this;
    }

    template <typename U, EnIf<IsCtor<T, U&>, int> = 0>
    constexpr TypeWrapper& operator=(TypeWrapper<U>& r) noexcept
    {
        new (this) TypeWrapper(r.Get());
        return *this;
    }

    constexpr T& Get() noexcept
    {
        return m_ref;
    }

    constexpr const T& Get() const noexcept
    {
        return m_ref;
    }

private:

    T m_ref;
};

template <typename T, typename U>
inline bool operator==(const TypeWrapper<T>& l,
                       const TypeWrapper<U>& r) noexcept
{
    return l.Get() == r.Get();
}

template <typename T, typename U>
inline bool operator==(const TypeWrapper<T>& l, const U& r) noexcept
{
    return l.Get() == r;
}

template <typename T, typename U>
inline bool operator==(const U& l, const TypeWrapper<T>& r) noexcept
{
    return r.Get() == l;
}

template <typename T, typename U>
inline bool operator!=(const TypeWrapper<T>& l,
                       const TypeWrapper<U>& r) noexcept
{
    return !(l == r);
}

template <typename T, typename U>
inline bool operator!=(const TypeWrapper<T>& l, const U& r) noexcept
{
    return !(l == r);
}

template <typename T, typename U>
inline bool operator!=(const U& r, const TypeWrapper<T>& l) noexcept
{
    return !(l == r);
}

template <typename T, typename U>
inline bool operator<(const TypeWrapper<T>& l, const TypeWrapper<U>& r) noexcept
{
    return l.Get() < r.Get();
}

template <typename T, typename U>
inline bool operator<(const TypeWrapper<T>& l, const U& r) noexcept
{
    return l.Get() < r;
}

template <typename T, typename U>
inline bool operator<(const U& l, const TypeWrapper<T>& r) noexcept
{
    return l < r.Get();
}

template <typename T, typename U>
inline bool operator<=(const TypeWrapper<T>& l,
                       const TypeWrapper<U>& r) noexcept
{
    return l.Get() <= r.Get();
}

template <typename T, typename U>
inline bool operator<=(const TypeWrapper<T>& l, const U& r) noexcept
{
    return l.Get() <= r;
}

template <typename T, typename U>
inline bool operator<=(const U& l, const TypeWrapper<T>& r) noexcept
{
    return l <= r.Get();
}

template <typename T, typename U>
inline bool operator>(const TypeWrapper<T>& l, const TypeWrapper<U>& r) noexcept
{
    return l.Get() > r.Get();
}

template <typename T, typename U>
inline bool operator>(const TypeWrapper<T>& l, const U& r) noexcept
{
    return l.Get() > r;
}

template <typename T, typename U>
inline bool operator>(const U& l, const TypeWrapper<T>& r) noexcept
{
    return l > r.Get();
}

template <typename T, typename U>
inline bool operator>=(const TypeWrapper<T>& l,
                       const TypeWrapper<U>& r) noexcept
{
    return l.Get() >= r.Get();
}

template <typename T, typename U>
inline bool operator>=(const TypeWrapper<T>& l, const U& r) noexcept
{
    return l.Get() >= r;
}

template <typename T, typename U>
inline bool operator>=(const U& l, const TypeWrapper<T>& r) noexcept
{
    return l >= r.Get();
}

} // namespace rad
