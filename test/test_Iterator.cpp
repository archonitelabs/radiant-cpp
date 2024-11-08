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

#include "radiant/Iterator.h"

struct Data
{
    int value;
    bool boolean;
};

const Data g_data[] = {
    {  0,  true },
    {  1, false },
    {  2,  true },
    {  3, false },
    {  4,  true },
    {  5, false },
    {  6,  true },
    {  7, false },
    {  8,  true },
    {  9, false },
    { 10,  true },
};
using It = rad::Iterator<const Data*>;
using RIt = rad::ReverseIterator<It>;

TEST(IteratorTest, DefaultConstruct)
{
    It it;
    EXPECT_EQ(it.operator->(), nullptr);

    RIt rit;
    RAD_UNUSED(rit);
    // UB
    // EXPECT_EQ(rit.operator->(), nullptr);
}

TEST(InteratorTest, ValueConstruct)
{
    It it(g_data);
    EXPECT_EQ(it.operator->(), g_data);

    RIt rit(g_data + 1);
    EXPECT_EQ(rit.operator->(), g_data);
}

TEST(InteratorTest, CopyConstruct)
{
    It it(g_data);
    It itOther(it);
    EXPECT_EQ(it.Base(), itOther.Base());

    RIt rit(g_data);
    RIt ritOther(rit);
    EXPECT_EQ(rit.Base(), ritOther.Base());
}

TEST(InteratorTest, DereferenceOperator)
{
    It it(g_data);
    EXPECT_EQ(&it.operator*(), g_data);

    RIt rit(g_data + 1);
    EXPECT_EQ(&rit.operator*(), g_data);
}

TEST(InteratorTest, SubscriptOperator)
{
    It it(g_data);
    EXPECT_EQ(&it.operator[](0), g_data);
    EXPECT_EQ(&it.operator[](1), g_data + 1);

    RIt rit(g_data + 2);
    EXPECT_EQ(&rit.operator[](0), g_data + 1);
    EXPECT_EQ(&rit.operator[](1), g_data);
}

TEST(InteratorTest, Increment)
{
    It it(g_data);
    it++;
    EXPECT_EQ(it.operator->(), g_data + 1);

    RIt rit(g_data + 2);
    rit++;
    EXPECT_EQ(rit.operator->(), g_data);
}

TEST(InteratorTest, PreIncrement)
{
    It it(g_data);
    ++it;
    EXPECT_EQ(it.operator->(), g_data + 1);

    RIt rit(g_data + 2);
    ++rit;
    EXPECT_EQ(rit.operator->(), g_data);
}

TEST(InteratorTest, Decrement)
{
    It it(g_data + 1);
    it--;
    EXPECT_EQ(it.operator->(), g_data);

    RIt rit(g_data + 1);
    rit--;
    EXPECT_EQ(rit.operator->(), g_data + 1);
}

TEST(InteratorTest, PreDecrement)
{
    It it(g_data + 1);
    --it;
    EXPECT_EQ(it.operator->(), g_data);

    RIt rit(g_data + 1);
    --rit;
    EXPECT_EQ(rit.operator->(), g_data + 1);
}

TEST(InteratorTest, CompoundIncrement)
{
    It it(g_data);
    it += 1;
    EXPECT_EQ(it.operator->(), g_data + 1);

    RIt rit(g_data + 2);
    rit += 1;
    EXPECT_EQ(rit.operator->(), g_data);
}

TEST(InteratorTest, CompoundDecrement)
{
    It it(g_data + 1);
    it -= 1;
    EXPECT_EQ(it.operator->(), g_data);

    RIt rit(g_data + 1);
    rit -= 1;
    EXPECT_EQ(rit.operator->(), g_data + 1);
}

TEST(InteratorTest, AdditionOperator)
{
    It it(g_data);
    it = it + 1;
    EXPECT_EQ(it.operator->(), g_data + 1);

    RIt rit(g_data + 2);
    rit = rit + 1;
    EXPECT_EQ(rit.operator->(), g_data);
}

TEST(InteratorTest, SubtractionOperator)
{
    It it(g_data + 1);
    it = it - 1;
    EXPECT_EQ(it.operator->(), g_data);

    RIt rit(g_data + 1);
    rit = rit - 1;
    EXPECT_EQ(rit.operator->(), g_data + 1);
}

TEST(InteratorTest, Base)
{
    It it(g_data);
    EXPECT_EQ(it.Base(), g_data);
    EXPECT_EQ(it.base(), g_data);

    RIt rit(g_data);
    EXPECT_EQ(rit.Base(), it);
    EXPECT_EQ(rit.base(), it);
}
