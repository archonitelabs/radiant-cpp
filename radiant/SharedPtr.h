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

#include "radiant/Memory.h"
#include "radiant/Atomic.h"
#include "radiant/Locks.h"
#include "radiant/EmptyOptimizedPair.h"

namespace rad
{

namespace detail
{

/// @brief Internal use only. Reference counting management for smart pointers.
/// @tparam Atomic counter type
template <typename TAtomic>
class TPtrRefCount final
{
public:

    TPtrRefCount() noexcept
        : m_strongCount(1),
          m_weakCount(1)
    {
    }

    void Increment() const noexcept
    {
        m_strongCount.FetchAdd(1, rad::MemOrderRelaxed);
    }

    bool Decrement() const noexcept
    {
        return m_strongCount.FetchSub(1, rad::MemOrderAcqRel) == 1;
    }

    void IncrementWeak() const noexcept
    {
        m_weakCount.FetchAdd(1, rad::MemOrderRelaxed);
    }

    bool DecrementWeak() const noexcept
    {
        return m_weakCount.FetchSub(1, rad::MemOrderAcqRel) == 1;
    }

    bool LockWeak() const noexcept
    {
        uint32_t count = m_strongCount.Load(rad::MemOrderRelaxed);
        if (count == 0)
        {
            return false;
        }

        atomic::LockRegion lockRegion;
        do
        {
            if RAD_LIKELY (m_strongCount.CompareExchangeWeak(
                               count,
                               count + 1,
                               rad::MemOrderAcqRel,
                               rad::MemOrderRelaxed))
            {
                return true;
            }
        }
        while (count != 0);

        return false;
    }

    uint32_t StrongCount() const
    {
        return m_strongCount.Load(rad::MemOrderRelaxed);
    }

    uint32_t WeakCount() const
    {
        return m_weakCount.Load(rad::MemOrderRelaxed);
    }

private:

    mutable TAtomic m_strongCount;
    mutable TAtomic m_weakCount;
};

using PtrRefCount = TPtrRefCount<Atomic<uint32_t>>;

class PtrBlockBase
{
public:

    virtual ~PtrBlockBase() noexcept
    {
    }

    PtrBlockBase() noexcept = default;

    RAD_NOT_COPYABLE(PtrBlockBase);

    virtual void OnRefZero() const noexcept = 0;
    virtual void OnWeakZero() const noexcept = 0;

    const PtrRefCount& RefCount() const noexcept
    {
        return m_refcount;
    }

    void Acquire() const noexcept
    {
        RefCount().Increment();
    }

    void AcquireWeak() const noexcept
    {
        RefCount().IncrementWeak();
    }

    bool LockWeak() const noexcept
    {
        return RefCount().LockWeak();
    }

    void Release() const noexcept
    {
        if (RefCount().Decrement())
        {
            OnRefZero();
            ReleaseWeak();
        }
    }

    void ReleaseWeak() const noexcept
    {
        if (RefCount().DecrementWeak())
        {
            OnWeakZero();
        }
    }

    uint32_t UseCount() const
    {
        return RefCount().StrongCount();
    }

    uint32_t WeakCount() const
    {
        return RefCount().WeakCount();
    }

private:

    PtrRefCount m_refcount;
};

/// @brief Internal use only. SharedPtr control block
/// @tparam T Value type
/// @tparam TAlloc Allocator type
template <typename T, typename TAlloc>
class PtrBlock final : public PtrBlockBase
{
public:

    using AllocatorType = typename TAlloc::template Rebind<PtrBlock>::Other;
    using ValueType = T;
    using PairType = EmptyOptimizedPair<AllocatorType, ValueType>;

    RAD_S_ASSERT_ALLOCATOR_REQUIRES_T(TAlloc);

    template <typename... TArgs>
    PtrBlock(const AllocatorType& alloc, TArgs&&... args) noexcept(
        noexcept(PairType(alloc, Forward<TArgs>(args)...)))
        : PtrBlockBase(),
          m_pair(alloc, Forward<TArgs>(args)...)
    {
    }

    void OnRefZero() const noexcept override
    {
        //
        // The strong reference count has dropped to zero, destruct the managed
        // object now. Once the weak reference count drops to zero the rest of
        // the block will be destructed and freed.
        //
        Value().~ValueType();
    }

