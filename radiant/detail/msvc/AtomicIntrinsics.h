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

#if defined(_M_ARM) || defined(_M_ARM64) || defined(_M_ARM64EC)
#define RAD_MEM_BARRIER            __dmb(0xb); // _ARM_BARRIER_ISH / _ARM64_BARRIER_ISH
#define RAD_INTRIN_RELAXED(Intrin) RAD_CONCAT(Intrin, _nf)
#define RAD_INTRIN_ACQUIRE(Intrin) RAD_CONCAT(Intrin, _acq)
#define RAD_INTRIN_RELEASE(Intrin) RAD_CONCAT(Intrin, _rel)
#elif defined(_M_IX86) || defined(_M_AMD64)
#define RAD_MEM_BARRIER                                                        \
    _Pragma("warning(push)") _Pragma("warning(disable : 4996)")                \
        _ReadWriteBarrier() _Pragma("warning(pop)")
#define RAD_INTRIN_RELAXED(Intrin) Intrin
#define RAD_INTRIN_ACQUIRE(Intrin) Intrin
#define RAD_INTRIN_RELEASE(Intrin) Intrin
#endif

// clang-format off
RAD_INLINE_VAR constexpr MemoryOrder combinedOrders[6][6] = {
    {MemoryOrder::Relaxed, MemoryOrder::Consume, MemoryOrder::Acquire, MemoryOrder::Release, MemoryOrder::AcqRel, MemoryOrder::SeqCst},
    {MemoryOrder::Consume, MemoryOrder::Consume, MemoryOrder::Acquire, MemoryOrder::AcqRel,  MemoryOrder::AcqRel, MemoryOrder::SeqCst},
    {MemoryOrder::Acquire, MemoryOrder::Acquire, MemoryOrder::Acquire, MemoryOrder::AcqRel,  MemoryOrder::AcqRel, MemoryOrder::SeqCst},
    {MemoryOrder::Release, MemoryOrder::AcqRel,  MemoryOrder::AcqRel,  MemoryOrder::Release, MemoryOrder::AcqRel, MemoryOrder::SeqCst},
    {MemoryOrder::AcqRel,  MemoryOrder::AcqRel,  MemoryOrder::AcqRel,  MemoryOrder::AcqRel,  MemoryOrder::AcqRel, MemoryOrder::SeqCst},
    {MemoryOrder::SeqCst,  MemoryOrder::SeqCst,  MemoryOrder::SeqCst,  MemoryOrder::SeqCst,  MemoryOrder::SeqCst, MemoryOrder::SeqCst}};

// clang-format on

constexpr inline MemoryOrder CombineMemoryOrders(MemoryOrder success,
                                                 MemoryOrder fail) noexcept
{
    return combinedOrders[static_cast<int>(success)][static_cast<int>(fail)];
}

template <typename T>
constexpr inline T Negate(const T val) noexcept
{
    return static_cast<T>(0U - static_cast<MakeUnsigned<T>>(val));
}

template <typename T, size_t Size = sizeof(T)>
struct SelectIntrinsic
{
    RAD_S_ASSERTMSG(
        Size == 0,
        "rad::detail::atomic::SelectIntrinsic not supported for this type");
};

template <typename T>
struct SelectIntrinsic<T, 1>
{
    RAD_S_ASSERT(sizeof(T) == 1);
    using Type = char;

    static inline T Load(const volatile T& storage, RelaxedTag) noexcept
    {
        return static_cast<T>(__iso_volatile_load8(AddrAs<Type>(storage)));
    }

    template <typename TOrder>
    static inline T Load(const volatile T& storage, OrderTag<TOrder>) noexcept
    {
        CheckLoadMemoryOrder<TOrder>();
        T ret = static_cast<T>(__iso_volatile_load8(AddrAs<Type>(storage)));
        RAD_MEM_BARRIER;
        return ret;
    }

    static inline void Store(volatile T& storage, T val, RelaxedTag) noexcept
    {
        __iso_volatile_store8(AddrAs<Type>(storage), ValAs<Type>(val));
    }

    template <typename TOrder>
    static inline void Store(volatile T& storage,
                             T val,
                             OrderTag<TOrder>) noexcept
    {
        CheckStoreMemoryOrder<TOrder>();
        RAD_MEM_BARRIER;
        __iso_volatile_store8(AddrAs<Type>(storage), ValAs<Type>(val));
    }

