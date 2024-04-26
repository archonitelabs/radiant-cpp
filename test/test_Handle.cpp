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

#include "radiant/Handle.h"

static int g_IsValidCalls = 0;
static int g_CloseCalls = 0;
static int g_CloserCalls = 0;

struct HandleCloser
{
    static void Close(int value) noexcept
    {
        RAD_UNUSED(value);
        g_CloserCalls++;
    }
};

using TestHandle = rad::HandleDef<int, HandleCloser>;

struct MockHandlePolicy
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

constexpr int MockHandlePolicy::InvalidValue;

using MockHandle = rad::Handle<MockHandlePolicy>;

namespace
{

void MockOpener(int* handle)
{
    *handle = 123;
}

} // namespace

class HandleTests : public ::testing::Test
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

TEST_F(HandleTests, DefaultConstruct)
{
    {
        MockHandle h;

        EXPECT_EQ(h.Get(), MockHandlePolicy::InvalidValue);
    }

    EXPECT_EQ(g_IsValidCalls, 1);
    EXPECT_EQ(g_CloseCalls, 0);
}

TEST_F(HandleTests, Construct)
{
    {
        MockHandle h(456);

        EXPECT_EQ(h.Get(), 456);
    }

    EXPECT_EQ(g_IsValidCalls, 1);
    EXPECT_EQ(g_CloseCalls, 1);
}

TEST_F(HandleTests, MoveConstruct)
{
    {
        MockHandle h(456);
        MockHandle o(Move(h));

        EXPECT_EQ(g_IsValidCalls, 0);
        EXPECT_EQ(g_CloseCalls, 0);

        EXPECT_EQ(h.Get(), MockHandlePolicy::InvalidValue);
        EXPECT_EQ(o.Get(), 456);
    }

    EXPECT_EQ(g_IsValidCalls, 2);
    EXPECT_EQ(g_CloseCalls, 1);
}

TEST_F(HandleTests, MoveAssign)
{
    {
        MockHandle h(456);
        MockHandle o;

        EXPECT_EQ(g_IsValidCalls, 0);
        EXPECT_EQ(g_CloseCalls, 0);

        o = Move(h);

        EXPECT_EQ(g_IsValidCalls, 1);
        EXPECT_EQ(g_CloseCalls, 0);

        EXPECT_EQ(h.Get(), MockHandlePolicy::InvalidValue);
        EXPECT_EQ(o.Get(), 456);
    }

    EXPECT_EQ(g_IsValidCalls, 3);
    EXPECT_EQ(g_CloseCalls, 1);
}

TEST_F(HandleTests, IsValid)
{
    MockHandle h;

    EXPECT_FALSE(h.IsValid());

    h = MockHandle(123);

    EXPECT_TRUE(h.IsValid());
}

TEST_F(HandleTests, Reset)
{
    MockHandle h;

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

TEST_F(HandleTests, Release)
{
    MockHandle h;

    EXPECT_FALSE(h.IsValid());
    EXPECT_FALSE(h);

    EXPECT_EQ(h.Release(), MockHandlePolicy::InvalidValue);

    h.Reset(123);

    EXPECT_TRUE(h.IsValid());

    EXPECT_EQ(h.Release(), 123);

    EXPECT_FALSE(h.IsValid());
}

TEST_F(HandleTests, Swap)
{
    MockHandle h(123);
    MockHandle o;

    EXPECT_EQ(h.Get(), 123);
    EXPECT_EQ(o.Get(), MockHandlePolicy::InvalidValue);

    EXPECT_EQ(g_CloseCalls, 0);

    h.Swap(o);

    EXPECT_EQ(g_CloseCalls, 0);

    EXPECT_EQ(h.Get(), MockHandlePolicy::InvalidValue);
    EXPECT_EQ(o.Get(), 123);
}

TEST_F(HandleTests, Put)
{
    MockHandle h;

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

TEST_F(HandleTests, Compare)
{
    MockHandle h(1);
    MockHandle o(2);

    EXPECT_FALSE(h == o);
    EXPECT_TRUE(h != o);
    EXPECT_TRUE(h < o);
    EXPECT_TRUE(h <= o);
    EXPECT_FALSE(h > o);
    EXPECT_FALSE(h >= o);

    EXPECT_FALSE(h == MockHandlePolicy::InvalidValue);
    EXPECT_TRUE(h != MockHandlePolicy::InvalidValue);
    EXPECT_TRUE(h < MockHandlePolicy::InvalidValue);
    EXPECT_TRUE(h <= MockHandlePolicy::InvalidValue);
    EXPECT_FALSE(h > MockHandlePolicy::InvalidValue);
    EXPECT_FALSE(h >= MockHandlePolicy::InvalidValue);

    EXPECT_FALSE(MockHandlePolicy::InvalidValue == h);
    EXPECT_TRUE(MockHandlePolicy::InvalidValue != h);
    EXPECT_FALSE(MockHandlePolicy::InvalidValue < h);
    EXPECT_FALSE(MockHandlePolicy::InvalidValue <= h);
    EXPECT_TRUE(MockHandlePolicy::InvalidValue > h);
    EXPECT_TRUE(MockHandlePolicy::InvalidValue >= h);
}

TEST_F(HandleTests, TestDef)
{
    TestHandle h(123);

    EXPECT_TRUE(h);

    EXPECT_EQ(g_IsValidCalls, 0);
    EXPECT_EQ(g_CloserCalls, 0);

    h.Reset();

    EXPECT_FALSE(h);

    EXPECT_EQ(g_IsValidCalls, 0);
    EXPECT_EQ(g_CloserCalls, 1);
}