    void OnWeakZero() const noexcept override
    {
        //
        // Take a copy of the allocator first, this will be used to do the free.
        // Then destruct the remaining parts of the block.
        //
        // N.B. The PtrBlockBase and PairType destructors are not invoked
        // directly because the base contains only atomics which do not need to
        // be destructed and the PairType is destructed in parts. Perhaps there
        // is a more correct way to handle this, but this seems to account for
        // destructing the necessary parts as it is.
        //
        AllocatorType alloc(Allocator());
        auto self = const_cast<PtrBlock*>(this);
        Allocator().~AllocatorType();
        alloc.Free(self);
    }

    AllocatorType& Allocator() noexcept
    {
        return m_pair.First();
    }

    const AllocatorType& Allocator() const noexcept
    {
        return m_pair.First();
    }

    ValueType& Value() noexcept
    {
        return m_pair.Second();
    }

    const ValueType& Value() const noexcept
    {
        return m_pair.Second();
    }

private:

    mutable PairType m_pair;
};

struct AllocateSharedImpl;

} // namespace detail

template <typename T>
class WeakPtr;

template <typename T>
class AtomicSharedPtr;

template <typename T>
class AtomicWeakPtr;

/// @brief Smart pointer implementing shared ownership mechanics.
/// @tparam T Value type to point to
template <typename T>
class SharedPtr final
{

public:

    using ValueType = T;

    ~SharedPtr()
    {
        Reset();
    }

    /// @brief Construct empty SharedPtr (nullptr)
    SharedPtr() noexcept
        : m_block(),
          m_ptr()
    {
    }

    /// @brief Construct empty SharedPtr (nullptr)
    SharedPtr(rad::nullptr_t) noexcept
        : SharedPtr()
    {
    }

    /// @brief Construct a new SharedPtr taking an additional reference to an
    /// existing pointer.
    /// @param r Existing pointer
    SharedPtr(const SharedPtr& r) noexcept
        : m_block(r.m_block),
          m_ptr(r.m_ptr)
    {
        RAD_S_ASSERT(noexcept(m_block->Acquire()));

        if (m_block)
        {
            m_block->Acquire();
        }
    }

    /// @brief Construct a new SharedPtr by moving an existing SharedPtr.
    /// @param r Existing pointer
    SharedPtr(SharedPtr&& r) noexcept
        : m_block(r.m_block),
          m_ptr(r.m_ptr)
    {
        r.m_block = nullptr;
        r.m_ptr = nullptr;
    }

    /// @brief Construct a new SharedPtr from a convertible pointer.
    /// @param r Existing pointer
    template <typename U, EnIf<IsConv<U*, T*>, int> = 0>
    SharedPtr(const SharedPtr<U>& r) noexcept
        : m_block(r.m_block),
          m_ptr(r.m_ptr)
    {
        RAD_S_ASSERT(noexcept(m_block->Acquire()));

        if (m_block)
        {
            m_block->Acquire();
        }
    }

    /// @brief bool operator returning true if the pointer is not nullptr.
    operator bool() const noexcept
    {
        return m_ptr != nullptr;
    }

    /// @brief Retrieve a pointer to the stored object
    ValueType* Get() const noexcept
    {
        return m_ptr;
    }

    /// @brief Check if the pointer is not nullptr
    bool operator!=(rad::nullptr_t) const noexcept
    {
        return *this;
    }

    /// @brief Check if the pointer is nullptr
    bool operator==(rad::nullptr_t) const noexcept
    {
        return !*this;
    }

    /// @brief Dereference the pointer
    T& operator*() noexcept
    {
        RAD_ASSERT(m_ptr != nullptr);
        return *Get();
    }

    /// @brief Dereference the pointer
    const T& operator*() const noexcept
    {
        RAD_ASSERT(m_ptr != nullptr);
        return *Get();
    }

    /// @brief Dereference the pointer
    T* operator->() noexcept
    {
        RAD_ASSERT(m_ptr != nullptr);
        return Get();
    }

    /// @brief Dereference the pointer
    const T* operator->() const noexcept
    {
        RAD_ASSERT(m_ptr != nullptr);
        return Get();
    }

