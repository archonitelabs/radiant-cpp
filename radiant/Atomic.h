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

#include "radiant/detail/AtomicIntrinsics.h"

namespace rad
{

template <typename TOrderTag>
using MemoryOrderTag = detail::atomic::OrderTag<TOrderTag>;

RAD_INLINE_VAR constexpr detail::atomic::RelaxedTag MemOrderRelaxed{};
RAD_INLINE_VAR constexpr detail::atomic::ConsumeTag MemOrderConsume{};
RAD_INLINE_VAR constexpr detail::atomic::AcquireTag MemOrderAcquire{};
RAD_INLINE_VAR constexpr detail::atomic::ReleaseTag MemOrderRelease{};
RAD_INLINE_VAR constexpr detail::atomic::AcqRelTag MemOrderAcqRel{};
RAD_INLINE_VAR constexpr detail::atomic::SeqCstTag MemOrderSeqCst{};

namespace detail
{
namespace atomic
{

template <typename T>
class AtomicIntegral
{
public:

    RAD_S_ASSERTMSG(IsIntegral<T>,
                    "rad::Atomic supports only integral and pointer types");

    using ValueType = T;
    using DifferenceType = ValueType;

    constexpr AtomicIntegral() noexcept = default;

    constexpr AtomicIntegral(T value) noexcept
        : m_val(value)
    {
    }

    RAD_NOT_COPYABLE(AtomicIntegral);

#ifndef RAD_REQUIRE_EXPLICIT_ATOMIC_ORDERING

    template <typename Order = detail::atomic::SeqCstTag>
    void Store(T val, MemoryOrderTag<Order> = MemoryOrderTag<Order>()) noexcept
    {
        detail::atomic::SelectIntrinsic<T>::Store(m_val, val, Order());
    }

    template <typename Order = detail::atomic::SeqCstTag>
    T Load(MemoryOrderTag<Order> = MemoryOrderTag<Order>()) const noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::Load(m_val, Order());
    }

    template <typename Order = detail::atomic::SeqCstTag>
    T Exchange(T val, MemoryOrderTag<Order> = MemoryOrderTag<Order>()) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::Exchange(m_val,
                                                            val,
                                                            Order());
    }

    template <typename Success, typename Failure>
    bool CompareExchangeWeak(T& expected,
                             T desired,
                             MemoryOrderTag<Success>,
                             MemoryOrderTag<Failure>) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::CompareExchangeWeak(
            m_val,
            desired,
            expected,
            Success(),
            Failure());
    }

    template <typename Order = detail::atomic::SeqCstTag>
    bool CompareExchangeWeak(
        T& expected,
        T desired,
        MemoryOrderTag<Order> = MemoryOrderTag<Order>()) noexcept
    {
        return CompareExchangeWeak<Order, Order>(expected,
                                                 desired,
                                                 Order(),
                                                 Order());
    }

    template <typename Success, typename Failure>
    bool CompareExchangeStrong(T& expected,
                               T desired,
                               MemoryOrderTag<Success>,
                               MemoryOrderTag<Failure>) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::CompareExchangeStrong(
            m_val,
            desired,
            expected,
            Success(),
            Failure());
    }

    template <typename Order = detail::atomic::SeqCstTag>
    bool CompareExchangeStrong(
        T& expected,
        T desired,
        MemoryOrderTag<Order> = MemoryOrderTag<Order>()) noexcept
    {
        return CompareExchangeStrong(expected, desired, Order(), Order());
    }

    template <typename Order = rad::detail::atomic::SeqCstTag>
    T FetchAdd(T val, MemoryOrderTag<Order> = MemoryOrderTag<Order>()) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::FetchAdd(m_val,
                                                            val,
                                                            Order());
    }

    template <typename Order = rad::detail::atomic::SeqCstTag>
    T FetchSub(T val, MemoryOrderTag<Order> = MemoryOrderTag<Order>()) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::FetchSub(m_val,
                                                            val,
                                                            Order());
    }

    template <typename Order = rad::detail::atomic::SeqCstTag>
    T FetchAnd(T val, MemoryOrderTag<Order> = MemoryOrderTag<Order>()) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::FetchAnd(m_val,
                                                            val,
                                                            Order());
    }

    template <typename Order = rad::detail::atomic::SeqCstTag>
    T FetchOr(T val, MemoryOrderTag<Order> = MemoryOrderTag<Order>()) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::FetchOr(m_val, val, Order());
    }

    template <typename Order = rad::detail::atomic::SeqCstTag>
    T FetchXor(T val, MemoryOrderTag<Order> = MemoryOrderTag<Order>()) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::FetchXor(m_val,
                                                            val,
                                                            Order());
    }

    operator T() const noexcept
    {
        return Load();
    }

    T operator=(T val) noexcept
    {
        Store(val);
        return val;
    }

    T operator++() noexcept
    {
        return static_cast<T>(FetchAdd(1) + static_cast<T>(1));
    }

    T operator++(int) noexcept
    {
        return FetchAdd(1);
    }

    T operator--() noexcept
    {
        return static_cast<T>(FetchSub(1) - static_cast<T>(1));
    }

    T operator--(int) noexcept
    {
        return FetchSub(1);
    }

    T operator+=(T val) noexcept
    {
        return static_cast<T>(FetchAdd(val) + val);
    }

    T operator-=(T val) noexcept
    {
        return static_cast<T>(FetchSub(val) - val);
    }

    T operator&=(T val) noexcept
    {
        return FetchAnd(val) & val;
    }

    T operator|=(T val) noexcept
    {
        return FetchOr(val) | val;
    }

    T operator^=(T val) noexcept
    {
        return FetchXor(val) ^ val;
    }