    static inline T Exchange(volatile T& storage, T val, RelaxedTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELAXED(
            _InterlockedExchange8)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T Exchange(volatile T& storage, T val, AcquireTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_ACQUIRE(
            _InterlockedExchange8)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T Exchange(volatile T& storage, T val, ReleaseTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELEASE(
            _InterlockedExchange8)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T Exchange(volatile T& storage, T val, SeqCstTag) noexcept
    {
        return static_cast<T>(
            _InterlockedExchange8(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline bool CasRet(T& expected, Type comparand, Type old) noexcept
    {
        if (old != comparand)
        {
            expected = static_cast<T>(old);
            return false;
        }
        return true;
    }

    template <typename Ts, typename Tf>
    static inline bool CompareExchangeWeak(volatile T& storage,
                                           T val,
                                           T& expected,
                                           OrderTag<Ts>,
                                           OrderTag<Tf>) noexcept
    {
        // MSVC does not provide a weak CAS intrinsic for any platform
        Ts success;
        Tf fail;
        return CompareExchangeStrong(storage, val, expected, success, fail);
    }

    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             ReleaseTag,
                                             ReleaseTag) noexcept
    {
        ReleaseTag success;
        RelaxedTag fail;
        return CompareExchangeStrong(storage, val, expected, success, fail);
    }

    static inline bool CompareExchangeStrong(
        volatile T& storage, T val, T& expected, AcqRelTag, AcqRelTag) noexcept
    {
        AcqRelTag success;
        AcquireTag fail;
        return CompareExchangeStrong(storage, val, expected, success, fail);
    }

    template <typename Ts, typename Tf>
    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             OrderTag<Ts>,
                                             OrderTag<Tf>) noexcept
    {
        CheckLoadMemoryOrder<Tf>();
        CheckCasMemoryOrdering<Ts, Tf>();
        typename OrderToTag<CombineMemoryOrders(Ts::Order, Tf::Order)>::Type o;
        return CompareExchangeStrong(storage, val, expected, o);
    }

    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             RelaxedTag) noexcept
    {
        return CasRet(expected,
                      ValAs<Type>(expected),
                      RAD_INTRIN_RELAXED(_InterlockedCompareExchange8)(
                          AddrAs<Type>(storage),
                          ValAs<Type>(val),
                          ValAs<Type>(expected)));
    }

    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             AcquireTag) noexcept
    {
        return CasRet(expected,
                      ValAs<Type>(expected),
                      RAD_INTRIN_ACQUIRE(_InterlockedCompareExchange8)(
                          AddrAs<Type>(storage),
                          ValAs<Type>(val),
                          ValAs<Type>(expected)));
    }

    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             ReleaseTag) noexcept
    {
        return CasRet(expected,
                      ValAs<Type>(expected),
                      RAD_INTRIN_RELEASE(_InterlockedCompareExchange8)(
                          AddrAs<Type>(storage),
                          ValAs<Type>(val),
                          ValAs<Type>(expected)));
    }

    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             SeqCstTag) noexcept
    {
        return CasRet(expected,
                      ValAs<Type>(expected),
                      _InterlockedCompareExchange8(AddrAs<Type>(storage),
                                                   ValAs<Type>(val),
                                                   ValAs<Type>(expected)));
    }