    /// @brief Take an additional reference to an existing pointer. Drops
    /// reference to its current stored pointer if not nullptr.
    SharedPtr& operator=(const SharedPtr& r) noexcept
    {
        RAD_S_ASSERT(noexcept(m_block->Release()));
        RAD_S_ASSERT(noexcept(m_block->Acquire()));

        if RAD_LIKELY (&r != this)
        {
            if (m_block)
            {
                m_block->Release();
            }

            m_block = r.m_block;
            m_ptr = r.m_ptr;

            if (m_block)
            {
                m_block->Acquire();
            }
        }

        return *this;
    }

    /// @brief Take ownership of an existing pointer by move semantics. Drops
    /// reference to its current stored pointer if not nullptr.
    SharedPtr& operator=(SharedPtr&& r) noexcept
    {
        RAD_S_ASSERT(noexcept(m_block->Release()));

        if RAD_LIKELY (&r != this)
        {
            if (m_block)
            {
                m_block->Release();
            }

            m_block = r.m_block;
            m_ptr = r.m_ptr;
            r.m_block = nullptr;
            r.m_ptr = nullptr;
        }

        return *this;
    }

    /// @brief Take additional reference to an existing, convertible pointer.
    /// Drops reference to its current stored pointer if not nullptr.
    template <typename U, EnIf<IsConv<U*, T*>, int> = 0>
    SharedPtr& operator=(const SharedPtr<U>& r) noexcept
    {
        RAD_S_ASSERT(noexcept(m_block->Release()));
        RAD_S_ASSERT(noexcept(m_block->Acquire()));

        if (m_block)
        {
            m_block->Release();
        }

        m_block = r.m_block;
        m_ptr = r.m_ptr;

        if (m_block)
        {
            m_block->Acquire();
        }

        return *this;
    }

    /// @brief Drop existing reference if a reference is held.
    void Reset() noexcept
    {
        RAD_S_ASSERT(noexcept(m_block->Release()));

        if (m_block)
        {
            m_block->Release();
            m_block = nullptr;
            m_ptr = nullptr;
        }
    }

    /// @brief Swaps the managed objects.
    /// @param o Other shared ptr to swap with this.
    void Swap(SharedPtr& o) noexcept
    {
        if RAD_LIKELY (&o != this)
        {
            auto block = m_block;
            auto ptr = m_ptr;
            m_block = o.m_block;
            m_ptr = o.m_ptr;
            o.m_block = block;
            o.m_ptr = ptr;
        }
    }

    /// @brief Current refcount (this is inherently not thread-safe and only
    /// exposed for testing)
    uint32_t UseCount() const noexcept
    {
        return m_block ? m_block->UseCount() : 0;
    }

    /// @brief Current WeakPtr refcount (this is inherently not thread-safe and
    /// only exposed for testing)
    uint32_t WeakCount() const noexcept
    {
        return m_block ? m_block->WeakCount() : 0;
    }

private:

    SharedPtr(detail::PtrBlockBase* block, T* ptr) noexcept
        : m_block(block),
          m_ptr(ptr)
    {
    }

    template <typename U>
    friend class SharedPtr;

    template <typename U>
    friend class WeakPtr;

    friend class AtomicSharedPtr<T>;
    friend class AtomicWeakPtr<T>;

    detail::PtrBlockBase* m_block;
    T* m_ptr;

    friend struct detail::AllocateSharedImpl;
};

template <typename T>
RAD_NODISCARD bool operator==(rad::nullptr_t, const SharedPtr<T>& sp) noexcept
{
    return sp == nullptr;
}

template <typename T>
RAD_NODISCARD bool operator!=(rad::nullptr_t, const SharedPtr<T>& sp) noexcept
{
    return sp != nullptr;
}

template <typename T, typename U>
RAD_NODISCARD bool operator==(const SharedPtr<T>& l,
                              const SharedPtr<U>& r) noexcept
{
    return l.Get() == r.Get();
}

template <typename T, typename U>
RAD_NODISCARD bool operator!=(const SharedPtr<T>& l,
                              const SharedPtr<U>& r) noexcept
{
    return l.Get() != r.Get();
}

template <typename T, typename U>
RAD_NODISCARD bool operator<(const SharedPtr<T>& l,
                             const SharedPtr<U>& r) noexcept
{
    return l.Get() < r.Get();
}

template <typename T, typename U>
RAD_NODISCARD bool operator<=(const SharedPtr<T>& l,
                              const SharedPtr<U>& r) noexcept
{
    return l.Get() <= r.Get();
}

template <typename T, typename U>
RAD_NODISCARD bool operator>(const SharedPtr<T>& l,
                             const SharedPtr<U>& r) noexcept
{
    return l.Get() > r.Get();
}

template <typename T, typename U>
RAD_NODISCARD bool operator>=(const SharedPtr<T>& l,
                              const SharedPtr<U>& r) noexcept
{
    return l.Get() >= r.Get();
}

/// @brief Smart pointer implementing weak ownership mechanics.
/// @tparam T Value type to point to
template <typename T>
class WeakPtr final
{
public:

