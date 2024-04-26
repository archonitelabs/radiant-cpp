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

#if RAD_WINDOWS && RAD_KERNEL_MODE
#include <ntddk.h>
#endif

namespace rad
{
enum class MemoryOrder : int
{
    Relaxed,
    Consume,
    Acquire,
    Release,
    AcqRel,
    SeqCst
};

namespace detail
{
namespace atomic
{

class LockRegion
{
public:

    LockRegion() noexcept
    {
#if RAD_WINDOWS && RAD_KERNEL_MODE
        region = KeGetCurrentIrql() <= APC_LEVEL;
        if (region)
        {
            KeEnterCriticalRegion();
        }
#endif
    }

    ~LockRegion()
    {
#if RAD_WINDOWS && RAD_KERNEL_MODE
        if (region)
        {
            KeLeaveCriticalRegion();
        }
#endif
    }

#if RAD_WINDOWS && RAD_KERNEL_MODE
private:

    bool region;
#endif

    RAD_NOT_COPYABLE(LockRegion);
};

template <typename T>
struct OrderTag
{
    using Type = T;
};

struct RelaxedTag : public OrderTag<RelaxedTag>
{
    static constexpr MemoryOrder Order = MemoryOrder::Relaxed;
};

struct ConsumeTag : public OrderTag<ConsumeTag>
{
    static constexpr MemoryOrder Order = MemoryOrder::Consume;
};

struct AcquireTag : public OrderTag<AcquireTag>
{
    static constexpr MemoryOrder Order = MemoryOrder::Acquire;

#ifdef RAD_MSC_VERSION
    AcquireTag() = default;

    AcquireTag(ConsumeTag)
    {
    }
#endif
};

struct ReleaseTag : public OrderTag<ReleaseTag>
{
    static constexpr MemoryOrder Order = MemoryOrder::Release;
};

struct AcqRelTag : public OrderTag<AcqRelTag>
{
    static constexpr MemoryOrder Order = MemoryOrder::AcqRel;
};

struct SeqCstTag : public OrderTag<SeqCstTag>
{
    static constexpr MemoryOrder Order = MemoryOrder::SeqCst;

#ifdef RAD_MSC_VERSION
    SeqCstTag() = default;

    SeqCstTag(AcqRelTag)
    {
    }
#endif
};

template <MemoryOrder Order>
struct OrderToTag
{
};

template <>
struct OrderToTag<MemoryOrder::Relaxed>
{
    using Type = RelaxedTag;
};

template <>
struct OrderToTag<MemoryOrder::Consume>
{
    using Type = ConsumeTag;
};

template <>
struct OrderToTag<MemoryOrder::Acquire>
{
    using Type = AcquireTag;
};

template <>
struct OrderToTag<MemoryOrder::Release>
{
    using Type = ReleaseTag;
};

template <>
struct OrderToTag<MemoryOrder::AcqRel>
{
    using Type = AcqRelTag;
};

template <>
struct OrderToTag<MemoryOrder::SeqCst>
{
    using Type = SeqCstTag;
};

template <typename Order>
constexpr inline void CheckLoadMemoryOrder()
{
    RAD_S_ASSERTMSG((IsSame<Order, RelaxedTag> || //
                     IsSame<Order, ConsumeTag> || //
                     IsSame<Order, AcquireTag> || //
                     IsSame<Order, SeqCstTag>),   //
                    "Invalid memory order for atomic load operation");
}

template <typename Success, typename Failure>
constexpr inline void CheckCasMemoryOrdering()
{
    constexpr int success = static_cast<int>(Success::Order);
    constexpr int fail = static_cast<int>(Failure::Order);
    RAD_S_ASSERTMSG(success >= fail,
                    "Invalid memory order for atomic load operation");
}

template <typename Order>
constexpr inline void CheckStoreMemoryOrder()
{
    RAD_S_ASSERTMSG((IsSame<Order, RelaxedTag> || //
                     IsSame<Order, ReleaseTag> || //
                     IsSame<Order, SeqCstTag>),   //
                    "Invalid memory order for atomic store operation");
}

template <typename R, typename T>
volatile R* AddrAs(T& val) noexcept
{
    return reinterpret_cast<volatile R*>(AddrOf(val));
}

template <typename R, typename T>
const volatile R* AddrAs(const T& val) noexcept
{
    return reinterpret_cast<const volatile R*>(AddrOf(val));
}

template <typename R, typename T, EnIf<IsIntegral<R> && IsIntegral<T>, int> = 0>
R ValAs(T val) noexcept
{
    return static_cast<R>(val);
}

template <typename R, typename T, EnIf<IsPointer<R> || IsPointer<T>, int> = 0>
R ValAs(T val) noexcept
{
    return reinterpret_cast<R>(val);
}

#ifdef RAD_MSC_VERSION
#include "radiant/detail/msvc/AtomicIntrinsics.h"
#elif defined(RAD_GCC_VERSION) || defined(RAD_CLANG_VERSION)
#include "radiant/detail/gcc/AtomicIntrinsics.h"
#endif

} // namespace atomic
} // namespace detail
} // namespace rad
