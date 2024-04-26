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

#include "gtest/gtest.h"

#include "radiant/Locks.h"

struct TestLock
{
    void LockExclusive() noexcept
    {
        exclusive = true;
        ++count;
    }

    void LockShared() noexcept
    {
        exclusive = false;
        ++count;
    }

    void Unlock() noexcept
    {
        exclusive = false;
        --count;
    }

    bool exclusive{};
    int count{};
};

static_assert(noexcept(TestLock().LockExclusive()), "");
static_assert(noexcept(TestLock().LockShared()), "");
static_assert(noexcept(TestLock().Unlock()), "");

#if RAD_CPP17
#define DEDUCED(GuardType) GuardType
#else
#define DEDUCED(GuardType) GuardType<TestLock>
#endif

TEST(LocksTest, LockExclusive)
{
    TestLock lock;

    {
        DEDUCED(rad::LockExclusive) guard(lock);
        EXPECT_TRUE(lock.exclusive);
        EXPECT_EQ(lock.count, 1);
    }

    EXPECT_EQ(lock.count, 0);
}

TEST(LocksTest, LockShared)
{
    TestLock lock;

    {
        DEDUCED(rad::LockShared) guard(lock);
        EXPECT_FALSE(lock.exclusive);
        EXPECT_EQ(lock.count, 1);
    }

    EXPECT_EQ(lock.count, 0);
}

TEST(LocksTest, RelockableExclusiveCtor)
{
    TestLock lock;

    DEDUCED(rad::RelockableExclusive) guard(lock);
    EXPECT_TRUE(lock.exclusive);
    EXPECT_EQ(lock.count, 1);
}

TEST(LocksTest, RelockableExclusiveDeferredCtor)
{
    TestLock lock;

    DEDUCED(rad::RelockableExclusive) guard(lock, rad::DeferLocking);
    EXPECT_FALSE(lock.exclusive);
    EXPECT_EQ(lock.count, 0);
}

TEST(LocksTest, RelockableExclusiveLockedDtor)
{
    TestLock lock;

    {
        DEDUCED(rad::RelockableExclusive) guard(lock);
        EXPECT_EQ(lock.count, 1);
    }

    EXPECT_EQ(lock.count, 0);
}

TEST(LocksTest, RelockableExclusiveUnlockedDtor)
{
    TestLock lock;

    {
        DEDUCED(rad::RelockableExclusive) guard(lock, rad::DeferLocking);
        EXPECT_EQ(lock.count, 0);
    }

    EXPECT_EQ(lock.count, 0);
}

TEST(LocksTest, RelockableExclusiveLock)
{
    TestLock lock;

    DEDUCED(rad::RelockableExclusive) guard(lock, rad::DeferLocking);
    guard.Lock();
    EXPECT_TRUE(lock.exclusive);
    EXPECT_EQ(lock.count, 1);
}

TEST(LocksTest, RelockableExclusiveUnlock)
{
    TestLock lock;

    DEDUCED(rad::RelockableExclusive) guard(lock);
    guard.Unlock();
    EXPECT_EQ(lock.count, 0);
}

TEST(LocksTest, RelockableSharedCtor)
{
    TestLock lock;

    DEDUCED(rad::RelockableShared) guard(lock);
    EXPECT_FALSE(lock.exclusive);
    EXPECT_EQ(lock.count, 1);
}

TEST(LocksTest, RelockableSharedDeferredCtor)
{
    TestLock lock;

    DEDUCED(rad::RelockableShared) guard(lock, rad::DeferLocking);
    EXPECT_FALSE(lock.exclusive);
    EXPECT_EQ(lock.count, 0);
}

TEST(LocksTest, RelockableSharedLockedDtor)
{
    TestLock lock;

    {
        DEDUCED(rad::RelockableShared) guard(lock);
        EXPECT_EQ(lock.count, 1);
    }

    EXPECT_EQ(lock.count, 0);
}

TEST(LocksTest, RelockableSharedUnlockedDtor)
{
    TestLock lock;

    {
        DEDUCED(rad::RelockableShared) guard(lock, rad::DeferLocking);
        EXPECT_EQ(lock.count, 0);
    }

    EXPECT_EQ(lock.count, 0);
}

TEST(LocksTest, RelockableSharedLock)
{
    TestLock lock;

    DEDUCED(rad::RelockableShared) guard(lock, rad::DeferLocking);
    guard.Lock();
    EXPECT_FALSE(lock.exclusive);
    EXPECT_EQ(lock.count, 1);
}

TEST(LocksTest, RelockableSharedUnlock)
{
    TestLock lock;

    DEDUCED(rad::RelockableShared) guard(lock);
    guard.Unlock();
    EXPECT_EQ(lock.count, 0);
}