    using ValueType = T;
    using SharedType = SharedPtr<T>;

    ~WeakPtr()
    {
        if (m_block)
        {
            m_block->ReleaseWeak();
        }
    }

    /// @brief Construct empty WeakPtr
    WeakPtr() noexcept
        : m_block(),
          m_ptr()
    {
    }

    /// @brief Construct a new WeakPtr taking an additional weak reference to an
    /// existing pointer.
    /// @param r Existing pointer
    WeakPtr(const WeakPtr& r) noexcept
        : m_block(r.m_block),
          m_ptr(r.m_ptr)
    {
        RAD_S_ASSERT(noexcept(m_block->AcquireWeak()));

        if (m_block)
        {
            m_block->AcquireWeak();
        }
    }

    /// @brief Construct a new WeakPtr taking an additional weak reference to an
    /// existing pointer.
    /// @param r Existing pointer
    WeakPtr(const SharedType& r) noexcept
        : m_block(r.m_block),
          m_ptr(r.m_ptr)
    {
        RAD_S_ASSERT(noexcept(m_block->AcquireWeak()));

        if (m_block)
        {
            m_block->AcquireWeak();
        }
    }

    /// @brief Construct a new WeakPtr by moving an existing WeakPtr.
    /// @param r Existing pointer
    WeakPtr(WeakPtr&& r) noexcept
        : m_block(r.m_block),
          m_ptr(r.m_ptr)
    {
        r.m_block = nullptr;
        r.m_ptr = nullptr;
    }

    /// @brief Construct a new WeakPtr from a convertible pointer.
    /// @param r Existing pointer
    template <typename U, EnIf<IsConv<U*, T*>, int> = 0>
    WeakPtr(const WeakPtr<U>& r) noexcept
        : m_block(r.m_block),
          m_ptr(r.m_ptr)
    {
        RAD_S_ASSERT(noexcept(m_block->AcquireWeak()));

        if (m_block)
        {
            m_block->AcquireWeak();
        }
    }

    /// @brief Construct a new WeakPtr from a convertible pointer.
    /// @param r Existing pointer
    template <typename U, EnIf<IsConv<U*, T*>, int> = 0>
    WeakPtr(const SharedPtr<U>& r) noexcept
        : m_block(r.m_block),
          m_ptr(r.m_ptr)
    {
        RAD_S_ASSERT(noexcept(m_block->AcquireWeak()));

        if (m_block)
        {
            m_block->AcquireWeak();
        }
    }

    /// @brief Construct a new WeakPtr by moving an existing convertible
    /// pointer.
    /// @param r Existing pointer
    template <typename U, EnIf<IsConv<U*, T*>, int> = 0>
    WeakPtr(WeakPtr<U>&& r) noexcept
        : m_block(r.m_block),
          m_ptr(r.m_ptr)
    {
        r.m_block = nullptr;
        r.m_ptr = nullptr;
    }

    /// @brief Replaces the managed object with another.
    /// @param r Other object to store in this.
    /// @return Reference to this.
    WeakPtr& operator=(const WeakPtr& r) noexcept
    {
        RAD_S_ASSERT(noexcept(m_block->ReleaseWeak()));
        RAD_S_ASSERT(noexcept(m_block->AcquireWeak()));

        if RAD_LIKELY (&r != this)
        {
            if (m_block)
            {
                m_block->ReleaseWeak();
            }

            m_block = r.m_block;
            m_ptr = r.m_ptr;

            if (m_block)
            {
                m_block->AcquireWeak();
            }
        }

        return *this;
    }

    /// @brief Replaces the managed object with another.
    /// @param r Other object to store in this.
    /// @return Reference to this.
    template <typename U, EnIf<IsConv<U*, T*>, int> = 0>
    WeakPtr& operator=(const WeakPtr<U>& r) noexcept
    {
        RAD_S_ASSERT(noexcept(m_block->ReleaseWeak()));
        RAD_S_ASSERT(noexcept(m_block->AcquireWeak()));

        if (m_block)
        {
            m_block->ReleaseWeak();
        }

        m_block = r.m_block;
        m_ptr = r.m_ptr;

        if (m_block)
        {
            m_block->AcquireWeak();
        }

        return *this;
    }