    static inline T FetchAdd(volatile T& storage, T val, RelaxedTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELAXED(
            _InterlockedExchangeAdd8)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchAdd(volatile T& storage, T val, AcquireTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_ACQUIRE(
            _InterlockedExchangeAdd8)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchAdd(volatile T& storage, T val, ReleaseTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELEASE(
            _InterlockedExchangeAdd8)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchAdd(volatile T& storage, T val, SeqCstTag) noexcept
    {
        return static_cast<T>(
            _InterlockedExchangeAdd8(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    template <typename TOrder>
    static inline T FetchSub(volatile T& storage,
                             T val,
                             OrderTag<TOrder>) noexcept
    {
        typename OrderTag<TOrder>::Type order;
        return FetchAdd(storage, Negate(val), order);
    }

    static inline T FetchAnd(volatile T& storage, T val, RelaxedTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELAXED(
            _InterlockedAnd8)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchAnd(volatile T& storage, T val, AcquireTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_ACQUIRE(
            _InterlockedAnd8)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchAnd(volatile T& storage, T val, ReleaseTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELEASE(
            _InterlockedAnd8)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchAnd(volatile T& storage, T val, SeqCstTag) noexcept
    {
        return static_cast<T>(
            _InterlockedAnd8(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchOr(volatile T& storage, T val, RelaxedTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELAXED(
            _InterlockedOr8)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchOr(volatile T& storage, T val, AcquireTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_ACQUIRE(
            _InterlockedOr8)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchOr(volatile T& storage, T val, ReleaseTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELEASE(
            _InterlockedOr8)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchOr(volatile T& storage, T val, SeqCstTag) noexcept
    {
        return static_cast<T>(
            _InterlockedOr8(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchXor(volatile T& storage, T val, RelaxedTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELAXED(
            _InterlockedXor8)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchXor(volatile T& storage, T val, AcquireTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_ACQUIRE(
            _InterlockedXor8)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchXor(volatile T& storage, T val, ReleaseTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELEASE(
            _InterlockedXor8)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchXor(volatile T& storage, T val, SeqCstTag) noexcept
    {
        return static_cast<T>(
            _InterlockedXor8(AddrAs<Type>(storage), ValAs<Type>(val)));
    }
};

template <typename T>
struct SelectIntrinsic<T, 2>
{
    RAD_S_ASSERT(sizeof(T) == 2);
    using Type = short;

    static inline T Load(const volatile T& storage, RelaxedTag) noexcept
    {
        return static_cast<T>(__iso_volatile_load16(AddrAs<Type>(storage)));
    }

    template <typename TOrder>
    static inline T Load(const volatile T& storage, OrderTag<TOrder>) noexcept
    {
        CheckLoadMemoryOrder<TOrder>();
        T ret = static_cast<T>(__iso_volatile_load16(AddrAs<Type>(storage)));
        RAD_MEM_BARRIER;
        return ret;
    }

    static inline void Store(volatile T& storage, T val, RelaxedTag) noexcept
    {
        __iso_volatile_store16(AddrAs<Type>(storage), ValAs<Type>(val));
    }

    template <typename TOrder>
    static inline void Store(volatile T& storage,
                             T val,
                             OrderTag<TOrder>) noexcept
    {
        CheckStoreMemoryOrder<TOrder>();
        RAD_MEM_BARRIER;
        __iso_volatile_store16(AddrAs<Type>(storage), ValAs<Type>(val));
    }

    static inline T Exchange(volatile T& storage, T val, RelaxedTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELAXED(
            _InterlockedExchange16)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T Exchange(volatile T& storage, T val, AcquireTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_ACQUIRE(
            _InterlockedExchange16)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T Exchange(volatile T& storage, T val, ReleaseTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELEASE(
            _InterlockedExchange16)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T Exchange(volatile T& storage, T val, SeqCstTag) noexcept
    {
        return static_cast<T>(
            _InterlockedExchange16(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline bool CasRet(T& expected, Type comparand, Type old) noexcept
    {
        if (old != comparand)
        {
            expected = static_cast<T>(old);
            return false;
        }
        return true;
    }

    template <typename Ts, typename Tf>
    static inline bool CompareExchangeWeak(volatile T& storage,
                                           T val,
                                           T& expected,
                                           OrderTag<Ts>,
                                           OrderTag<Tf>) noexcept
    {
        // MSVC does not provide a weak CAS intrinsic for any platform
        Ts success;
        Tf fail;
        return CompareExchangeStrong(storage, val, expected, success, fail);
    }

    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             ReleaseTag,
                                             ReleaseTag) noexcept
    {
        ReleaseTag success;
        RelaxedTag fail;
        return CompareExchangeStrong(storage, val, expected, success, fail);
    }

    static inline bool CompareExchangeStrong(
        volatile T& storage, T val, T& expected, AcqRelTag, AcqRelTag) noexcept
    {
        AcqRelTag success;
        AcquireTag fail;
        return CompareExchangeStrong(storage, val, expected, success, fail);
    }

    template <typename Ts, typename Tf>
    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             OrderTag<Ts>,
                                             OrderTag<Tf>) noexcept
    {
        CheckLoadMemoryOrder<Tf>();
        CheckCasMemoryOrdering<Ts, Tf>();
        typename OrderToTag<CombineMemoryOrders(Ts::Order, Tf::Order)>::Type o;
        return CompareExchangeStrong(storage, val, expected, o);
    }

    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             RelaxedTag) noexcept
    {
        return CasRet(expected,
                      ValAs<Type>(expected),
                      RAD_INTRIN_RELAXED(_InterlockedCompareExchange16)(
                          AddrAs<Type>(storage),
                          ValAs<Type>(val),
                          ValAs<Type>(expected)));
    }

    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             AcquireTag) noexcept
    {
        return CasRet(expected,
                      ValAs<Type>(expected),
                      RAD_INTRIN_ACQUIRE(_InterlockedCompareExchange16)(
                          AddrAs<Type>(storage),
                          ValAs<Type>(val),
                          ValAs<Type>(expected)));
    }

    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             ReleaseTag) noexcept
    {
        return CasRet(expected,
                      ValAs<Type>(expected),
                      RAD_INTRIN_RELEASE(_InterlockedCompareExchange16)(
                          AddrAs<Type>(storage),
                          ValAs<Type>(val),
                          ValAs<Type>(expected)));
    }

    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             SeqCstTag) noexcept
    {
        return CasRet(expected,
                      ValAs<Type>(expected),
                      _InterlockedCompareExchange16(AddrAs<Type>(storage),
                                                    ValAs<Type>(val),
                                                    ValAs<Type>(expected)));
    }

    static inline T FetchAdd(volatile T& storage, T val, RelaxedTag) noexcept
    {
        return static_cast<T>(
            RAD_INTRIN_RELAXED(_InterlockedExchangeAdd16)(AddrAs<Type>(storage),
                                                          ValAs<Type>(val)));
    }

    static inline T FetchAdd(volatile T& storage, T val, AcquireTag) noexcept
    {
        return static_cast<T>(
            RAD_INTRIN_ACQUIRE(_InterlockedExchangeAdd16)(AddrAs<Type>(storage),
                                                          ValAs<Type>(val)));
    }

    static inline T FetchAdd(volatile T& storage, T val, ReleaseTag) noexcept
    {
        return static_cast<T>(
            RAD_INTRIN_RELEASE(_InterlockedExchangeAdd16)(AddrAs<Type>(storage),
                                                          ValAs<Type>(val)));
    }

    static inline T FetchAdd(volatile T& storage, T val, SeqCstTag) noexcept
    {
        return static_cast<T>(
            _InterlockedExchangeAdd16(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    template <typename TOrder>
    static inline T FetchSub(volatile T& storage,
                             T val,
                             OrderTag<TOrder>) noexcept
    {
        typename OrderTag<TOrder>::Type order;
        return FetchAdd(storage, Negate(val), order);
    }

    static inline T FetchAnd(volatile T& storage, T val, RelaxedTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELAXED(
            _InterlockedAnd16)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchAnd(volatile T& storage, T val, AcquireTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_ACQUIRE(
            _InterlockedAnd16)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchAnd(volatile T& storage, T val, ReleaseTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELEASE(
            _InterlockedAnd16)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchAnd(volatile T& storage, T val, SeqCstTag) noexcept
    {
        return static_cast<T>(
            _InterlockedAnd16(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchOr(volatile T& storage, T val, RelaxedTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELAXED(
            _InterlockedOr16)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchOr(volatile T& storage, T val, AcquireTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_ACQUIRE(
            _InterlockedOr16)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchOr(volatile T& storage, T val, ReleaseTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELEASE(
            _InterlockedOr16)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchOr(volatile T& storage, T val, SeqCstTag) noexcept
    {
        return static_cast<T>(
            _InterlockedOr16(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchXor(volatile T& storage, T val, RelaxedTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELAXED(
            _InterlockedXor16)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchXor(volatile T& storage, T val, AcquireTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_ACQUIRE(
            _InterlockedXor16)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchXor(volatile T& storage, T val, ReleaseTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELEASE(
            _InterlockedXor16)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchXor(volatile T& storage, T val, SeqCstTag) noexcept
    {
        return static_cast<T>(
            _InterlockedXor16(AddrAs<Type>(storage), ValAs<Type>(val)));
    }
};

template <typename T>
struct SelectIntrinsic<T, 4>
{
    RAD_S_ASSERT(sizeof(T) == 4);
    using Type = long;

    static inline T Load(const volatile T& storage, RelaxedTag) noexcept
    {
        return ValAs<T>(__iso_volatile_load32(AddrAs<int>(storage)));
    }

    template <typename TOrder>
    static inline T Load(const volatile T& storage, OrderTag<TOrder>) noexcept
    {
        CheckLoadMemoryOrder<TOrder>();
        T ret = ValAs<T>(__iso_volatile_load32(AddrAs<int>(storage)));
        RAD_MEM_BARRIER;
        return ret;
    }

    static inline void Store(volatile T& storage, T val, RelaxedTag) noexcept
    {
        __iso_volatile_store32(AddrAs<int>(storage), ValAs<int>(val));
    }

    template <typename TOrder>
    static inline void Store(volatile T& storage,
                             T val,
                             OrderTag<TOrder>) noexcept
    {
        CheckStoreMemoryOrder<TOrder>();
        RAD_MEM_BARRIER;
        __iso_volatile_store32(AddrAs<int>(storage), ValAs<int>(val));
    }

    static inline T Exchange(volatile T& storage, T val, RelaxedTag) noexcept
    {
        return ValAs<T>(RAD_INTRIN_RELAXED(
            _InterlockedExchange)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T Exchange(volatile T& storage, T val, AcquireTag) noexcept
    {
        return ValAs<T>(RAD_INTRIN_ACQUIRE(
            _InterlockedExchange)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T Exchange(volatile T& storage, T val, ReleaseTag) noexcept
    {
        return ValAs<T>(RAD_INTRIN_RELEASE(
            _InterlockedExchange)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T Exchange(volatile T& storage, T val, SeqCstTag) noexcept
    {
        return ValAs<T>(
            _InterlockedExchange(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline bool CasRet(T& expected, Type comparand, Type old) noexcept
    {
        if (old != comparand)
        {
            expected = ValAs<T>(old);
            return false;
        }
        return true;
    }

    template <typename Ts, typename Tf>
    static inline bool CompareExchangeWeak(volatile T& storage,
                                           T val,
                                           T& expected,
                                           OrderTag<Ts>,
                                           OrderTag<Tf>) noexcept
    {
        // MSVC does not provide a weak CAS intrinsic for any platform
        Ts success;
        Tf fail;
        return CompareExchangeStrong(storage, val, expected, success, fail);
    }

    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             ReleaseTag,
                                             ReleaseTag) noexcept
    {
        ReleaseTag success;
        RelaxedTag fail;
        return CompareExchangeStrong(storage, val, expected, success, fail);
    }

    static inline bool CompareExchangeStrong(
        volatile T& storage, T val, T& expected, AcqRelTag, AcqRelTag) noexcept
    {
        AcqRelTag success;
        AcquireTag fail;
        return CompareExchangeStrong(storage, val, expected, success, fail);
    }

    template <typename Ts, typename Tf>
    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             OrderTag<Ts>,
                                             OrderTag<Tf>) noexcept
    {
        CheckLoadMemoryOrder<Tf>();
        CheckCasMemoryOrdering<Ts, Tf>();
        typename OrderToTag<CombineMemoryOrders(Ts::Order, Tf::Order)>::Type o;
        return CompareExchangeStrong(storage, val, expected, o);
    }

    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             RelaxedTag) noexcept
    {
        return CasRet(expected,
                      ValAs<Type>(expected),
                      RAD_INTRIN_RELAXED(_InterlockedCompareExchange)(
                          AddrAs<Type>(storage),
                          ValAs<Type>(val),
                          ValAs<Type>(expected)));
    }

    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             AcquireTag) noexcept
    {
        return CasRet(expected,
                      ValAs<Type>(expected),
                      RAD_INTRIN_ACQUIRE(_InterlockedCompareExchange)(
                          AddrAs<Type>(storage),
                          ValAs<Type>(val),
                          ValAs<Type>(expected)));
    }

    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             ReleaseTag) noexcept
    {
        return CasRet(expected,
                      ValAs<Type>(expected),
                      RAD_INTRIN_RELEASE(_InterlockedCompareExchange)(
                          AddrAs<Type>(storage),
                          ValAs<Type>(val),
                          ValAs<Type>(expected)));
    }

    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             SeqCstTag) noexcept
    {
        return CasRet(expected,
                      ValAs<Type>(expected),
                      _InterlockedCompareExchange(AddrAs<Type>(storage),
                                                  ValAs<Type>(val),
                                                  ValAs<Type>(expected)));
    }

    template <typename U = T, EnIf<IsIntegral<U>, int> = 0>
    static inline T FetchAdd(volatile T& storage, T val, RelaxedTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELAXED(
            _InterlockedExchangeAdd)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    template <typename U = T, EnIf<IsPointer<U>, int> = 0>
    static inline T FetchAdd(volatile T& storage,
                             ptrdiff_t val,
                             RelaxedTag) noexcept
    {
        return reinterpret_cast<T>(RAD_INTRIN_RELAXED(
            _InterlockedExchangeAdd)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    template <typename U = T, EnIf<IsIntegral<U>, int> = 0>
    static inline T FetchAdd(volatile T& storage, T val, AcquireTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_ACQUIRE(
            _InterlockedExchangeAdd)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    template <typename U = T, EnIf<IsPointer<U>, int> = 0>
    static inline T FetchAdd(volatile T& storage,
                             ptrdiff_t val,
                             AcquireTag) noexcept
    {
        return reinterpret_cast<T>(RAD_INTRIN_ACQUIRE(
            _InterlockedExchangeAdd)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    template <typename U = T, EnIf<IsIntegral<U>, int> = 0>
    static inline T FetchAdd(volatile T& storage, T val, ReleaseTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELEASE(
            _InterlockedExchangeAdd)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    template <typename U = T, EnIf<IsPointer<U>, int> = 0>
    static inline T FetchAdd(volatile T& storage,
                             ptrdiff_t val,
                             ReleaseTag) noexcept
    {
        return reinterpret_cast<T>(RAD_INTRIN_RELEASE(
            _InterlockedExchangeAdd)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    template <typename U = T, EnIf<IsIntegral<U>, int> = 0>
    static inline T FetchAdd(volatile T& storage, T val, SeqCstTag) noexcept
    {
        return static_cast<T>(
            _InterlockedExchangeAdd(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    template <typename U = T, EnIf<IsPointer<U>, int> = 0>
    static inline T FetchAdd(volatile T& storage,
                             ptrdiff_t val,
                             SeqCstTag) noexcept
    {
        return reinterpret_cast<T>(
            _InterlockedExchangeAdd(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    template <typename TOrder, typename U = T, EnIf<IsIntegral<U>, int> = 0>
    static inline T FetchSub(volatile T& storage,
                             T val,
                             OrderTag<TOrder>) noexcept
    {
        typename OrderTag<TOrder>::Type order;
        return FetchAdd(storage, Negate(val), order);
    }

    template <typename TOrder, typename U = T, EnIf<IsPointer<U>, int> = 0>
    static inline T FetchSub(volatile T& storage,
                             ptrdiff_t val,
                             OrderTag<TOrder>) noexcept
    {
        typename OrderTag<TOrder>::Type order;
        return FetchAdd(storage, Negate(val), order);
    }

    static inline T FetchAnd(volatile T& storage, T val, RelaxedTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELAXED(
            _InterlockedAnd)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchAnd(volatile T& storage, T val, AcquireTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_ACQUIRE(
            _InterlockedAnd)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchAnd(volatile T& storage, T val, ReleaseTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELEASE(
            _InterlockedAnd)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchAnd(volatile T& storage, T val, SeqCstTag) noexcept
    {
        return static_cast<T>(
            _InterlockedAnd(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchOr(volatile T& storage, T val, RelaxedTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELAXED(
            _InterlockedOr)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchOr(volatile T& storage, T val, AcquireTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_ACQUIRE(
            _InterlockedOr)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchOr(volatile T& storage, T val, ReleaseTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELEASE(
            _InterlockedOr)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchOr(volatile T& storage, T val, SeqCstTag) noexcept
    {
        return static_cast<T>(
            _InterlockedOr(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchXor(volatile T& storage, T val, RelaxedTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELAXED(
            _InterlockedXor)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchXor(volatile T& storage, T val, AcquireTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_ACQUIRE(
            _InterlockedXor)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchXor(volatile T& storage, T val, ReleaseTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELEASE(
            _InterlockedXor)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchXor(volatile T& storage, T val, SeqCstTag) noexcept
    {
        return static_cast<T>(
            _InterlockedXor(AddrAs<Type>(storage), ValAs<Type>(val)));
    }
};

template <typename T>
struct SelectIntrinsic<T, 8>
{
    RAD_S_ASSERT(sizeof(T) == 8);
    using Type = __int64;

    // Load and Store operations for 64-bit integrals on x86 (32-bit) will
    // produce fild and flstp instructions for atomicity when compiling with
    // /kernel. This is due to /kernel forcing IA86 and not allowing override
    // with SSE support. This, unfortunately, can cause alignment issues at
    // DISPATCH_LEVEL and bug check.
    //
    // The workaround for this is to use slower atomic exchange and cas.
    // This will unfortunately result in all loads/stores on x86 enforcing
    // sequential ordering with the associated performance impact.

    static inline T Load(const volatile T& storage, RelaxedTag) noexcept
    {
#if RAD_I386 && RAD_KERNEL_MODE
        return InterlockedCompareExchange64((volatile LONG64*)&storage, 0, 0);
#else
        return ValAs<T>(__iso_volatile_load64(AddrAs<Type>(storage)));
#endif
    }

    template <typename TOrder>
    static inline T Load(const volatile T& storage, OrderTag<TOrder>) noexcept
    {
        CheckLoadMemoryOrder<TOrder>();
#if RAD_I386 && RAD_KERNEL_MODE
        return InterlockedCompareExchange64((volatile LONG64*)&storage, 0, 0);
#else
        T ret = ValAs<T>(__iso_volatile_load64(AddrAs<Type>(storage)));
        RAD_MEM_BARRIER;
        return ret;
#endif
    }

    static inline void Store(volatile T& storage, T val, RelaxedTag) noexcept
    {
#if RAD_I386 && RAD_KERNEL_MODE
        InterlockedExchange64(AddrAs<Type>(storage), val);
#else
        __iso_volatile_store64(AddrAs<Type>(storage), ValAs<Type>(val));
#endif
    }

    template <typename TOrder>
    static inline void Store(volatile T& storage,
                             T val,
                             OrderTag<TOrder>) noexcept
    {
        CheckStoreMemoryOrder<TOrder>();
#if RAD_I386 && RAD_KERNEL_MODE
        InterlockedExchange64(AddrAs<Type>(storage), val);
#else
        RAD_MEM_BARRIER;
        __iso_volatile_store64(AddrAs<Type>(storage), ValAs<Type>(val));
#endif
    }

    static inline T Exchange(volatile T& storage, T val, RelaxedTag) noexcept
    {
        return ValAs<T>(RAD_INTRIN_RELAXED(
            InterlockedExchange64)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T Exchange(volatile T& storage, T val, AcquireTag) noexcept
    {
        return ValAs<T>(RAD_INTRIN_ACQUIRE(
            InterlockedExchange64)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T Exchange(volatile T& storage, T val, ReleaseTag) noexcept
    {
        return ValAs<T>(RAD_INTRIN_RELEASE(
            InterlockedExchange64)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T Exchange(volatile T& storage, T val, SeqCstTag) noexcept
    {
        return ValAs<T>(
            InterlockedExchange64(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline bool CasRet(T& expected, Type comparand, Type old) noexcept
    {
        if (old != comparand)
        {
            expected = ValAs<T>(old);
            return false;
        }
        return true;
    }

    template <typename Ts, typename Tf>
    static inline bool CompareExchangeWeak(volatile T& storage,
                                           T val,
                                           T& expected,
                                           OrderTag<Ts>,
                                           OrderTag<Tf>) noexcept
    {
        // MSVC does not provide a weak CAS intrinsic for any platform
        Ts success;
        Tf fail;
        return CompareExchangeStrong(storage, val, expected, success, fail);
    }

    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             ReleaseTag,
                                             ReleaseTag) noexcept
    {
        ReleaseTag success;
        RelaxedTag fail;
        return CompareExchangeStrong(storage, val, expected, success, fail);
    }

    static inline bool CompareExchangeStrong(
        volatile T& storage, T val, T& expected, AcqRelTag, AcqRelTag) noexcept
    {
        AcqRelTag success;
        AcquireTag fail;
        return CompareExchangeStrong(storage, val, expected, success, fail);
    }

    template <typename Ts, typename Tf>
    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             OrderTag<Ts>,
                                             OrderTag<Tf>) noexcept
    {
        CheckLoadMemoryOrder<Tf>();
        CheckCasMemoryOrdering<Ts, Tf>();
        typename OrderToTag<CombineMemoryOrders(Ts::Order, Tf::Order)>::Type o;
        return CompareExchangeStrong(storage, val, expected, o);
    }

    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             RelaxedTag) noexcept
    {
        return CasRet(expected,
                      ValAs<Type>(expected),
                      RAD_INTRIN_RELAXED(_InterlockedCompareExchange64)(
                          AddrAs<Type>(storage),
                          ValAs<Type>(val),
                          ValAs<Type>(expected)));
    }

    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             AcquireTag) noexcept
    {
        return CasRet(expected,
                      ValAs<Type>(expected),
                      RAD_INTRIN_ACQUIRE(_InterlockedCompareExchange64)(
                          AddrAs<Type>(storage),
                          ValAs<Type>(val),
                          ValAs<Type>(expected)));
    }

    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             ReleaseTag) noexcept
    {
        return CasRet(expected,
                      ValAs<Type>(expected),
                      RAD_INTRIN_RELEASE(_InterlockedCompareExchange64)(
                          AddrAs<Type>(storage),
                          ValAs<Type>(val),
                          ValAs<Type>(expected)));
    }

    static inline bool CompareExchangeStrong(volatile T& storage,
                                             T val,
                                             T& expected,
                                             SeqCstTag) noexcept
    {
        return CasRet(expected,
                      ValAs<Type>(expected),
                      _InterlockedCompareExchange64(AddrAs<Type>(storage),
                                                    ValAs<Type>(val),
                                                    ValAs<Type>(expected)));
    }

    template <typename U = T, EnIf<IsIntegral<U>, int> = 0>
    static inline T FetchAdd(volatile T& storage, T val, RelaxedTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELAXED(
            InterlockedExchangeAdd64)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    template <typename U = T, EnIf<IsPointer<U>, int> = 0>
    static inline T FetchAdd(volatile T& storage,
                             ptrdiff_t val,
                             RelaxedTag) noexcept
    {
        return reinterpret_cast<T>(RAD_INTRIN_RELAXED(
            InterlockedExchangeAdd64)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    template <typename U = T, EnIf<IsIntegral<U>, int> = 0>
    static inline T FetchAdd(volatile T& storage, T val, AcquireTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_ACQUIRE(
            InterlockedExchangeAdd64)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    template <typename U = T, EnIf<IsPointer<U>, int> = 0>
    static inline T FetchAdd(volatile T& storage,
                             ptrdiff_t val,
                             AcquireTag) noexcept
    {
        return reinterpret_cast<T>(RAD_INTRIN_ACQUIRE(
            InterlockedExchangeAdd64)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    template <typename U = T, EnIf<IsIntegral<U>, int> = 0>
    static inline T FetchAdd(volatile T& storage, T val, ReleaseTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELEASE(
            InterlockedExchangeAdd64)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    template <typename U = T, EnIf<IsPointer<U>, int> = 0>
    static inline T FetchAdd(volatile T& storage,
                             ptrdiff_t val,
                             ReleaseTag) noexcept
    {
        return reinterpret_cast<T>(RAD_INTRIN_RELEASE(
            InterlockedExchangeAdd64)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    template <typename U = T, EnIf<IsIntegral<U>, int> = 0>
    static inline T FetchAdd(volatile T& storage, T val, SeqCstTag) noexcept
    {
        return static_cast<T>(
            InterlockedExchangeAdd64(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    template <typename U = T, EnIf<IsPointer<U>, int> = 0>
    static inline T FetchAdd(volatile T& storage,
                             ptrdiff_t val,
                             SeqCstTag) noexcept
    {
        return reinterpret_cast<T>(
            InterlockedExchangeAdd64(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    template <typename TOrder, typename U = T, EnIf<IsIntegral<U>, int> = 0>
    static inline T FetchSub(volatile T& storage,
                             T val,
                             OrderTag<TOrder>) noexcept
    {
        typename OrderTag<TOrder>::Type order;
        return FetchAdd(storage, Negate(val), order);
    }

    template <typename TOrder, typename U = T, EnIf<IsPointer<U>, int> = 0>
    static inline T FetchSub(volatile T& storage,
                             ptrdiff_t val,
                             OrderTag<TOrder>) noexcept
    {
        typename OrderTag<TOrder>::Type order;
        return FetchAdd(storage, Negate(val), order);
    }

    static inline T FetchAnd(volatile T& storage, T val, RelaxedTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELAXED(
            InterlockedAnd64)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchAnd(volatile T& storage, T val, AcquireTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_ACQUIRE(
            InterlockedAnd64)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchAnd(volatile T& storage, T val, ReleaseTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELEASE(
            InterlockedAnd64)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchAnd(volatile T& storage, T val, SeqCstTag) noexcept
    {
        return static_cast<T>(
            InterlockedAnd64(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchOr(volatile T& storage, T val, RelaxedTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELAXED(
            InterlockedOr64)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchOr(volatile T& storage, T val, AcquireTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_ACQUIRE(
            InterlockedOr64)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchOr(volatile T& storage, T val, ReleaseTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELEASE(
            InterlockedOr64)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchOr(volatile T& storage, T val, SeqCstTag) noexcept
    {
        return static_cast<T>(
            InterlockedOr64(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchXor(volatile T& storage, T val, RelaxedTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELAXED(
            InterlockedXor64)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchXor(volatile T& storage, T val, AcquireTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_ACQUIRE(
            InterlockedXor64)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchXor(volatile T& storage, T val, ReleaseTag) noexcept
    {
        return static_cast<T>(RAD_INTRIN_RELEASE(
            InterlockedXor64)(AddrAs<Type>(storage), ValAs<Type>(val)));
    }

    static inline T FetchXor(volatile T& storage, T val, SeqCstTag) noexcept
    {
        return static_cast<T>(
            InterlockedXor64(AddrAs<Type>(storage), ValAs<Type>(val)));
    }
};