#else
    template <typename Order>
    void Store(T val, MemoryOrderTag<Order>) noexcept
    {
        detail::atomic::SelectIntrinsic<T>::Store(m_val, val, Order());
    }

    template <typename Order>
    T Load(MemoryOrderTag<Order>) const noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::Load(m_val, Order());
    }

    template <typename Order>
    T Exchange(T val, MemoryOrderTag<Order>) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::Exchange(m_val,
                                                            val,
                                                            Order());
    }

    template <typename Success, typename Failure>
    bool CompareExchangeWeak(T& expected,
                             T desired,
                             MemoryOrderTag<Success>,
                             MemoryOrderTag<Failure>) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::CompareExchangeWeak(
            m_val,
            desired,
            expected,
            Success(),
            Failure());
    }

    template <typename Order>
    bool CompareExchangeWeak(T& expected,
                             T desired,
                             MemoryOrderTag<Order>) noexcept
    {
        return CompareExchangeWeak<Order, Order>(expected,
                                                 desired,
                                                 Order(),
                                                 Order());
    }

    template <typename Success, typename Failure>
    bool CompareExchangeStrong(T& expected,
                               T desired,
                               MemoryOrderTag<Success>,
                               MemoryOrderTag<Failure>) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::CompareExchangeStrong(
            m_val,
            desired,
            expected,
            Success(),
            Failure());
    }

    template <typename Order>
    bool CompareExchangeStrong(T& expected,
                               T desired,
                               MemoryOrderTag<Order>) noexcept
    {
        return CompareExchangeStrong(expected, desired, Order(), Order());
    }

    template <typename Order>
    T FetchAdd(T val, MemoryOrderTag<Order>) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::FetchAdd(m_val,
                                                            val,
                                                            Order());
    }

    template <typename Order>
    T FetchSub(T val, MemoryOrderTag<Order>) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::FetchSub(m_val,
                                                            val,
                                                            Order());
    }

    template <typename Order>
    T FetchAnd(T val, MemoryOrderTag<Order>) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::FetchAnd(m_val,
                                                            val,
                                                            Order());
    }

    template <typename Order>
    T FetchOr(T val, MemoryOrderTag<Order>) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::FetchOr(m_val, val, Order());
    }

    template <typename Order>
    T FetchXor(T val, MemoryOrderTag<Order>) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::FetchXor(m_val,
                                                            val,
                                                            Order());
    }
#endif

private:

    T m_val{};
};

template <typename T>
class AtomicPointer
{
public:

    RAD_S_ASSERTMSG(IsPointer<T>,
                    "rad::Atomic supports only integral and pointer types");

    using ValueType = T;
    using DifferenceType = ValueType;

    constexpr AtomicPointer() noexcept = default;

    constexpr AtomicPointer(T value) noexcept
        : m_val(value)
    {
    }

    RAD_NOT_COPYABLE(AtomicPointer);

#ifndef RAD_REQUIRE_EXPLICIT_ATOMIC_ORDERING

    template <typename Order = detail::atomic::SeqCstTag>
    void Store(T val, MemoryOrderTag<Order> = MemoryOrderTag<Order>()) noexcept
    {
        detail::atomic::SelectIntrinsic<T>::Store(m_val, val, Order());
    }