    /// @brief Replaces the managed object with another.
    /// @param r Other object to store in this.
    /// @return Reference to this.
    WeakPtr& operator=(const SharedType& r) noexcept
    {
        RAD_S_ASSERT(noexcept(m_block->ReleaseWeak()));
        RAD_S_ASSERT(noexcept(m_block->AcquireWeak()));

        if (m_block)
        {
            m_block->ReleaseWeak();
        }

        m_block = r.m_block;
        m_ptr = r.m_ptr;

        if (m_block)
        {
            m_block->AcquireWeak();
        }

        return *this;
    }

    /// @brief Replaces the managed object with another.
    /// @param r Other object to store in this.
    /// @return Reference to this.
    template <typename U, EnIf<IsConv<U*, T*>, int> = 0>
    WeakPtr& operator=(const SharedPtr<U>& r) noexcept
    {
        RAD_S_ASSERT(noexcept(m_block->ReleaseWeak()));
        RAD_S_ASSERT(noexcept(m_block->AcquireWeak()));

        if (m_block)
        {
            m_block->ReleaseWeak();
        }

        m_block = r.m_block;
        m_ptr = r.m_ptr;

        if (m_block)
        {
            m_block->AcquireWeak();
        }

        return *this;
    }

    /// @brief Moves another managed object into this.
    /// @param r Other object to move into this.
    /// @return Reference to this.
    WeakPtr& operator=(WeakPtr&& r) noexcept
    {
        RAD_S_ASSERT(noexcept(m_block->ReleaseWeak()));

        if RAD_LIKELY (&r != this)
        {
            if (m_block)
            {
                m_block->ReleaseWeak();
            }

            m_block = r.m_block;
            m_ptr = r.m_ptr;
            r.m_block = nullptr;
            r.m_ptr = nullptr;
        }

        return *this;
    }

    /// @brief Moves another managed object into this.
    /// @param r Other object to move into this.
    /// @return Reference to this.
    template <typename U, EnIf<IsConv<U*, T*>, int> = 0>
    WeakPtr& operator=(WeakPtr<U>&& r) noexcept
    {
        RAD_S_ASSERT(noexcept(m_block->ReleaseWeak()));

        if (m_block)
        {
            m_block->ReleaseWeak();
        }

        m_block = r.m_block;
        m_ptr = r.m_ptr;
        r.m_block = nullptr;
        r.m_ptr = nullptr;

        return *this;
    }

    /// @brief Resets the referenced to the managed object.
    void Reset() noexcept
    {
        RAD_S_ASSERT(noexcept(m_block->ReleaseWeak()));

        if (m_block)
        {
            m_block->ReleaseWeak();
            m_block = nullptr;
            m_ptr = nullptr;
        }
    }

    /// @brief Swaps the managed objects.
    /// @param o Other shared ptr to swap with this.
    void Swap(WeakPtr& o) noexcept
    {
        if RAD_LIKELY (&o != this)
        {
            auto block = m_block;
            auto ptr = m_ptr;
            m_block = o.m_block;
            m_ptr = o.m_ptr;
            o.m_block = block;
            o.m_ptr = ptr;
        }
    }

    /// @brief Retrieves the number of shared objects that manage the object.
    /// @return Number of shared objects that manage the object.
    uint32_t UseCount() const noexcept
    {
        return m_block ? m_block->UseCount() : 0;
    }

    /// @brief Checks whether the referenced object was already deleted.
    /// @return True if the object was already deleted, false otherwise.
    bool Expired() const noexcept
    {
        return UseCount() == 0;
    }

    /// @brief Creates a SharedPtr that manages the object.
    /// @return SharedPtr that manages the object, nullptr if the object is
    /// already destructed.
    RAD_NODISCARD SharedType Lock() const noexcept
    {
        RAD_S_ASSERT(noexcept(m_block->ReleaseWeak()));

        if (m_block && m_block->LockWeak())
        {
            return SharedType(m_block, m_ptr);
        }

        return nullptr;
    }

private:

    WeakPtr(detail::PtrBlockBase* block, T* ptr) noexcept
        : m_block(block),
          m_ptr(ptr)
    {
    }

