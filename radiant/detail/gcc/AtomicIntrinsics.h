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

template <typename T, size_t Size = sizeof(T)>
struct SelectIntrinsic
{
    RAD_S_ASSERTMSG(
        (IsIntegral<T> && sizeof(T) == Size &&
         (Size == 1 || Size == 2 || Size == 4 || Size == 8)) ||
            (IsPointer<T> && Size == sizeof(void*)),
        "rad::detail::atomic::SelectIntrinsic not supported for this type");

    template <typename TOrder>
    static inline T Load(const volatile T& storage, OrderTag<TOrder>) noexcept
    {
        CheckLoadMemoryOrder<TOrder>();
        constexpr int order = static_cast<int>(TOrder::Order);
        return __atomic_load_n(&storage, order);
    }

    template <typename TOrder>
    static inline void Store(volatile T& storage,
                             T val,
                             OrderTag<TOrder>) noexcept
    {
        CheckStoreMemoryOrder<TOrder>();
        constexpr int order = static_cast<int>(TOrder::Order);
        __atomic_store_n(&storage, val, order);
    }

    template <typename TOrder>
    static inline T Exchange(volatile T& storage,
                             T val,
                             OrderTag<TOrder>) noexcept
    {
        constexpr int order = static_cast<int>(TOrder::Order);
        return __atomic_exchange_n(&storage, val, order);
    }

    static inline bool CompareExchangeWeak(volatile T& storage,
                                           T val,
                                           T& expected,
                                           ReleaseTag,
                                           ReleaseTag) noexcept
    {
        ReleaseTag success;
        RelaxedTag fail;
        return CompareExchangeWeak(storage, val, expected, success, fail);
    }

    static inline bool CompareExchangeWeak(
        volatile T& storage, T val, T& expected, AcqRelTag, AcqRelTag) noexcept
    {
        AcqRelTag success;
        AcquireTag fail;
        return CompareExchangeWeak(storage, val, expected, success, fail);
    }

    template <typename Ts, typename Tf>
    static inline bool CompareExchangeWeak(volatile T& storage,
                                           T val,
                                           T& expected,
                                           OrderTag<Ts>,
                                           OrderTag<Tf>) noexcept
    {
        CheckLoadMemoryOrder<Tf>();
        CheckCasMemoryOrdering<Ts, Tf>();
        constexpr int success = static_cast<int>(Ts::Order);
        constexpr int fail = static_cast<int>(Tf::Order);
        return __atomic_compare_exchange_n(&storage,
                                           &expected,
                                           val,
                                           true,
                                           success,
                                           fail);
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
        constexpr int success = static_cast<int>(Ts::Order);
        constexpr int fail = static_cast<int>(Tf::Order);
        return __atomic_compare_exchange_n(&storage,
                                           &expected,
                                           val,
                                           false,
                                           success,
                                           fail);
    }

    template <typename TOrder, typename U = T, EnIf<IsIntegral<U>, int> = 0>
    static inline T FetchAdd(volatile T& storage,
                             T val,
                             OrderTag<TOrder>) noexcept
    {
        constexpr int order = static_cast<int>(TOrder::Order);
        return __atomic_fetch_add(&storage, val, order);
    }

    template <typename TOrder, typename U = T, EnIf<IsPointer<U>, int> = 0>
    static inline T FetchAdd(volatile T& storage,
                             ptrdiff_t val,
                             OrderTag<TOrder>) noexcept
    {
        constexpr int order = static_cast<int>(TOrder::Order);
        return __atomic_fetch_add(&storage, val, order);
    }

    template <typename TOrder, typename U = T, EnIf<IsIntegral<U>, int> = 0>
    static inline T FetchSub(volatile T& storage,
                             T val,
                             OrderTag<TOrder>) noexcept
    {
        constexpr int order = static_cast<int>(TOrder::Order);
        return __atomic_fetch_sub(&storage, val, order);
    }

    template <typename TOrder, typename U = T, EnIf<IsPointer<U>, int> = 0>
    static inline T FetchSub(volatile T& storage,
                             ptrdiff_t val,
                             OrderTag<TOrder>) noexcept
    {
        constexpr int order = static_cast<int>(TOrder::Order);
        return __atomic_fetch_sub(&storage, val, order);
    }

    template <typename TOrder>
    static inline T FetchAnd(volatile T& storage,
                             T val,
                             OrderTag<TOrder>) noexcept
    {
        constexpr int order = static_cast<int>(TOrder::Order);
        return __atomic_fetch_and(&storage, val, order);
    }

    template <typename TOrder>
    static inline T FetchOr(volatile T& storage,
                            T val,
                            OrderTag<TOrder>) noexcept
    {
        constexpr int order = static_cast<int>(TOrder::Order);
        return __atomic_fetch_or(&storage, val, order);
    }

    template <typename TOrder>
    static inline T FetchXor(volatile T& storage,
                             T val,
                             OrderTag<TOrder>) noexcept
    {
        constexpr int order = static_cast<int>(TOrder::Order);
        return __atomic_fetch_xor(&storage, val, order);
    }
};