    template <typename Order = detail::atomic::SeqCstTag>
    T Load(MemoryOrderTag<Order> = MemoryOrderTag<Order>()) const noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::Load(m_val, Order());
    }

    template <typename Order = detail::atomic::SeqCstTag>
    T Exchange(T val, MemoryOrderTag<Order> = MemoryOrderTag<Order>()) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::Exchange(m_val,
                                                            val,
                                                            Order());
    }

    template <typename Success, typename Failure>
    bool CompareExchangeWeak(T& expected,
                             T desired,
                             MemoryOrderTag<Success>,
                             MemoryOrderTag<Failure>) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::CompareExchangeWeak(
            m_val,
            desired,
            expected,
            Success(),
            Failure());
    }

    template <typename Order = detail::atomic::SeqCstTag>
    bool CompareExchangeWeak(
        T& expected,
        T desired,
        MemoryOrderTag<Order> = MemoryOrderTag<Order>()) noexcept
    {
        return CompareExchangeWeak<Order, Order>(expected,
                                                 desired,
                                                 Order(),
                                                 Order());
    }

    template <typename Success, typename Failure>
    bool CompareExchangeStrong(T& expected,
                               T desired,
                               MemoryOrderTag<Success>,
                               MemoryOrderTag<Failure>) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::CompareExchangeStrong(
            m_val,
            desired,
            expected,
            Success(),
            Failure());
    }

    template <typename Order = detail::atomic::SeqCstTag>
    bool CompareExchangeStrong(
        T& expected,
        T desired,
        MemoryOrderTag<Order> = MemoryOrderTag<Order>()) noexcept
    {
        return CompareExchangeStrong(expected, desired, Order(), Order());
    }

    template <typename Order = rad::detail::atomic::SeqCstTag>
    T FetchAdd(ptrdiff_t val,
               MemoryOrderTag<Order> = MemoryOrderTag<Order>()) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::FetchAdd(m_val,
                                                            val,
                                                            Order());
    }

    template <typename Order = rad::detail::atomic::SeqCstTag>
    T FetchSub(ptrdiff_t val,
               MemoryOrderTag<Order> = MemoryOrderTag<Order>()) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::FetchSub(m_val,
                                                            val,
                                                            Order());
    }

    operator T() const noexcept
    {
        return Load();
    }

    T operator=(T val) noexcept
    {
        Store(val);
        return val;
    }

    T operator++() noexcept
    {
        return static_cast<T>(FetchAdd(1) + static_cast<ptrdiff_t>(1));
    }

    T operator++(int) noexcept
    {
        return FetchAdd(1);
    }

    T operator--() noexcept
    {
        return static_cast<T>(FetchSub(1) - static_cast<ptrdiff_t>(1));
    }

    T operator--(int) noexcept
    {
        return FetchSub(1);
    }

    T operator+=(ptrdiff_t val) noexcept
    {
        return static_cast<T>(FetchAdd(val) + val);
    }

    T operator-=(ptrdiff_t val) noexcept
    {
        return static_cast<T>(FetchSub(val) - val);
    }

#else

    template <typename Order>
    void Store(T val, MemoryOrderTag<Order>) noexcept
    {
        detail::atomic::SelectIntrinsic<T>::Store(m_val, val, Order());
    }

    template <typename Order>
    T Load(MemoryOrderTag<Order>) const noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::Load(m_val, Order());
    }

    template <typename Order>
    T Exchange(T val, MemoryOrderTag<Order>) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::Exchange(m_val,
                                                            val,
                                                            Order());
    }

    template <typename Success, typename Failure>
    bool CompareExchangeWeak(T& expected,
                             T desired,
                             MemoryOrderTag<Success>,
                             MemoryOrderTag<Failure>) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::CompareExchangeWeak(
            m_val,
            desired,
            expected,
            Success(),
            Failure());
    }

    template <typename Order>
    bool CompareExchangeWeak(T& expected,
                             T desired,
                             MemoryOrderTag<Order>) noexcept
    {
        return CompareExchangeWeak<Order, Order>(expected,
                                                 desired,
                                                 Order(),
                                                 Order());
    }

    template <typename Success, typename Failure>
    bool CompareExchangeStrong(T& expected,
                               T desired,
                               MemoryOrderTag<Success>,
                               MemoryOrderTag<Failure>) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::CompareExchangeStrong(
            m_val,
            desired,
            expected,
            Success(),
            Failure());
    }

    template <typename Order>
    bool CompareExchangeStrong(T& expected,
                               T desired,
                               MemoryOrderTag<Order>) noexcept
    {
        return CompareExchangeStrong(expected, desired, Order(), Order());
    }

    template <typename Order>
    T FetchAdd(ptrdiff_t val, MemoryOrderTag<Order>) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::FetchAdd(m_val,
                                                            val,
                                                            Order());
    }

    template <typename Order>
    T FetchSub(ptrdiff_t val, MemoryOrderTag<Order>) noexcept
    {
        return detail::atomic::SelectIntrinsic<T>::FetchSub(m_val,
                                                            val,
                                                            Order());
    }

#endif

private:

    T m_val{};
};

} // namespace atomic
} // namespace detail

template <typename T>
class Atomic final : public Cond<IsIntegral<T>,
                                 detail::atomic::AtomicIntegral<T>,
                                 detail::atomic::AtomicPointer<T>>
{
    using BaseType = Cond<IsIntegral<T>,
                          detail::atomic::AtomicIntegral<T>,
                          detail::atomic::AtomicPointer<T>>;

public:

    RAD_NOT_COPYABLE(Atomic);

    constexpr Atomic() noexcept
        : BaseType()
    {
    }

    using BaseType::BaseType;
    using BaseType::operator=;
};

} // namespace rad
