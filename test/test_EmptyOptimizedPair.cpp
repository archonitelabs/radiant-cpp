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

#include "radiant/EmptyOptimizedPair.h"

#include "gtest/gtest.h"
#include "test/TestAlloc.h"
#include <vector>

struct Empty
{
    Empty() = default;

    Empty(Empty&) noexcept
    {
        ++counter;
    }

    Empty(Empty const&) noexcept
    {
        counter += 2;
    }

    Empty(Empty&&) noexcept
    {
        counter += 3;
    }

    uint32_t One() const noexcept
    {
        return 1;
    }

    static int counter;
};

int Empty::counter = 0;

struct Stateful
{
    uint32_t value;
};

struct ThrowingEmpty
{
    ThrowingEmpty()
    {
    }

    ThrowingEmpty(ThrowingEmpty const&)
    {
    }

    ThrowingEmpty(ThrowingEmpty&&)
    {
    }
};

struct ThrowingStateful
{
    explicit ThrowingStateful(uint32_t v)
        : value(v)
    {
    }

    ThrowingStateful(ThrowingStateful const&)
    {
    }

    ThrowingStateful(ThrowingStateful&&)
    {
    }

    uint32_t value;
};

// empty ctors
RAD_S_ASSERT(noexcept(rad::EmptyOptimizedPair<Empty, bool>()));
RAD_S_ASSERT(!noexcept(rad::EmptyOptimizedPair<ThrowingEmpty, bool>()));
RAD_S_ASSERT(!noexcept(rad::EmptyOptimizedPair<Empty, ThrowingEmpty>()));
RAD_S_ASSERT(noexcept(rad::EmptyOptimizedPair<radtest::Allocator<int>, bool>(
    rad::DeclVal<radtest::Allocator<int>&>(), true)));
RAD_S_ASSERT(noexcept(rad::EmptyOptimizedPair<radtest::Allocator<int>, bool>(
    rad::DeclVal<const radtest::Allocator<int>&>(), true)));
RAD_S_ASSERT(noexcept(rad::EmptyOptimizedPair<radtest::Allocator<int>, bool>(
    rad::DeclVal<radtest::Allocator<int>&&>(), true)));

// empty copy ctors
RAD_S_ASSERT(noexcept(rad::EmptyOptimizedPair<Empty, Empty>(
    rad::DeclVal<rad::EmptyOptimizedPair<Empty, Empty>&>())));
RAD_S_ASSERT(!noexcept(rad::EmptyOptimizedPair<ThrowingEmpty, Empty>(
    rad::DeclVal<rad::EmptyOptimizedPair<ThrowingEmpty, Empty>&>())));
RAD_S_ASSERT(!noexcept(rad::EmptyOptimizedPair<Empty, ThrowingEmpty>(
    rad::DeclVal<rad::EmptyOptimizedPair<Empty, ThrowingEmpty>&>())));

// empty move ctors
RAD_S_ASSERT(noexcept(rad::EmptyOptimizedPair<Empty, Empty>(
    rad::DeclVal<rad::EmptyOptimizedPair<Empty, Empty>&&>())));
RAD_S_ASSERT(!noexcept(rad::EmptyOptimizedPair<ThrowingEmpty, Empty>(
    rad::DeclVal<rad::EmptyOptimizedPair<ThrowingEmpty, Empty>&&>())));
RAD_S_ASSERT(!noexcept(rad::EmptyOptimizedPair<Empty, ThrowingEmpty>(
    rad::DeclVal<rad::EmptyOptimizedPair<Empty, ThrowingEmpty>&&>())));

// non-empty ctors
RAD_S_ASSERT(noexcept(rad::EmptyOptimizedPair<Stateful, bool>()));
RAD_S_ASSERT(!noexcept(rad::EmptyOptimizedPair<ThrowingStateful, bool>(
    ThrowingStateful(0), true)));
RAD_S_ASSERT(!noexcept(
    rad::EmptyOptimizedPair<Stateful, ThrowingStateful>(Stateful(), 0)));
RAD_S_ASSERT(noexcept(rad::EmptyOptimizedPair<Stateful, bool>(
    rad::DeclVal<const Stateful&>(), true)));
RAD_S_ASSERT(noexcept(
    rad::EmptyOptimizedPair<Stateful, bool>(rad::DeclVal<Stateful&&>(), true)));

// non-empty copy ctors
RAD_S_ASSERT(noexcept(rad::EmptyOptimizedPair<Stateful, bool>(
    rad::DeclVal<rad::EmptyOptimizedPair<Stateful, bool>&>())));
RAD_S_ASSERT(!noexcept(rad::EmptyOptimizedPair<ThrowingStateful, bool>(
    rad::DeclVal<rad::EmptyOptimizedPair<ThrowingStateful, bool>&>())));
RAD_S_ASSERT(!noexcept(rad::EmptyOptimizedPair<bool, ThrowingStateful>(
    rad::DeclVal<rad::EmptyOptimizedPair<bool, ThrowingStateful>&>())));

// non-empty move ctors
RAD_S_ASSERT(noexcept(rad::EmptyOptimizedPair<Stateful, bool>(
    rad::DeclVal<rad::EmptyOptimizedPair<Stateful, bool>&&>())));
RAD_S_ASSERT(!noexcept(rad::EmptyOptimizedPair<ThrowingStateful, bool>(
    rad::DeclVal<rad::EmptyOptimizedPair<ThrowingStateful, bool>&&>())));
RAD_S_ASSERT(!noexcept(rad::EmptyOptimizedPair<bool, ThrowingStateful>(
    rad::DeclVal<rad::EmptyOptimizedPair<bool, ThrowingStateful>&&>())));

