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

//
// Disable nothrow assertions for unit testing
//
#define RAD_ENABLE_NOTHROW_ASSERTIONS 0

#include "gtest/gtest.h"

#include "radiant/Utility.h"

#include "test/TestMove.h"

RAD_S_ASSERT(noexcept(rad::Forward<int>(rad::DeclVal<int>())));
RAD_S_ASSERT(noexcept(rad::Forward<int>(rad::DeclVal<int&>())));
RAD_S_ASSERT(noexcept(rad::Forward<int>(rad::DeclVal<int&&>())));
RAD_S_ASSERT(noexcept(rad::Move(rad::DeclVal<int>())));
RAD_S_ASSERT(noexcept(rad::Move(rad::DeclVal<int&&>())));
RAD_S_ASSERT(noexcept(rad::MoveIfNoExcept(rad::DeclVal<int&>())));

TEST(Utility, Move)
{
    radtest::MoveTester::ResetCounts();
    radtest::MoveTester v1;
    v1.value = 123;
    radtest::MoveTester v2;
    v2.value = 456;

    v2 = rad::Move(v1);
    EXPECT_EQ(v2.value, 123u);
    EXPECT_EQ(radtest::MoveTester::MoveCount(), 1u);

    radtest::MoveTester v3 = rad::Move(v2);
    EXPECT_EQ(v3.value, 123u);
    EXPECT_EQ(radtest::MoveTester::MoveCount(), 2u);
}

TEST(Utility, MoveIfNoExcept)
{
    radtest::MoveTester::ResetCounts();
    radtest::MoveTester v1;
    v1.value = 123;
    radtest::MoveTester v2;
    v2.value = 456;

    v2 = rad::MoveIfNoExcept(v1);
    EXPECT_EQ(v2.value, 123u);
    EXPECT_EQ(radtest::MoveTester::MoveCount(), 1u);

    radtest::MoveTester v3 = rad::MoveIfNoExcept(v2);
    EXPECT_EQ(v3.value, 123u);
    EXPECT_EQ(radtest::MoveTester::MoveCount(), 2u);
}