    template <typename U>
    friend class WeakPtr;

    friend class AtomicWeakPtr<T>;

    detail::PtrBlockBase* m_block;
    T* m_ptr;
};

namespace detail
{
/// @brief Internal use only
struct AllocateSharedImpl
{
    /// @brief RAII-safety wrapper helper
    template <typename TAlloc>
    struct AllocateSharedHelper
    {
        constexpr AllocateSharedHelper(TAlloc& ta) noexcept
            : alloc(ta),
              block(nullptr)
        {
        }

        ~AllocateSharedHelper()
        {
            if (block)
            {
                alloc.Free(block);
            }
        }

        TAlloc& alloc;
        typename TAlloc::ValueType* block;
    };

    template <typename T, typename TAlloc, typename... TArgs>
    static inline SharedPtr<T> AllocateShared(const TAlloc& alloc,
                                              TArgs&&... args) //
        noexcept(noexcept(DeclVal<typename PtrBlock<T, TAlloc>::AllocatorType>()
                              .Alloc(1)) &&
                 IsNoThrowCtor<T, TArgs...>)
    {
        using BlockType = PtrBlock<T, TAlloc>;
        typename BlockType::AllocatorType blockAlloc(alloc);

        AllocateSharedHelper<decltype(blockAlloc)> excSafe(blockAlloc);

        excSafe.block = blockAlloc.Alloc(1);
        if RAD_LIKELY (excSafe.block != nullptr)
        {
            new (excSafe.block) BlockType(blockAlloc, Forward<TArgs>(args)...);
            auto block = excSafe.block;
            excSafe.block = nullptr;
            return SharedPtr<T>(block, &block->Value());
        }
        return nullptr;
    }
};

} // namespace detail

/// @brief Constructs and wraps an object of type T in a SharedPtr with a custom
/// allocator.
/// @tparam T Type of object to construct
/// @tparam TAlloc Type of the custom allocator
/// @param alloc Allocator instance
/// @param args Arguments for T construction
/// @return A SharedPtr<T, TAlloc>
template <typename T, typename TAlloc, typename... TArgs>
SharedPtr<T> AllocateShared(const TAlloc& alloc, TArgs&&... args) //
    noexcept(noexcept(detail::AllocateSharedImpl::AllocateShared<T, TAlloc>(
        alloc, Forward<TArgs>(args)...)))
{
    RAD_S_ASSERT_ALLOCATOR_REQUIRES_T(TAlloc);

    return detail::AllocateSharedImpl::AllocateShared<T, TAlloc>(
        alloc,
        Forward<TArgs>(args)...);
}

#ifdef RAD_DEFAULT_ALLOCATOR
/// @brief Constructs and wraps an object of type T in a SharedPtr with the
/// default allocator.
/// @tparam T Type of object to construct
/// @tparam TAlloc Type of the custom allocator
/// @param args Arguments for T construction
/// @return A SharedPtr<T, Allocator<T>>
template <typename T, typename TAlloc RAD_ALLOCATOR_EQ(T), typename... TArgs>
SharedPtr<T> MakeShared(TArgs&&... args) noexcept(
    noexcept(AllocateShared<T>(DeclVal<TAlloc&>(), Forward<TArgs>(args)...)))
{
    RAD_S_ASSERT_ALLOCATOR_REQUIRES_T(TAlloc);

    TAlloc alloc;
    return AllocateShared<T>(alloc, Forward<TArgs>(args)...);
}
#endif

namespace detail
{

template <typename T>
class LockablePtr
{
public:

    // can consider making this 3 on 64-bit systems
    static constexpr uintptr_t ExclusiveBit = 2;
    static constexpr uintptr_t ExclusiveFlag = 1 << ExclusiveBit;
    RAD_S_ASSERTMSG(alignof(T) >= ExclusiveFlag,
                    "low order bits are needed by LockablePtr");
    static constexpr uintptr_t SharedMax = ExclusiveFlag - 1;
    static constexpr uintptr_t LockMask = (ExclusiveFlag | SharedMax);
    static constexpr uintptr_t PtrMask = ~LockMask;

    using ThisType = LockablePtr<T>;
    using ValueType = T;
    using PointerType = T*;

    ~LockablePtr() = default;

    constexpr LockablePtr() noexcept = default;