TEST(TestEmptyOptimizedPair, EmptyBaseDefaultValues)
{
    rad::EmptyOptimizedPair<Empty, uint32_t> pair;
    EXPECT_EQ(sizeof(pair), sizeof(uint32_t));
    EXPECT_EQ(pair.First().One(), 1u);
    EXPECT_EQ(pair.Second(), 0u);
}

TEST(TestEmptyOptimizedPair, EmptyBaseInitializedSecond)
{
    rad::EmptyOptimizedPair<Empty, uint32_t> pair(1u);
    EXPECT_EQ(sizeof(pair), sizeof(uint32_t));
    EXPECT_EQ(pair.Second(), 1u);

    const decltype(pair)& ref = pair;
    EXPECT_EQ(ref.First().One(), 1u);
    EXPECT_EQ(ref.Second(), 1u);
}

TEST(TestEmptyOptimizedPair, EmptyBaseInitializedBoth)
{
    Empty empty;

    rad::EmptyOptimizedPair<Empty, uint32_t> pair1(empty, 1u);
    EXPECT_EQ(Empty::counter, 1);
    EXPECT_EQ(pair1.Second(), 1u);

    const Empty constEmpty;
    rad::EmptyOptimizedPair<Empty, uint32_t> pair2(constEmpty, 1u);
    EXPECT_EQ(Empty::counter, 3);
    EXPECT_EQ(pair2.Second(), 1u);

    rad::EmptyOptimizedPair<Empty, uint32_t> pair3(rad::Move(empty), 1u);
    EXPECT_EQ(Empty::counter, 6);
    EXPECT_EQ(pair3.Second(), 1u);
}

TEST(TestEmtpyOptimizedPair, EmptyCopy)
{
    rad::EmptyOptimizedPair<Empty, uint32_t> pair1(1u);
    rad::EmptyOptimizedPair<Empty, uint32_t> pair2(pair1);

    EXPECT_EQ(pair1.Second(), pair2.Second());

    const rad::EmptyOptimizedPair<Empty, uint32_t> pair3(2u);
    rad::EmptyOptimizedPair<Empty, uint32_t> pair4(pair3);

    EXPECT_EQ(pair3.Second(), pair4.Second());
}

TEST(TestEmtpyOptimizedPair, EmptyMove)
{
    rad::EmptyOptimizedPair<Empty, std::vector<char>> pair1;
    pair1.Second().push_back(1);

    rad::EmptyOptimizedPair<Empty, std::vector<char>> pair2(rad::Move(pair1));
    EXPECT_TRUE(pair1.Second().empty());
    EXPECT_EQ(pair2.Second().size(), 1u);
}

TEST(TestEmptyOptimizedPair, StatefulBaseDefaultValues)
{
    rad::EmptyOptimizedPair<Stateful, uint32_t> pair;
    EXPECT_GT(sizeof(pair), sizeof(uint32_t));
    EXPECT_EQ(pair.First().value, 0u);
    EXPECT_EQ(pair.Second(), 0u);
}

TEST(TestEmptyOptimizedPair, StatefulBaseInitialized)
{
    rad::EmptyOptimizedPair<Stateful, uint32_t> pair(Stateful{ 1u }, 2u);
    EXPECT_GT(sizeof(pair), sizeof(uint32_t));
    EXPECT_EQ(pair.First().value, 1u);
    EXPECT_EQ(pair.Second(), 2u);

    const decltype(pair)& ref = pair;
    EXPECT_EQ(ref.First().value, 1u);
    EXPECT_EQ(ref.Second(), 2u);

    Stateful s{ 2u };
    rad::EmptyOptimizedPair<Stateful, uint32_t> pair2(s, 3u);
    EXPECT_GT(sizeof(pair2), sizeof(uint32_t));
    EXPECT_EQ(pair2.First().value, 2u);
    EXPECT_EQ(pair2.Second(), 3u);

    rad::EmptyOptimizedPair<Stateful, uint32_t> pair3(rad::Move(s), 3u);
    EXPECT_GT(sizeof(pair3), sizeof(uint32_t));
    EXPECT_EQ(pair3.First().value, 2u);
    EXPECT_EQ(pair3.Second(), 3u);
}

TEST(TestEmtpyOptimizedPair, StatefulCopy)
{
    rad::EmptyOptimizedPair<Stateful, uint32_t> pair1(Stateful{ 1u }, 2u);
    rad::EmptyOptimizedPair<Stateful, uint32_t> pair2(pair1);

    EXPECT_EQ(pair1.First().value, pair2.First().value);
    EXPECT_EQ(pair1.Second(), pair2.Second());

    const rad::EmptyOptimizedPair<Stateful, uint32_t> pair3(Stateful{ 2u }, 3u);
    rad::EmptyOptimizedPair<Stateful, uint32_t> pair4(pair3);

    EXPECT_EQ(pair3.First().value, pair4.First().value);
    EXPECT_EQ(pair3.Second(), pair4.Second());
}

TEST(TestEmtpyOptimizedPair, StatefulMove)
{
    rad::EmptyOptimizedPair<std::vector<char>, std::vector<char>> pair1;
    pair1.First().push_back(1);
    pair1.Second().push_back(2);

    rad::EmptyOptimizedPair<std::vector<char>, std::vector<char>> pair2(
        rad::Move(pair1));

    EXPECT_TRUE(pair1.First().empty());
    EXPECT_EQ(pair2.First().size(), 1u);
    EXPECT_EQ(pair2.First().front(), 1);

    EXPECT_TRUE(pair1.Second().empty());
    EXPECT_EQ(pair2.Second().size(), 1u);
    EXPECT_EQ(pair2.Second().front(), 2);
}
