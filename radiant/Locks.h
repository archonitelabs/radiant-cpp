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

#include "radiant/TypeTraits.h"

namespace rad
{

// Lock guards implementing management of locking and unlocking of locks. Locks
// are expected to implement the following interface to be compatible with these
// guards:
//
//    void Unlock();
//
//  and one or both of:
//
//    void LockExclusive();
//    void LockShared();
//
// Lock guards are not allowed to fail lock acquisition in construction, and so
// by extension, LockExclusive() and LockShared() cannot fail.

/// @brief Lock guard to acquire a lock exclusively. Guarantees lock is acquired
/// in construction.
template <typename TLock>
class RAD_NODISCARD LockExclusive final
{
public:

    using LockType = TLock;

    RAD_NOT_COPYABLE(LockExclusive);

    /// @brief Acquire lock exclusively during construction
    LockExclusive(LockType& lock) noexcept(noexcept(lock.LockExclusive()))
        : m_lock(lock)
    {
        RAD_S_ASSERT_NOTHROW(noexcept(lock.LockExclusive()));

        m_lock.LockExclusive();
    }

    ~LockExclusive()
    {
        RAD_S_ASSERT_NOTHROW_DTOR(noexcept(m_lock.Unlock()));

        m_lock.Unlock();
    }

private:

    LockType& m_lock;
};

/// @brief Lock guard to acquire a lock shared. Guarantees lock is acquired in
/// construction.
template <typename TLock>
class RAD_NODISCARD LockShared final
{
public:

    using LockType = TLock;

    RAD_NOT_COPYABLE(LockShared);

    /// @brief Acquire lock shared during construction
    LockShared(LockType& lock) noexcept(noexcept(lock.LockShared()))
        : m_lock(lock)
    {
        RAD_S_ASSERT_NOTHROW(noexcept(lock.LockShared()));

        m_lock.LockShared();
    }

    ~LockShared()
    {
        RAD_S_ASSERT_NOTHROW_DTOR(noexcept(m_lock.Unlock()));

        m_lock.Unlock();
    }

private:

    LockType& m_lock;
};

/// @brief Tag-type to indicate Relockable guards to defer locking to after
/// construction with a manual call to Lock.
struct DeferLockingTag
{
};

RAD_INLINE_VAR constexpr DeferLockingTag DeferLocking{};

/// @brief Lock guard to acquire a lock exclusively and allow Unlock() and
/// Lock() in an RAII-safe manner mid-use.
/// @details Acquires the given lock exclusively in construction unless the
/// DeferLocking tag is specified. In the DeferLocking case, Lock() must be
/// called to acquire the lock.
template <typename TLock>
class RAD_NODISCARD RelockableExclusive final
{
public:

    using LockType = TLock;

    RAD_NOT_COPYABLE(RelockableExclusive);

    /// @brief Automatically acquire lock during construction
    RelockableExclusive(LockType& lock) noexcept(noexcept(lock.LockExclusive()))
        : m_lock(lock),
          m_acquired(true)
    {
        RAD_S_ASSERT_NOTHROW(noexcept(lock.LockExclusive()));
        m_lock.LockExclusive();
    }

    /// @brief Defer acquiring the lock to a manual call to Lock()
    RelockableExclusive(LockType& lock, DeferLockingTag) noexcept
        : m_lock(lock),
          m_acquired(false)
    {
    }

    ~RelockableExclusive()
    {
        RAD_S_ASSERT_NOTHROW_DTOR(noexcept(m_lock.Unlock()));

        if (m_acquired)
        {
            m_lock.Unlock();
        }
    }

    /// @brief Releases the lock. Destruction will not release the lock again.
    /// @warning It is not allowed to call Unlock() if the lock is not acquired.
    void Unlock() noexcept(noexcept(DeclVal<LockType>().Unlock()))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(DeclVal<LockType>().Unlock()));

        RAD_ASSERT(m_acquired);
        m_acquired = false;
        m_lock.Unlock();
    }

    /// @brief Acquires the lock exclusively. The lock will automatically be
    /// released upon destruction.
    /// @warning It is not allowed to call Lock() if the lock is already
    /// acquired.
    void Lock() noexcept(noexcept(DeclVal<LockType>().LockExclusive()))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(DeclVal<LockType>().LockExclusive()));

        RAD_ASSERT(!m_acquired);
        m_lock.LockExclusive();
        m_acquired = true;
    }

private:

    LockType& m_lock;
    bool m_acquired;
};

/// @brief Lock guard to acquire a lock shared and allow Unlock() and Lock()
/// in an RAII-safe manner mid-use.
/// @details Acquires the given lock shared in construction unless the
/// DeferLocking tag is specified. In the DeferLocking case, Lock() must be
/// called to acquire the lock.
template <typename TLock>
class RAD_NODISCARD RelockableShared final
{
public:

    using LockType = TLock;

    RAD_NOT_COPYABLE(RelockableShared);

    /// @brief Automatically acquire lock during construction
    RelockableShared(LockType& lock) noexcept(noexcept(lock.LockShared()))
        : m_lock(lock),
          m_acquired(true)
    {
        RAD_S_ASSERT_NOTHROW(noexcept(lock.LockShared()));

        m_lock.LockShared();
    }

    /// @brief Defer acquiring the lock to a manual call to Lock()
    RelockableShared(LockType& lock, DeferLockingTag) noexcept
        : m_lock(lock),
          m_acquired(false)
    {
    }

    /// @brief Defer acquiring the lock to a manual call to Lock()
    ~RelockableShared()
    {
        RAD_S_ASSERT_NOTHROW_DTOR(noexcept(m_lock.Unlock()));

        if (m_acquired)
        {
            m_lock.Unlock();
        }
    }

    /// @brief Releases the lock. Destruction will not release the lock again.
    /// @warning It is not allowed to call Unlock() if the lock is not acquired.
    void Unlock() noexcept(noexcept(DeclVal<LockType>().Unlock()))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(DeclVal<LockType>().Unlock()));

        RAD_ASSERT(m_acquired);
        m_acquired = false;
        m_lock.Unlock();
    }

    /// @brief Acquires the lock shared. The lock will automatically be
    /// released upon destruction.
    /// @warning It is not allowed to call Lock() if the lock is already
    /// acquired.
    void Lock() noexcept(noexcept(DeclVal<LockType>().LockShared()))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(DeclVal<LockType>().Unlock()));

        RAD_ASSERT(!m_acquired);
        m_lock.LockShared();
        m_acquired = true;
    }

private:

    LockType& m_lock;
    bool m_acquired;
};

} // namespace rad