    constexpr LockablePtr(PointerType value) noexcept
        : m_storage(reinterpret_cast<uintptr_t>(value))
    {
    }

    RAD_NOT_COPYABLE(LockablePtr);

    RAD_NODISCARD PointerType UnsafeGet() const noexcept
    {
        return reinterpret_cast<PointerType>(m_storage.Load(MemOrderRelaxed) &
                                             PtrMask);
    }

    void UnsafeSet(PointerType value) noexcept
    {
        auto ptr = m_storage.Load(MemOrderRelaxed);
        m_storage.Store(reinterpret_cast<uintptr_t>(value) | (ptr & LockMask),
                        MemOrderRelaxed);
    }

    void Unlock() noexcept
    {
        if (m_storage.Load(MemOrderRelaxed) & ExclusiveFlag)
        {
            m_storage.FetchAnd(~ExclusiveFlag, MemOrderRelease);
        }
        else
        {
            m_storage.FetchSub(1, MemOrderRelease);
        }
    }

    void LockShared() noexcept
    {
        auto ptr = m_storage.Load(MemOrderAcquire);

        for (;; RAD_YIELD_PROCESSOR())
        {
            if ((ptr & LockMask) >= SharedMax)
            {
                continue;
            }

            if RAD_LIKELY (m_storage.CompareExchangeWeak(ptr,
                                                         ptr + 1,
                                                         MemOrderAcqRel,
                                                         MemOrderRelaxed))
            {
                break;
            }
        }
    }

    void LockExclusive() noexcept
    {
        auto ptr = m_storage.Load(MemOrderAcquire);

        for (;; RAD_YIELD_PROCESSOR())
        {
            if (ptr & ExclusiveFlag)
            {
                continue;
            }

            if RAD_LIKELY (m_storage.CompareExchangeWeak(ptr,
                                                         ptr | ExclusiveFlag,
                                                         MemOrderAcqRel,
                                                         MemOrderRelaxed))
            {
                break;
            }
        }

        for (; ptr & SharedMax; RAD_YIELD_PROCESSOR())
        {
            ptr = m_storage.Load(MemOrderAcquire);
        }
    }

private:

    Atomic<uintptr_t> m_storage{ 0 };
};

} // namespace detail

/// @brief Object for atomically managing a shared pointer strong reference.
/// @tparam T Type held in a shared pointer.
/// @tparam TAlloc Allocator for the shared pointer.
template <typename T>
class AtomicSharedPtr final
{
    using BlockType = detail::PtrBlockBase;
    using LockType = detail::LockablePtr<BlockType>;

public:

    using ThisType = AtomicSharedPtr<T>;
    using ValueType = SharedPtr<T>;

    ~AtomicSharedPtr()
    {
        auto block = m_block.UnsafeGet();
        if (block)
        {
            block->Release();
        }
    }

    constexpr AtomicSharedPtr() noexcept = default;

    constexpr AtomicSharedPtr(rad::nullptr_t) noexcept
        : m_block()
    {
    }

    /// @brief Constructs from an existing shared pointer.
    /// @param value Shared pointer to store.
    constexpr AtomicSharedPtr(const ValueType& value) noexcept
        : m_block(value.m_block),
          m_ptr(value.m_ptr)
    {
        if (value.m_block)
        {
            value.m_block->Acquire();
        }
    }

    RAD_NOT_COPYABLE(AtomicSharedPtr);

    /// @brief Stores a shared pointer in the atomic storage.
    /// @param value Shared pointer to store.
    void Store(ValueType value) noexcept
    {
        detail::atomic::LockRegion lockRegion;
        LockExclusive<LockType> lock(m_block);
        auto temp = value.m_block;
        value.m_block = m_block.UnsafeGet();
        m_block.UnsafeSet(temp);
        m_ptr.Store(value.m_ptr, MemOrderRelaxed);
    }

    /// @brief Loads the shared pointer from atomic storage.
    /// @return Shared pointer.
    RAD_NODISCARD ValueType Load() const noexcept
    {
        ValueType res;
        {
            detail::atomic::LockRegion lockRegion;
            LockShared<LockType> lock(m_block);
            res.m_block = m_block.UnsafeGet();
            res.m_ptr = m_ptr.Load(MemOrderRelaxed);
            if (res.m_block)
            {
                res.m_block->Acquire();
            }
        }
        return res;
    }

