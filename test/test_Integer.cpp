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

#include "radiant/Integer.h"

// clang-format off

RAD_S_ASSERT(noexcept(rad::i8()));
RAD_S_ASSERT(noexcept(rad::i8(1)));
RAD_S_ASSERT(noexcept(rad::i8(rad::DeclVal<const rad::i8&>())));
RAD_S_ASSERT(noexcept(rad::i8(rad::DeclVal<rad::i8&&>())));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i8>().operator=(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i8>().operator=(rad::DeclVal<const rad::i8&>())));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i8>().operator=(rad::DeclVal<rad::i8&&>())));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i8>().operator int8_t()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i8>().Max(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i8>().Min(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i8>().Add(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i8>().Sub(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i8>().Mul(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i8>().SaturatingAdd(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i8>().SaturatingSub(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i8>().SaturatingMul(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i8>().UncheckedAdd(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i8>().UncheckedSub(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i8>().UncheckedMul(1)));

RAD_S_ASSERT(noexcept(rad::u8()));
RAD_S_ASSERT(noexcept(rad::u8(1)));
RAD_S_ASSERT(noexcept(rad::u8(rad::DeclVal<const rad::u8&>())));
RAD_S_ASSERT(noexcept(rad::u8(rad::DeclVal<rad::u8&&>())));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u8>().operator=(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u8>().operator=(rad::DeclVal<const rad::u8&>())));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u8>().operator=(rad::DeclVal<rad::u8&&>())));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u8>().operator uint8_t()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u8>().Max(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u8>().Min(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u8>().Add(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u8>().Sub(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u8>().Mul(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u8>().SaturatingAdd(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u8>().SaturatingSub(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u8>().SaturatingMul(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u8>().UncheckedAdd(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u8>().UncheckedSub(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u8>().UncheckedMul(1)));

RAD_S_ASSERT(noexcept(rad::i16()));
RAD_S_ASSERT(noexcept(rad::i16(1)));
RAD_S_ASSERT(noexcept(rad::i16(rad::DeclVal<const rad::i16&>())));
RAD_S_ASSERT(noexcept(rad::i16(rad::DeclVal<rad::i16&&>())));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i16>().operator=(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i16>().operator=(rad::DeclVal<const rad::i16&>())));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i16>().operator=(rad::DeclVal<rad::i16&&>())));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i16>().operator int16_t()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i16>().Max(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i16>().Min(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i16>().Add(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i16>().Sub(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i16>().Mul(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i16>().SaturatingAdd(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i16>().SaturatingSub(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i16>().SaturatingMul(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i16>().UncheckedAdd(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i16>().UncheckedSub(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i16>().UncheckedMul(1)));

RAD_S_ASSERT(noexcept(rad::u16()));
RAD_S_ASSERT(noexcept(rad::u16(1)));
RAD_S_ASSERT(noexcept(rad::u16(rad::DeclVal<const rad::u16&>())));
RAD_S_ASSERT(noexcept(rad::u16(rad::DeclVal<rad::u16&&>())));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u16>().operator=(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u16>().operator=(rad::DeclVal<const rad::u16&>())));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u16>().operator=(rad::DeclVal<rad::u16&&>())));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u16>().operator uint16_t()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u16>().Max(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u16>().Min(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u16>().Add(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u16>().Sub(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u16>().Mul(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u16>().SaturatingAdd(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u16>().SaturatingSub(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u16>().SaturatingMul(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u16>().UncheckedAdd(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u16>().UncheckedSub(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u16>().UncheckedMul(1)));

RAD_S_ASSERT(noexcept(rad::i32()));
RAD_S_ASSERT(noexcept(rad::i32(1)));
RAD_S_ASSERT(noexcept(rad::i32(rad::DeclVal<const rad::i32&>())));
RAD_S_ASSERT(noexcept(rad::i32(rad::DeclVal<rad::i32&&>())));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i32>().operator=(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i32>().operator=(rad::DeclVal<const rad::i32&>())));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i32>().operator=(rad::DeclVal<rad::i32&&>())));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i32>().operator int32_t()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i32>().Max(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i32>().Min(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i32>().Add(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i32>().Sub(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i32>().Mul(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i32>().SaturatingAdd(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i32>().SaturatingSub(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i32>().SaturatingMul(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i32>().UncheckedAdd(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i32>().UncheckedSub(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::i32>().UncheckedMul(1)));

RAD_S_ASSERT(noexcept(rad::u32()));
RAD_S_ASSERT(noexcept(rad::u32(1)));
RAD_S_ASSERT(noexcept(rad::u32(rad::DeclVal<const rad::u32&>())));
RAD_S_ASSERT(noexcept(rad::u32(rad::DeclVal<rad::u32&&>())));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u32>().operator=(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u32>().operator=(rad::DeclVal<const rad::u32&>())));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u32>().operator=(rad::DeclVal<rad::u32&&>())));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u32>().operator uint32_t()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u32>().Max(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u32>().Min(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u32>().Add(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u32>().Sub(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u32>().Mul(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u32>().SaturatingAdd(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u32>().SaturatingSub(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u32>().SaturatingMul(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u32>().UncheckedAdd(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u32>().UncheckedSub(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::u32>().UncheckedMul(1)));

// clang-format on

RAD_S_ASSERT(rad::i8::MIN == INT8_MIN);
RAD_S_ASSERT(rad::i8::MAX == INT8_MAX);
RAD_S_ASSERT(rad::u8::MIN == 0);
RAD_S_ASSERT(rad::u8::MAX == UINT8_MAX);
RAD_S_ASSERT(rad::i16::MIN == INT16_MIN);
RAD_S_ASSERT(rad::i16::MAX == INT16_MAX);
RAD_S_ASSERT(rad::u16::MIN == 0);
RAD_S_ASSERT(rad::u16::MAX == UINT16_MAX);
RAD_S_ASSERT(rad::i32::MIN == INT32_MIN);
RAD_S_ASSERT(rad::i32::MAX == INT32_MAX);
RAD_S_ASSERT(rad::u32::MIN == 0);
RAD_S_ASSERT(rad::u32::MAX == UINT32_MAX);

RAD_S_ASSERT(rad::i8() == int8_t());
RAD_S_ASSERT(rad::u8() == uint8_t());
RAD_S_ASSERT(rad::i16() == int16_t());
RAD_S_ASSERT(rad::u16() == uint16_t());
RAD_S_ASSERT(rad::i32() == int32_t());
RAD_S_ASSERT(rad::u32() == uint32_t());

RAD_S_ASSERT(rad::i8(1) == 1);
RAD_S_ASSERT(rad::u8(1) == 1);
RAD_S_ASSERT(rad::i16(1) == 1);
RAD_S_ASSERT(rad::u16(1) == 1);
RAD_S_ASSERT(rad::i32(1) == 1);
RAD_S_ASSERT(rad::u32(1) == 1u);

RAD_S_ASSERT(rad::i8(rad::i8(1)) == 1);
RAD_S_ASSERT(rad::u8(rad::u8(1)) == 1);
RAD_S_ASSERT(rad::i16(rad::i16(1)) == 1);
RAD_S_ASSERT(rad::u16(rad::u16(1)) == 1);
RAD_S_ASSERT(rad::i32(rad::i32(1)) == 1);
RAD_S_ASSERT(rad::u32(rad::u32(1)) == 1u);

//
// Construct
//

TEST(IntegerTests, i8_Construct)
{
    rad::i8 a;
    EXPECT_EQ(a, int8_t());

    rad::i8 b(1);
    EXPECT_EQ(b, 1);

    rad::i8 c(b);
    EXPECT_EQ(c, 1);

    rad::i8 d(rad::Move(c));
    EXPECT_EQ(d, 1);
}

TEST(IntegerTests, u8_Construct)
{
    rad::u8 a;
    EXPECT_EQ(a, uint8_t());

    rad::u8 b(1);
    EXPECT_EQ(b, 1);

    rad::u8 c(b);
    EXPECT_EQ(c, 1);

    rad::u8 d(rad::Move(c));
    EXPECT_EQ(d, 1);
}

TEST(IntegerTests, i16_Construct)
{
    rad::i16 a;
    EXPECT_EQ(a, int16_t());

    rad::i16 b(1);
    EXPECT_EQ(b, 1);

    rad::i16 c(b);
    EXPECT_EQ(c, 1);

    rad::i16 d(rad::Move(c));
    EXPECT_EQ(d, 1);
}

TEST(IntegerTests, u16_Construct)
{
    rad::u16 a;
    EXPECT_EQ(a, uint16_t());

    rad::u16 b(1);
    EXPECT_EQ(b, 1);

    rad::u16 c(b);
    EXPECT_EQ(c, 1);

    rad::u16 d(rad::Move(c));
    EXPECT_EQ(d, 1);
}

TEST(IntegerTests, i32_Construct)
{
    rad::i32 a;
    EXPECT_EQ(a, int32_t());

    rad::i32 b(1);
    EXPECT_EQ(b, 1);

    rad::i32 c(b);
    EXPECT_EQ(c, 1);

    rad::i32 d(rad::Move(c));
    EXPECT_EQ(d, 1);
}

TEST(IntegerTests, u32_Construct)
{
    rad::u32 a;
    EXPECT_EQ(a, uint32_t());

    rad::u32 b(1);
    EXPECT_EQ(b, 1u);

    rad::u32 c(b);
    EXPECT_EQ(c, 1u);

    rad::u32 d(rad::Move(c));
    EXPECT_EQ(d, 1u);
}

//
// Assign
//

TEST(IntegerTests, i8_Assign)
{
    rad::i8 a;
    a = 1;
    EXPECT_EQ(a, 1);

    rad::i8 b;
    b = a;
    EXPECT_EQ(b, 1);

    rad::i8 c;
    c = rad::i8(1);
    EXPECT_EQ(c, 1);
}

TEST(IntegerTests, u8_Assign)
{
    rad::u8 a;
    a = 1;
    EXPECT_EQ(a, 1);

    rad::u8 b;
    b = a;
    EXPECT_EQ(b, 1);

    rad::u8 c;
    c = rad::u8(1);
    EXPECT_EQ(c, 1);
}

TEST(IntegerTests, i16_Assign)
{
    rad::i16 a;
    a = 1;
    EXPECT_EQ(a, 1);

    rad::i16 b;
    b = a;
    EXPECT_EQ(b, 1);

    rad::i16 c;
    c = rad::i16(1);
    EXPECT_EQ(c, 1);
}

TEST(IntegerTests, u16_Assign)
{
    rad::u16 a;
    a = 1;
    EXPECT_EQ(a, 1);

    rad::u16 b;
    b = a;
    EXPECT_EQ(b, 1);

    rad::u16 c;
    c = rad::u16(1);
    EXPECT_EQ(c, 1);
}

TEST(IntegerTests, i32_Assign)
{
    rad::i32 a;
    a = 1;
    EXPECT_EQ(a, 1);

    rad::i32 b;
    b = a;
    EXPECT_EQ(b, 1);

    rad::i32 c;
    c = rad::i32(1);
    EXPECT_EQ(c, 1);
}

TEST(IntegerTests, u32_Assign)
{
    rad::u32 a;
    a = 1;
    EXPECT_EQ(a, 1u);

    rad::u32 b;
    b = a;
    EXPECT_EQ(b, 1u);

    rad::u32 c;
    c = rad::u32(1);
    EXPECT_EQ(c, 1u);
}

//
// Min/Max
//

TEST(IntegerTests, i8_MinMax)
{
    EXPECT_EQ(rad::i8(1).Max(2), 2);
    EXPECT_EQ(rad::i8(2).Max(1), 2);
    EXPECT_EQ(rad::i8(1).Min(2), 1);
    EXPECT_EQ(rad::i8(2).Min(1), 1);

    EXPECT_EQ(rad::i8(-1).Max(-2), -1);
    EXPECT_EQ(rad::i8(-2).Max(-1), -1);
    EXPECT_EQ(rad::i8(-1).Min(-2), -2);
    EXPECT_EQ(rad::i8(-2).Min(-1), -2);
}

TEST(IntegerTests, u8_MinMax)
{
    EXPECT_EQ(rad::u8(1).Max(2), 2);
    EXPECT_EQ(rad::u8(2).Max(1), 2);
    EXPECT_EQ(rad::u8(1).Min(2), 1);
    EXPECT_EQ(rad::u8(2).Min(1), 1);
}

TEST(IntegerTests, i16_MinMax)
{
    EXPECT_EQ(rad::i16(1).Max(2), 2);
    EXPECT_EQ(rad::i16(2).Max(1), 2);
    EXPECT_EQ(rad::i16(1).Min(2), 1);
    EXPECT_EQ(rad::i16(2).Min(1), 1);

    EXPECT_EQ(rad::i16(-1).Max(-2), -1);
    EXPECT_EQ(rad::i16(-2).Max(-1), -1);
    EXPECT_EQ(rad::i16(-1).Min(-2), -2);
    EXPECT_EQ(rad::i16(-2).Min(-1), -2);
}

TEST(IntegerTests, u16_MinMax)
{
    EXPECT_EQ(rad::u16(1).Max(2), 2);
    EXPECT_EQ(rad::u16(2).Max(1), 2);
    EXPECT_EQ(rad::u16(1).Min(2), 1);
    EXPECT_EQ(rad::u16(2).Min(1), 1);
}

TEST(IntegerTests, i32_MinMax)
{
    EXPECT_EQ(rad::i32(1).Max(2), 2);
    EXPECT_EQ(rad::i32(2).Max(1), 2);
    EXPECT_EQ(rad::i32(1).Min(2), 1);
    EXPECT_EQ(rad::i32(2).Min(1), 1);

    EXPECT_EQ(rad::i32(-1).Max(-2), -1);
    EXPECT_EQ(rad::i32(-2).Max(-1), -1);
    EXPECT_EQ(rad::i32(-1).Min(-2), -2);
    EXPECT_EQ(rad::i32(-2).Min(-1), -2);
}

TEST(IntegerTests, u32_MinMax)
{
    EXPECT_EQ(rad::u32(1).Max(2), 2u);
    EXPECT_EQ(rad::u32(2).Max(1), 2u);
    EXPECT_EQ(rad::u32(1).Min(2), 1u);
    EXPECT_EQ(rad::u32(2).Min(1), 1u);
}

//
// Add
//

TEST(IntegerTests, i8_Add)
{
    rad::Res<rad::i8> a = rad::i8();

    a = a.Ok().Add(1);
    EXPECT_EQ(a, rad::i8(1));

    a = a.Ok().Add(rad::i8::MAX);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);

    a = rad::i8();

    a = a.Ok().Add(-1);
    EXPECT_EQ(a, rad::i8(-1));

    a = a.Ok().Add(rad::i8::MIN);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);
}

TEST(IntegerTests, u8_Add)
{
    rad::Res<rad::u8> a = rad::u8();

    a = a.Ok().Add(1);
    EXPECT_EQ(a, rad::u8(1));

    a = a.Ok().Add(rad::u8::MAX);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);
}

TEST(IntegerTests, i16_Add)
{
    rad::Res<rad::i16> a = rad::i16();

    a = a.Ok().Add(1);
    EXPECT_EQ(a, rad::i16(1));

    a = a.Ok().Add(rad::i16::MAX);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);

    a = rad::i16();

    a = a.Ok().Add(-1);
    EXPECT_EQ(a, rad::i16(-1));

    a = a.Ok().Add(rad::i16::MIN);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);
}

TEST(IntegerTests, u16_Add)
{
    rad::Res<rad::u16> a = rad::u16();

    a = a.Ok().Add(1);
    EXPECT_EQ(a, rad::u16(1));

    a = a.Ok().Add(rad::u16::MAX);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);
}

TEST(IntegerTests, i32_Add)
{
    rad::Res<rad::i32> a = rad::i32();

    a = a.Ok().Add(1);
    EXPECT_EQ(a, rad::i32(1));

    a = a.Ok().Add(rad::i32::MAX);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);

    a = rad::i32();

    a = a.Ok().Add(-1);
    EXPECT_EQ(a, rad::i32(-1));

    a = a.Ok().Add(rad::i32::MIN);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);
}

TEST(IntegerTests, u32_Add)
{
    rad::Res<rad::u32> a = rad::u32();

    a = a.Ok().Add(1);
    EXPECT_EQ(a, rad::u32(1));

    a = a.Ok().Add(rad::u32::MAX);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);
}

//
// Sub
//

TEST(IntegerTests, i8_Sub)
{
    rad::Res<rad::i8> a = rad::i8();

    a = a.Ok().Sub(1);
    EXPECT_EQ(a, rad::i8(-1));

    a = a.Ok().Sub(1).Ok().Sub(rad::i8::MAX);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);

    a = rad::i8();

    a = a.Ok().Sub(-1);
    EXPECT_EQ(a, rad::i8(1));

    a = a.Ok().Sub(rad::i8::MIN);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);
}

TEST(IntegerTests, u8_Sub)
{
    rad::Res<rad::u8> a = rad::u8();

    a = a.Ok().Sub(1);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);

    a = rad::u8(1);

    a = a.Ok().Sub(1);
    EXPECT_EQ(a, rad::u8(0));
}

TEST(IntegerTests, i16_Sub)
{
    rad::Res<rad::i16> a = rad::i16();

    a = a.Ok().Sub(1);
    EXPECT_EQ(a, rad::i16(-1));

    a = a.Ok().Sub(1).Ok().Sub(rad::i16::MAX);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);

    a = rad::i16();

    a = a.Ok().Sub(-1);
    EXPECT_EQ(a, rad::i16(1));

    a = a.Ok().Sub(rad::i16::MIN);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);
}

TEST(IntegerTests, u16_Sub)
{
    rad::Res<rad::u16> a = rad::u16();

    a = a.Ok().Sub(1);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);

    a = rad::u16(1);

    a = a.Ok().Sub(1);
    EXPECT_EQ(a, rad::u16(0));
}

TEST(IntegerTests, i32_Sub)
{
    rad::Res<rad::i32> a = rad::i32();

    a = a.Ok().Sub(1);
    EXPECT_EQ(a, rad::i32(-1));

    a = a.Ok().Sub(1).Ok().Sub(rad::i32::MAX);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);

    a = rad::i32();

    a = a.Ok().Sub(-1);
    EXPECT_EQ(a, rad::i32(1));

    a = a.Ok().Sub(rad::i32::MIN);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);
}

TEST(IntegerTests, u32_Sub)
{
    rad::Res<rad::u32> a = rad::u32();

    a = a.Ok().Sub(1);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);

    a = rad::u32(1);

    a = a.Ok().Sub(1);
    EXPECT_EQ(a, rad::u32(0));
}

//
// Mul
//

TEST(IntegerTests, i8_Mul)
{
    rad::Res<rad::i8> a = rad::i8(10);

    a = a.Ok().Mul(10);
    EXPECT_EQ(a, rad::i8(100));

    a = a.Ok().Mul(-1);
    EXPECT_EQ(a, rad::i8(-100));

    a = a.Ok().Mul(rad::i8::MAX);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);

    a = rad::i8(10);

    a = a.Ok().Mul(rad::i8::MAX);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);
}

TEST(IntegerTests, u8_Mul)
{
    rad::Res<rad::u8> a = rad::u8(10);

    a = a.Ok().Mul(10);
    EXPECT_EQ(a, rad::u8(100));

    a = a.Ok().Mul(rad::u8::MAX);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);
}

TEST(IntegerTests, i16_Mul)
{
    rad::Res<rad::i16> a = rad::i16(10);

    a = a.Ok().Mul(10);
    EXPECT_EQ(a, rad::i16(100));

    a = a.Ok().Mul(-1);
    EXPECT_EQ(a, rad::i16(-100));

    a = a.Ok().Mul(rad::i16::MAX);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);

    a = rad::i16(10);

    a = a.Ok().Mul(rad::i16::MAX);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);
}

TEST(IntegerTests, u16_Mul)
{
    rad::Res<rad::u16> a = rad::u16(10);

    a = a.Ok().Mul(10);
    EXPECT_EQ(a, rad::u16(100));

    a = a.Ok().Mul(rad::u16::MAX);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);
}

TEST(IntegerTests, i32_Mul)
{
    rad::Res<rad::i32> a = rad::i32(10);

    a = a.Ok().Mul(10);
    EXPECT_EQ(a, rad::i32(100));

    a = a.Ok().Mul(-1);
    EXPECT_EQ(a, rad::i32(-100));

    a = a.Ok().Mul(rad::i32::MAX);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);

    a = rad::i32(10);

    a = a.Ok().Mul(rad::i32::MAX);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);
}

TEST(IntegerTests, u32_Mul)
{
    rad::Res<rad::u32> a = rad::u32(10);

    a = a.Ok().Mul(10);
    EXPECT_EQ(a, rad::u32(100));

    a = a.Ok().Mul(rad::u32::MAX);
    EXPECT_EQ(a, rad::Error::IntegerOverflow);
}

//
// SaturatingAdd
//

TEST(IntegerTests, i8_SaturatingAdd)
{
    rad::i8 a = 10;

    a = a.SaturatingAdd(10);
    EXPECT_EQ(a, 20);

    a = a.SaturatingAdd(rad::i8::MAX);
    EXPECT_EQ(a, rad::i8::MAX);

    a = a.SaturatingAdd(rad::i8::MIN);
    EXPECT_EQ(a, -1);

    a = a.SaturatingAdd(rad::i8::MIN);
    EXPECT_EQ(a, rad::i8::MIN);
}

TEST(IntegerTests, u8_SaturatingAdd)
{
    rad::u8 a = 10;

    a = a.SaturatingAdd(10);
    EXPECT_EQ(a, 20);

    a = a.SaturatingAdd(rad::u8::MAX);
    EXPECT_EQ(a, rad::u8::MAX);
}

TEST(IntegerTests, i16_SaturatingAdd)
{
    rad::i16 a = 10;

    a = a.SaturatingAdd(10);
    EXPECT_EQ(a, 20);

    a = a.SaturatingAdd(rad::i16::MAX);
    EXPECT_EQ(a, rad::i16::MAX);

    a = a.SaturatingAdd(rad::i16::MIN);
    EXPECT_EQ(a, -1);

    a = a.SaturatingAdd(rad::i16::MIN);
    EXPECT_EQ(a, rad::i16::MIN);
}

TEST(IntegerTests, u16_SaturatingAdd)
{
    rad::u16 a = 10;

    a = a.SaturatingAdd(10);
    EXPECT_EQ(a, 20);

    a = a.SaturatingAdd(rad::u16::MAX);
    EXPECT_EQ(a, rad::u16::MAX);
}

TEST(IntegerTests, i32_SaturatingAdd)
{
    rad::i32 a = 10;

    a = a.SaturatingAdd(10);
    EXPECT_EQ(a, 20);

    a = a.SaturatingAdd(rad::i32::MAX);
    EXPECT_EQ(a, rad::i32::MAX);

    a = a.SaturatingAdd(rad::i32::MIN);
    EXPECT_EQ(a, -1);

    a = a.SaturatingAdd(rad::i32::MIN);
    EXPECT_EQ(a, rad::i32::MIN);
}

TEST(IntegerTests, u32_SaturatingAdd)
{
    rad::u32 a = 10;

    a = a.SaturatingAdd(10);
    EXPECT_EQ(a, 20u);

    a = a.SaturatingAdd(rad::u32::MAX);
    EXPECT_EQ(a, rad::u32::MAX);
}

//
// SaturatingSub
//

TEST(IntegerTests, i8_SaturatingSub)
{
    rad::i8 a = 0;

    a = a.SaturatingSub(10);
    EXPECT_EQ(a, -10);

    a = a.SaturatingSub(rad::i8::MAX);
    EXPECT_EQ(a, rad::i8::MIN);

    a = a.SaturatingSub(-1);
    EXPECT_EQ(a, rad::i8::MIN + 1);

    a = a.SaturatingSub(rad::i8::MIN);
    EXPECT_EQ(a, 1);

    a = a.SaturatingSub(rad::i8::MIN);
    EXPECT_EQ(a, rad::i8::MAX);
}

TEST(IntegerTests, u8_SaturatingSub)
{
    rad::u8 a = 10;

    a = a.SaturatingSub(5);
    EXPECT_EQ(a, 5);

    a = a.SaturatingSub(10);
    EXPECT_EQ(a, 0);
}

TEST(IntegerTests, i16_SaturatingSub)
{
    rad::i16 a = 0;

    a = a.SaturatingSub(10);
    EXPECT_EQ(a, -10);

    a = a.SaturatingSub(rad::i16::MAX);
    EXPECT_EQ(a, rad::i16::MIN);

    a = a.SaturatingSub(-1);
    EXPECT_EQ(a, rad::i16::MIN + 1);

    a = a.SaturatingSub(rad::i16::MIN);
    EXPECT_EQ(a, 1);

    a = a.SaturatingSub(rad::i16::MIN);
    EXPECT_EQ(a, rad::i16::MAX);
}

TEST(IntegerTests, u16_SaturatingSub)
{
    rad::u16 a = 10;

    a = a.SaturatingSub(5);
    EXPECT_EQ(a, 5);

    a = a.SaturatingSub(10);
    EXPECT_EQ(a, 0);
}

TEST(IntegerTests, i32_SaturatingSub)
{
    rad::i32 a = 0;

    a = a.SaturatingSub(10);
    EXPECT_EQ(a, -10);

    a = a.SaturatingSub(rad::i32::MAX);
    EXPECT_EQ(a, rad::i32::MIN);

    a = a.SaturatingSub(-1);
    EXPECT_EQ(a, rad::i32::MIN + 1);

    a = a.SaturatingSub(rad::i32::MIN);
    EXPECT_EQ(a, 1);

    a = a.SaturatingSub(rad::i32::MIN);
    EXPECT_EQ(a, rad::i32::MAX);
}

TEST(IntegerTests, u32_SaturatingSub)
{
    rad::u32 a = 10;

    a = a.SaturatingSub(5);
    EXPECT_EQ(a, 5u);

    a = a.SaturatingSub(10);
    EXPECT_EQ(a, 0u);
}

//
// SaturatingMul
//

TEST(IntegerTests, i8_SaturatingMul)
{
    rad::i8 a = 10;

    a = a.SaturatingMul(10);
    EXPECT_EQ(a, 100);

    a = a.SaturatingMul(-1);
    EXPECT_EQ(a, -100);

    a = a.SaturatingMul(rad::i8::MAX);
    EXPECT_EQ(a, rad::i8::MIN);

    a = a.SaturatingMul(rad::i8::MIN);
    EXPECT_EQ(a, rad::i8::MAX);

    a = a.SaturatingMul(rad::i8::MAX);
    EXPECT_EQ(a, rad::i8::MAX);
}

TEST(IntegerTests, u8_SaturatingMul)
{
    rad::u8 a = 10;

    a = a.SaturatingMul(10);
    EXPECT_EQ(a, 100);

    a = a.SaturatingMul(rad::u8::MAX);
    EXPECT_EQ(a, rad::u8::MAX);
}

TEST(IntegerTests, i16_SaturatingMul)
{
    rad::i16 a = 10;

    a = a.SaturatingMul(10);
    EXPECT_EQ(a, 100);

    a = a.SaturatingMul(-1);
    EXPECT_EQ(a, -100);

    a = a.SaturatingMul(rad::i16::MAX);
    EXPECT_EQ(a, rad::i16::MIN);

    a = a.SaturatingMul(rad::i16::MIN);
    EXPECT_EQ(a, rad::i16::MAX);

    a = a.SaturatingMul(rad::i16::MAX);
    EXPECT_EQ(a, rad::i16::MAX);
}

TEST(IntegerTests, u16_SaturatingMul)
{
    rad::u16 a = 10;

    a = a.SaturatingMul(10);
    EXPECT_EQ(a, 100);

    a = a.SaturatingMul(rad::u16::MAX);
    EXPECT_EQ(a, rad::u16::MAX);
}

TEST(IntegerTests, i32_SaturatingMul)
{
    rad::i32 a = 10;

    a = a.SaturatingMul(10);
    EXPECT_EQ(a, 100);

    a = a.SaturatingMul(-1);
    EXPECT_EQ(a, -100);

    a = a.SaturatingMul(rad::i32::MAX);
    EXPECT_EQ(a, rad::i32::MIN);

    a = a.SaturatingMul(rad::i32::MIN);
    EXPECT_EQ(a, rad::i32::MAX);

    a = a.SaturatingMul(rad::i32::MAX);
    EXPECT_EQ(a, rad::i32::MAX);
}

TEST(IntegerTests, u32_SaturatingMul)
{
    rad::u32 a = 10;

    a = a.SaturatingMul(10);
    EXPECT_EQ(a, 100u);

    a = a.SaturatingMul(rad::u32::MAX);
    EXPECT_EQ(a, rad::u32::MAX);
}

//
// Unchecked
//

TEST(IntegerTests, UncheckedAdd)
{
    EXPECT_EQ(rad::i8(rad::i8::MAX).UncheckedAdd(1), rad::i8::MIN);
    EXPECT_EQ(rad::u8(rad::u8::MAX).UncheckedAdd(1), rad::u8::MIN);
    EXPECT_EQ(rad::i16(rad::i16::MAX).UncheckedAdd(1), rad::i16::MIN);
    EXPECT_EQ(rad::u16(rad::u16::MAX).UncheckedAdd(1), rad::u16::MIN);
}

TEST(IntegerTests, UncheckedSub)
{
    EXPECT_EQ(rad::i8(rad::i8::MIN).UncheckedSub(1), rad::i8::MAX);
    EXPECT_EQ(rad::u8(rad::u8::MIN).UncheckedSub(1), rad::u8::MAX);
    EXPECT_EQ(rad::i16(rad::i16::MIN).UncheckedSub(1), rad::i16::MAX);
    EXPECT_EQ(rad::u16(rad::u16::MIN).UncheckedSub(1), rad::u16::MAX);
}

TEST(IntegerTests, UncheckedMul)
{
    EXPECT_EQ(rad::i8(rad::i8::MAX).UncheckedMul(2), -2);
    EXPECT_EQ(rad::u8(rad::u8::MAX).UncheckedMul(2), 254);
    EXPECT_EQ(rad::i16(rad::i16::MAX).UncheckedMul(2), -2);
    EXPECT_EQ(rad::u16(rad::u16::MAX).UncheckedMul(2), 65534);
    EXPECT_EQ(rad::i32(rad::i32::MAX).UncheckedMul(2), -2);
    EXPECT_EQ(rad::u32(rad::u32::MAX).UncheckedMul(2), 4294967294u);
}

//
// Compare operators
//

TEST(IntegerTests, i8_Compare)
{
    EXPECT_TRUE(rad::i8(1) == rad::i8(1));
    EXPECT_TRUE(rad::i8(1) != rad::i8(2));
    EXPECT_TRUE(rad::i8(1) < rad::i8(2));
    EXPECT_TRUE(rad::i8(1) <= rad::i8(2));
    EXPECT_TRUE(rad::i8(2) > rad::i8(1));
    EXPECT_TRUE(rad::i8(2) >= rad::i8(1));

    EXPECT_TRUE(rad::i8(1) == int8_t(1));
    EXPECT_TRUE(rad::i8(1) != int8_t(2));
    EXPECT_TRUE(rad::i8(1) < int8_t(2));
    EXPECT_TRUE(rad::i8(1) <= int8_t(2));
    EXPECT_TRUE(rad::i8(2) > int8_t(1));
    EXPECT_TRUE(rad::i8(2) >= int8_t(1));

    EXPECT_TRUE(rad::i8(1) == static_cast<signed int>(1));
    EXPECT_TRUE(rad::i8(1) != static_cast<signed int>(2));
    EXPECT_TRUE(rad::i8(1) < static_cast<signed int>(2));
    EXPECT_TRUE(rad::i8(1) <= static_cast<signed int>(2));
    EXPECT_TRUE(rad::i8(2) > static_cast<signed int>(1));
    EXPECT_TRUE(rad::i8(2) >= static_cast<signed int>(1));
}

TEST(IntegerTests, u8_Compare)
{
    EXPECT_TRUE(rad::u8(1) == rad::u8(1));
    EXPECT_TRUE(rad::u8(1) != rad::u8(2));
    EXPECT_TRUE(rad::u8(1) < rad::u8(2));
    EXPECT_TRUE(rad::u8(1) <= rad::u8(2));
    EXPECT_TRUE(rad::u8(2) > rad::u8(1));
    EXPECT_TRUE(rad::u8(2) >= rad::u8(1));

    EXPECT_TRUE(rad::u8(1) == uint8_t(1));
    EXPECT_TRUE(rad::u8(1) != uint8_t(2));
    EXPECT_TRUE(rad::u8(1) < uint8_t(2));
    EXPECT_TRUE(rad::u8(1) <= uint8_t(2));
    EXPECT_TRUE(rad::u8(2) > uint8_t(1));
    EXPECT_TRUE(rad::u8(2) >= uint8_t(1));

    EXPECT_TRUE(rad::u8(1) == static_cast<unsigned int>(1));
    EXPECT_TRUE(rad::u8(1) != static_cast<unsigned int>(2));
    EXPECT_TRUE(rad::u8(1) < static_cast<unsigned int>(2));
    EXPECT_TRUE(rad::u8(1) <= static_cast<unsigned int>(2));
    EXPECT_TRUE(rad::u8(2) > static_cast<unsigned int>(1));
    EXPECT_TRUE(rad::u8(2) >= static_cast<unsigned int>(1));
}

TEST(IntegerTests, i16_Compare)
{
    EXPECT_TRUE(rad::i16(1) == rad::i16(1));
    EXPECT_TRUE(rad::i16(1) != rad::i16(2));
    EXPECT_TRUE(rad::i16(1) < rad::i16(2));
    EXPECT_TRUE(rad::i16(1) <= rad::i16(2));
    EXPECT_TRUE(rad::i16(2) > rad::i16(1));
    EXPECT_TRUE(rad::i16(2) >= rad::i16(1));

    EXPECT_TRUE(rad::i16(1) == int16_t(1));
    EXPECT_TRUE(rad::i16(1) != int16_t(2));
    EXPECT_TRUE(rad::i16(1) < int16_t(2));
    EXPECT_TRUE(rad::i16(1) <= int16_t(2));
    EXPECT_TRUE(rad::i16(2) > int16_t(1));
    EXPECT_TRUE(rad::i16(2) >= int16_t(1));

    EXPECT_TRUE(rad::i16(1) == static_cast<signed int>(1));
    EXPECT_TRUE(rad::i16(1) != static_cast<signed int>(2));
    EXPECT_TRUE(rad::i16(1) < static_cast<signed int>(2));
    EXPECT_TRUE(rad::i16(1) <= static_cast<signed int>(2));
    EXPECT_TRUE(rad::i16(2) > static_cast<signed int>(1));
    EXPECT_TRUE(rad::i16(2) >= static_cast<signed int>(1));
}

TEST(IntegerTests, u16_Compare)
{
    EXPECT_TRUE(rad::u16(1) == rad::u16(1));
    EXPECT_TRUE(rad::u16(1) != rad::u16(2));
    EXPECT_TRUE(rad::u16(1) < rad::u16(2));
    EXPECT_TRUE(rad::u16(1) <= rad::u16(2));
    EXPECT_TRUE(rad::u16(2) > rad::u16(1));
    EXPECT_TRUE(rad::u16(2) >= rad::u16(1));

    EXPECT_TRUE(rad::u16(1) == uint16_t(1));
    EXPECT_TRUE(rad::u16(1) != uint16_t(2));
    EXPECT_TRUE(rad::u16(1) < uint16_t(2));
    EXPECT_TRUE(rad::u16(1) <= uint16_t(2));
    EXPECT_TRUE(rad::u16(2) > uint16_t(1));
    EXPECT_TRUE(rad::u16(2) >= uint16_t(1));

    EXPECT_TRUE(rad::u16(1) == static_cast<unsigned int>(1));
    EXPECT_TRUE(rad::u16(1) != static_cast<unsigned int>(2));
    EXPECT_TRUE(rad::u16(1) < static_cast<unsigned int>(2));
    EXPECT_TRUE(rad::u16(1) <= static_cast<unsigned int>(2));
    EXPECT_TRUE(rad::u16(2) > static_cast<unsigned int>(1));
    EXPECT_TRUE(rad::u16(2) >= static_cast<unsigned int>(1));
}

TEST(IntegerTests, i32_Compare)
{
    EXPECT_TRUE(rad::i32(1) == rad::i32(1));
    EXPECT_TRUE(rad::i32(1) != rad::i32(2));
    EXPECT_TRUE(rad::i32(1) < rad::i32(2));
    EXPECT_TRUE(rad::i32(1) <= rad::i32(2));
    EXPECT_TRUE(rad::i32(2) > rad::i32(1));
    EXPECT_TRUE(rad::i32(2) >= rad::i32(1));

    EXPECT_TRUE(rad::i32(1) == int32_t(1));
    EXPECT_TRUE(rad::i32(1) != int32_t(2));
    EXPECT_TRUE(rad::i32(1) < int32_t(2));
    EXPECT_TRUE(rad::i32(1) <= int32_t(2));
    EXPECT_TRUE(rad::i32(2) > int32_t(1));
    EXPECT_TRUE(rad::i32(2) >= int32_t(1));

    EXPECT_TRUE(rad::i32(1) == static_cast<signed int>(1));
    EXPECT_TRUE(rad::i32(1) != static_cast<signed int>(2));
    EXPECT_TRUE(rad::i32(1) < static_cast<signed int>(2));
    EXPECT_TRUE(rad::i32(1) <= static_cast<signed int>(2));
    EXPECT_TRUE(rad::i32(2) > static_cast<signed int>(1));
    EXPECT_TRUE(rad::i32(2) >= static_cast<signed int>(1));
}

TEST(IntegerTests, u32_Compare)
{
    EXPECT_TRUE(rad::u32(1) == rad::u32(1));
    EXPECT_TRUE(rad::u32(1) != rad::u32(2));
    EXPECT_TRUE(rad::u32(1) < rad::u32(2));
    EXPECT_TRUE(rad::u32(1) <= rad::u32(2));
    EXPECT_TRUE(rad::u32(2) > rad::u32(1));
    EXPECT_TRUE(rad::u32(2) >= rad::u32(1));

    EXPECT_TRUE(rad::u32(1) == uint32_t(1));
    EXPECT_TRUE(rad::u32(1) != uint32_t(2));
    EXPECT_TRUE(rad::u32(1) < uint32_t(2));
    EXPECT_TRUE(rad::u32(1) <= uint32_t(2));
    EXPECT_TRUE(rad::u32(2) > uint32_t(1));
    EXPECT_TRUE(rad::u32(2) >= uint32_t(1));

    EXPECT_TRUE(rad::u32(1) == static_cast<unsigned int>(1));
    EXPECT_TRUE(rad::u32(1) != static_cast<unsigned int>(2));
    EXPECT_TRUE(rad::u32(1) < static_cast<unsigned int>(2));
    EXPECT_TRUE(rad::u32(1) <= static_cast<unsigned int>(2));
    EXPECT_TRUE(rad::u32(2) > static_cast<unsigned int>(1));
    EXPECT_TRUE(rad::u32(2) >= static_cast<unsigned int>(1));
}
