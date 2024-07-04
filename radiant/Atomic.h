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

#if RAD_REQUIRE_EXPLICIT_ATOMIC_ORDERING
#define RAD_ATOMIC_MEMORDER_T typename Order
#define RAD_ATOMIC_MEMORDER_P MemoryOrderTag<Order>
#else
#define RAD_ATOMIC_MEMORDER_T typename Order = detail::atomic::SeqCstTag
#define RAD_ATOMIC_MEMORDER_P MemoryOrderTag<Order> = MemoryOrderTag<Order>()
#endif

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

    template <RAD_ATOMIC_MEMORDER_T>
    void Store(T val, RAD_ATOMIC_MEMORDER_P) noexcept
    {
        SelectIntrinsic<T>::Store(m_val, val, Order());
    }

    template <RAD_ATOMIC_MEMORDER_T>
    T Load(RAD_ATOMIC_MEMORDER_P) const noexcept
    {
        return SelectIntrinsic<T>::Load(m_val, Order());
    }

    template <RAD_ATOMIC_MEMORDER_T>
    T Exchange(T val, RAD_ATOMIC_MEMORDER_P) noexcept
    {
        return SelectIntrinsic<T>::Exchange(m_val, val, Order());
    }

    template <typename Success, typename Failure>
    bool CompareExchangeWeak(T& expected,
                             T desired,
                             MemoryOrderTag<Success>,
                             MemoryOrderTag<Failure>) noexcept
    {
        return SelectIntrinsic<T>::CompareExchangeWeak(m_val,
                                                       desired,
                                                       expected,
                                                       Success(),
                                                       Failure());
    }

    template <RAD_ATOMIC_MEMORDER_T>
    bool CompareExchangeWeak(T& expected,
                             T desired,
                             RAD_ATOMIC_MEMORDER_P) noexcept
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
        return SelectIntrinsic<T>::CompareExchangeStrong(m_val,
                                                         desired,
                                                         expected,
                                                         Success(),
                                                         Failure());
    }

    template <RAD_ATOMIC_MEMORDER_T>
    bool CompareExchangeStrong(T& expected,
                               T desired,
                               RAD_ATOMIC_MEMORDER_P) noexcept
    {
        return CompareExchangeStrong(expected, desired, Order(), Order());
    }

    template <RAD_ATOMIC_MEMORDER_T>
    T FetchAdd(T val, RAD_ATOMIC_MEMORDER_P) noexcept
    {
        return SelectIntrinsic<T>::FetchAdd(m_val, val, Order());
    }

    template <RAD_ATOMIC_MEMORDER_T>
    T FetchSub(T val, RAD_ATOMIC_MEMORDER_P) noexcept
    {
        return SelectIntrinsic<T>::FetchSub(m_val, val, Order());
    }

    template <RAD_ATOMIC_MEMORDER_T>
    T FetchAnd(T val, RAD_ATOMIC_MEMORDER_P) noexcept
    {
        return SelectIntrinsic<T>::FetchAnd(m_val, val, Order());
    }

    template <RAD_ATOMIC_MEMORDER_T>
    T FetchOr(T val, RAD_ATOMIC_MEMORDER_P) noexcept
    {
        return SelectIntrinsic<T>::FetchOr(m_val, val, Order());
    }

    template <RAD_ATOMIC_MEMORDER_T>
    T FetchXor(T val, RAD_ATOMIC_MEMORDER_P) noexcept
    {
        return SelectIntrinsic<T>::FetchXor(m_val, val, Order());
    }

#if !RAD_REQUIRE_EXPLICIT_ATOMIC_ORDERING
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
#endif // !RAD_REQUIRE_EXPLICIT_ATOMIC_ORDERING

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

    template <RAD_ATOMIC_MEMORDER_T>
    void Store(T val, RAD_ATOMIC_MEMORDER_P) noexcept
    {
        SelectIntrinsic<T>::Store(m_val, val, Order());
    }

    template <RAD_ATOMIC_MEMORDER_T>
    T Load(RAD_ATOMIC_MEMORDER_P) const noexcept
    {
        return SelectIntrinsic<T>::Load(m_val, Order());
    }

    template <RAD_ATOMIC_MEMORDER_T>
    T Exchange(T val, RAD_ATOMIC_MEMORDER_P) noexcept
    {
        return SelectIntrinsic<T>::Exchange(m_val, val, Order());
    }

    template <typename Success, typename Failure>
    bool CompareExchangeWeak(T& expected,
                             T desired,
                             MemoryOrderTag<Success>,
                             MemoryOrderTag<Failure>) noexcept
    {
        return SelectIntrinsic<T>::CompareExchangeWeak(m_val,
                                                       desired,
                                                       expected,
                                                       Success(),
                                                       Failure());
    }

    template <RAD_ATOMIC_MEMORDER_T>
    bool CompareExchangeWeak(T& expected,
                             T desired,
                             RAD_ATOMIC_MEMORDER_P) noexcept
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
        return SelectIntrinsic<T>::CompareExchangeStrong(m_val,
                                                         desired,
                                                         expected,
                                                         Success(),
                                                         Failure());
    }

    template <RAD_ATOMIC_MEMORDER_T>
    bool CompareExchangeStrong(T& expected,
                               T desired,
                               RAD_ATOMIC_MEMORDER_P) noexcept
    {
        return CompareExchangeStrong(expected, desired, Order(), Order());
    }

    template <RAD_ATOMIC_MEMORDER_T>
    T FetchAdd(ptrdiff_t val, RAD_ATOMIC_MEMORDER_P) noexcept
    {
        return SelectIntrinsic<T>::FetchAdd(m_val, val, Order());
    }

    template <RAD_ATOMIC_MEMORDER_T>
    T FetchSub(ptrdiff_t val, RAD_ATOMIC_MEMORDER_P) noexcept
    {
        return SelectIntrinsic<T>::FetchSub(m_val, val, Order());
    }

#if !RAD_REQUIRE_EXPLICIT_ATOMIC_ORDERING
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
#endif // !RAD_REQUIRE_EXPLICIT_ATOMIC_ORDERING

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