    /// @brief Exchanges the stored shared pointer with another.
    /// @param value Shared pointer to store.
    /// @return Previously stored shared pointer.
    RAD_NODISCARD ValueType Exchange(ValueType value) noexcept
    {
        ValueType res;
        {
            detail::atomic::LockRegion lockRegion;
            LockExclusive<LockType> lock(m_block);
            res.m_block = m_block.UnsafeGet();
            res.m_ptr = m_ptr.Load(MemOrderRelaxed);
            m_block.UnsafeSet(value.m_block);
            m_ptr.Store(value.m_ptr, MemOrderRelaxed);
            value.m_block = nullptr;
        }
        return res;
    }

    void operator=(ValueType value) noexcept
    {
        Store(Move(value));
    }

    operator ValueType() const noexcept
    {
        return Load();
    }

private:

    mutable LockType m_block;
    Atomic<T*> m_ptr{ nullptr };
};

/// @brief Object for atomically managing a weak pointer reference.
/// @tparam T Type held in a weak pointer.
/// @tparam TAlloc Allocator for the weak pointer.
template <typename T>
class AtomicWeakPtr final
{
    using BlockType = detail::PtrBlockBase;
    using LockType = detail::LockablePtr<BlockType>;

public:

    using ThisType = AtomicWeakPtr<T>;
    using ValueType = WeakPtr<T>;
    using SharedType = SharedPtr<T>;

    ~AtomicWeakPtr()
    {
        auto block = m_block.UnsafeGet();
        if (block)
        {
            block->ReleaseWeak();
        }
    }

    constexpr AtomicWeakPtr() noexcept = default;

    /// @brief Constructs from an existing weak pointer.
    /// @param value Weak pointer to store.
    constexpr AtomicWeakPtr(const ValueType& value) noexcept
        : m_block(value.m_block),
          m_ptr(value.m_ptr)
    {
        if (value.m_block)
        {
            value.m_block->AcquireWeak();
        }
    }

    /// @brief Constructs from an existing shared pointer.
    /// @param value Weak pointer to store.
    constexpr AtomicWeakPtr(const SharedType& value) noexcept
        : m_block(value.m_block),
          m_ptr(value.m_ptr)
    {
        if (value.m_block)
        {
            value.m_block->AcquireWeak();
        }
    }

    RAD_NOT_COPYABLE(AtomicWeakPtr);

    /// @brief Stores a weak pointer in the atomic storage.
    /// @param value Weak pointer to store.
    void Store(ValueType value) noexcept
    {
        detail::atomic::LockRegion lockRegion;
        LockExclusive<LockType> lock(m_block);
        auto temp = value.m_block;
        value.m_block = m_block.UnsafeGet();
        m_block.UnsafeSet(temp);
        m_ptr.Store(value.m_ptr, MemOrderRelaxed);
    }

    /// @brief Loads the weak pointer from atomic storage.
    /// @return Weak pointer.
    RAD_NODISCARD ValueType Load() const noexcept
    {
        ValueType res;
        {
            detail::atomic::LockRegion lockRegion;
            LockShared<LockType> lock(m_block);
            res.m_block = m_block.UnsafeGet();
            res.m_ptr = m_ptr.Load(MemOrderRelaxed);
            if (res.m_block)
            {
                res.m_block->AcquireWeak();
            }
        }
        return res;
    }

    /// @brief Exchanges the stored weak pointer with another.
    /// @param value Weak pointer to store.
    /// @return Previously stored weak pointer.
    RAD_NODISCARD ValueType Exchange(ValueType value) noexcept
    {
        ValueType res;
        {
            detail::atomic::LockRegion lockRegion;
            LockExclusive<LockType> lock(m_block);
            res.m_block = m_block.UnsafeGet();
            res.m_ptr = m_ptr.Load(MemOrderRelaxed);
            m_block.UnsafeSet(value.m_block);
            m_ptr.Store(value.m_ptr, MemOrderRelaxed);
            value.m_block = nullptr;
        }
        return res;
    }

    void operator=(ValueType value) noexcept
    {
        Store(Move(value));
    }

    void operator=(SharedType value) noexcept
    {
        Store(Move(value));
    }

    operator ValueType() const noexcept
    {
        return Load();
    }

    operator SharedType() const noexcept
    {
        return Load().Lock();
    }

private:

    mutable LockType m_block;
    Atomic<T*> m_ptr{ nullptr };
};

} // namespace rad
