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

#include "radiant/UniqueResource.h"

#include "gtest/gtest.h"

static int g_IsValidCalls = 0;
static int g_CloseCalls = 0;
static int g_CloserCalls = 0;

struct ResourceCloser
{
    static void Close(int value) noexcept
    {
        RAD_UNUSED(value);
        g_CloserCalls++;
    }
};

using TestResource = rad::UniqueResourceDef<int, ResourceCloser>;

struct MockResourcePolicy
{
    using ValueType = int;
    static constexpr int InvalidValue = 0xdead;

    static bool IsValid(int value) noexcept
    {
        g_IsValidCalls++;
        return value != InvalidValue;
    }

    static void Close(int value) noexcept
    {
        RAD_UNUSED(value);
        g_CloseCalls++;
    }
};

constexpr int MockResourcePolicy::InvalidValue;

using MockResource = rad::UniqueResource<MockResourcePolicy>;

namespace
{

void MockOpener(int* resource)
{
    *resource = 123;
}

} // namespace

class UniqueResourceTests : public ::testing::Test
{
public:

    void SetUp() override
    {
        g_IsValidCalls = 0;
        g_CloseCalls = 0;
        g_CloserCalls = 0;
    }

    void TearDown() override
    {
    }
};

TEST_F(UniqueResourceTests, DefaultConstruct)
{
    {
        MockResource h;

        EXPECT_EQ(h.Get(), MockResourcePolicy::InvalidValue);
    }

    EXPECT_EQ(g_IsValidCalls, 1);
    EXPECT_EQ(g_CloseCalls, 0);
}

TEST_F(UniqueResourceTests, Construct)
{
    {
        MockResource h(456);

        EXPECT_EQ(h.Get(), 456);
    }

    EXPECT_EQ(g_IsValidCalls, 1);
    EXPECT_EQ(g_CloseCalls, 1);
}

TEST_F(UniqueResourceTests, MoveConstruct)
{
    {
        MockResource h(456);
        MockResource o(Move(h));

        EXPECT_EQ(g_IsValidCalls, 0);
        EXPECT_EQ(g_CloseCalls, 0);

        EXPECT_EQ(h.Get(), MockResourcePolicy::InvalidValue);
        EXPECT_EQ(o.Get(), 456);
    }

    EXPECT_EQ(g_IsValidCalls, 2);
    EXPECT_EQ(g_CloseCalls, 1);
}

TEST_F(UniqueResourceTests, MoveAssign)
{
    {
        MockResource h(456);
        MockResource o;

        EXPECT_EQ(g_IsValidCalls, 0);
        EXPECT_EQ(g_CloseCalls, 0);

        o = Move(h);

        EXPECT_EQ(g_IsValidCalls, 1);
        EXPECT_EQ(g_CloseCalls, 0);

        EXPECT_EQ(h.Get(), MockResourcePolicy::InvalidValue);
        EXPECT_EQ(o.Get(), 456);
    }

    EXPECT_EQ(g_IsValidCalls, 3);
    EXPECT_EQ(g_CloseCalls, 1);
}

TEST_F(UniqueResourceTests, IsValid)
{
    MockResource h;

    EXPECT_FALSE(h.IsValid());

    h = MockResource(123);

    EXPECT_TRUE(h.IsValid());
}

TEST_F(UniqueResourceTests, Reset)
{
    MockResource h;

    EXPECT_FALSE(h.IsValid());

    EXPECT_EQ(g_CloseCalls, 0);

    h.Reset(123);

    EXPECT_EQ(g_CloseCalls, 0);

    EXPECT_EQ(h.Get(), 123);

    EXPECT_TRUE(h.IsValid());

    EXPECT_EQ(g_CloseCalls, 0);

    h.Reset();

    EXPECT_EQ(g_CloseCalls, 1);
}

TEST_F(UniqueResourceTests, Release)
{
    MockResource h;

    EXPECT_FALSE(h.IsValid());
    EXPECT_FALSE(h);

    EXPECT_EQ(h.Release(), MockResourcePolicy::InvalidValue);

    h.Reset(123);

    EXPECT_TRUE(h.IsValid());

    EXPECT_EQ(h.Release(), 123);

    EXPECT_FALSE(h.IsValid());
}

TEST_F(UniqueResourceTests, Swap)
{
    MockResource h(123);
    MockResource o;

    EXPECT_EQ(h.Get(), 123);
    EXPECT_EQ(o.Get(), MockResourcePolicy::InvalidValue);

    EXPECT_EQ(g_CloseCalls, 0);

    h.Swap(o);

    EXPECT_EQ(g_CloseCalls, 0);

    EXPECT_EQ(h.Get(), MockResourcePolicy::InvalidValue);
    EXPECT_EQ(o.Get(), 123);
}

TEST_F(UniqueResourceTests, Put)
{
    MockResource h;

    EXPECT_EQ(g_CloseCalls, 0);

    MockOpener(h.Put());

    EXPECT_EQ(g_CloseCalls, 0);

    EXPECT_EQ(h.Get(), 123);

    h.Reset();

    EXPECT_EQ(g_CloseCalls, 1);

    MockOpener(&h);

    EXPECT_EQ(g_CloseCalls, 1);

    EXPECT_EQ(h.Get(), 123);
}

TEST_F(UniqueResourceTests, Compare)
{
    MockResource h(1);
    MockResource o(2);

    EXPECT_FALSE(h == o);
    EXPECT_TRUE(h != o);
    EXPECT_TRUE(h < o);
    EXPECT_TRUE(h <= o);
    EXPECT_FALSE(h > o);
    EXPECT_FALSE(h >= o);

    EXPECT_FALSE(h == MockResourcePolicy::InvalidValue);
    EXPECT_TRUE(h != MockResourcePolicy::InvalidValue);
    EXPECT_TRUE(h < MockResourcePolicy::InvalidValue);
    EXPECT_TRUE(h <= MockResourcePolicy::InvalidValue);
    EXPECT_FALSE(h > MockResourcePolicy::InvalidValue);
    EXPECT_FALSE(h >= MockResourcePolicy::InvalidValue);

    EXPECT_FALSE(MockResourcePolicy::InvalidValue == h);
    EXPECT_TRUE(MockResourcePolicy::InvalidValue != h);
    EXPECT_FALSE(MockResourcePolicy::InvalidValue < h);
    EXPECT_FALSE(MockResourcePolicy::InvalidValue <= h);
    EXPECT_TRUE(MockResourcePolicy::InvalidValue > h);
    EXPECT_TRUE(MockResourcePolicy::InvalidValue >= h);
}

TEST_F(UniqueResourceTests, TestDef)
{
    TestResource h(123);

    EXPECT_TRUE(h);

    EXPECT_EQ(g_IsValidCalls, 0);
    EXPECT_EQ(g_CloserCalls, 0);

    h.Reset();

    EXPECT_FALSE(h);

    EXPECT_EQ(g_IsValidCalls, 0);
    EXPECT_EQ(g_CloserCalls, 1);
}
