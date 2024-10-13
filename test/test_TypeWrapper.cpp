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
#define RAD_ENABLE_NOTHROW_ASSERTIONS      0
#define RAD_ENABLE_NOTHROW_DTOR_ASSERTIONS 0
#define RAD_ENABLE_NOTHROW_MOVE_ASSERTIONS 0

#include "gtest/gtest.h"
#include "test/TestThrow.h"

#include "radiant/TypeWrapper.h"

template <typename T>
using TW = rad::TypeWrapper<T>;
using NTO = radtest::NonThrowingObject;
using TO = radtest::ThrowingObject;
using DNTO = radtest::DerivedNonThrowingObject;
using DTO = radtest::DerivedThrowingObject;

// noexcept passthrough default ctor
RAD_S_ASSERT(noexcept(TW<NTO>()));
RAD_S_ASSERT(!noexcept(TW<TO>()));

// noexcept passthrough copy ctor
RAD_S_ASSERT(noexcept(TW<NTO>(rad::DeclVal<NTO&>())));
RAD_S_ASSERT(!noexcept(TW<TO>(rad::DeclVal<TO&>())));

// noexcept passthrough move ctor
RAD_S_ASSERT(noexcept(TW<NTO>(rad::DeclVal<NTO&&>())));
RAD_S_ASSERT(!noexcept(TW<TO>(rad::DeclVal<TO&&>())));

// noexcept passthrough initializer_list ctor
RAD_S_ASSERT(noexcept(TW<NTO>({ 1, 2 })));
RAD_S_ASSERT(!noexcept(TW<TO>({ 1, 2 })));

// noexcept passthrough wrapper default copy ctor
RAD_S_ASSERT(noexcept(TW<NTO>(rad::DeclVal<TW<NTO>&>())));
RAD_S_ASSERT(!noexcept(TW<TO>(rad::DeclVal<TW<TO>&>())));

// noexcept passthrough wrapper default move ctor
RAD_S_ASSERT(noexcept(TW<NTO>(rad::DeclVal<TW<NTO>&&>())));
RAD_S_ASSERT(!noexcept(TW<TO>(rad::DeclVal<TW<TO>&&>())));

RAD_S_ASSERT(noexcept(TW<NTO>(rad::DeclVal<DNTO&>())));
RAD_S_ASSERT(!noexcept(TW<TO>(rad::DeclVal<DTO&>())));

// noexcept passthrough wrapper convertible ctor
RAD_S_ASSERT(noexcept(TW<NTO>(rad::DeclVal<TW<DNTO>&>())));
RAD_S_ASSERT(!noexcept(TW<TO>(rad::DeclVal<TW<DTO>&>())));

// noexcept passthrough wrapper convertible move ctor
RAD_S_ASSERT(noexcept(TW<NTO>(rad::DeclVal<TW<DNTO>&&>())));
RAD_S_ASSERT(!noexcept(TW<TO>(rad::DeclVal<TW<DTO>&&>())));

// noexcept passthrough forwarding ctor
RAD_S_ASSERT(noexcept(TW<NTO>(1)));

// noexcept passthrough type assign
RAD_S_ASSERT(noexcept(rad::DeclVal<TW<NTO>>() = rad::DeclVal<NTO&>()));
RAD_S_ASSERT(!noexcept(rad::DeclVal<TW<TO>>() = rad::DeclVal<TO&>()));

// noexcept passthrough type move assign
RAD_S_ASSERT(noexcept(rad::DeclVal<TW<NTO>>() = rad::DeclVal<NTO&&>()));
RAD_S_ASSERT(!noexcept(rad::DeclVal<TW<TO>>() = rad::DeclVal<TO&&>()));

// noexcept passthrough wrapper assign
RAD_S_ASSERT(noexcept(rad::DeclVal<TW<NTO>>() = rad::DeclVal<TW<NTO>&>()));
RAD_S_ASSERT(!noexcept(rad::DeclVal<TW<TO>>() = rad::DeclVal<TW<TO>&>()));

// noexcept passthrough wrapper move assign
RAD_S_ASSERT(noexcept(rad::DeclVal<TW<NTO>>() = rad::DeclVal<TW<NTO>&&>()));
RAD_S_ASSERT(!noexcept(rad::DeclVal<TW<TO>>() = rad::DeclVal<TW<TO>&&>()));

// noexcept l-ref ctor
RAD_S_ASSERT(noexcept(TW<TO&>(rad::DeclVal<TO&>())));
RAD_S_ASSERT(noexcept(TW<NTO&>(rad::DeclVal<NTO&>())));

// noexcept const l-ref ctor
RAD_S_ASSERT(noexcept(TW<const TO&>(rad::DeclVal<const TO&>())));
RAD_S_ASSERT(noexcept(TW<const NTO&>(rad::DeclVal<const NTO&>())));

// noexcept passthrough wrapper l-ref copy ctor
RAD_S_ASSERT(noexcept(TW<TO&>(rad::DeclVal<TW<TO&>>())));
RAD_S_ASSERT(noexcept(TW<TO&>(rad::DeclVal<TW<TO&>>())));

// l-ref valid/invalid ctors
RAD_S_ASSERT((rad::IsCtor<TW<TO&>, TO&>));
RAD_S_ASSERT((rad::IsCtor<TW<TO&>, DTO&>));
RAD_S_ASSERT(!(rad::IsCtor<TW<TO&>, TO&&>));
RAD_S_ASSERT(!(rad::IsCtor<TW<TO&>, const TO&>));
RAD_S_ASSERT((rad::IsCtor<TW<const TO&>, TO&>));
RAD_S_ASSERT((rad::IsCtor<TW<const TO&>, const TO&>));
RAD_S_ASSERT((rad::IsCtor<TW<const TO&>, const DTO&>));

RAD_S_ASSERT((rad::IsCtor<TW<TO&>, TW<TO&>>));
RAD_S_ASSERT((rad::IsCtor<TW<TO&>, const TW<TO&>>));
RAD_S_ASSERT(!(rad::IsCtor<TW<TO&>, const TW<const TO&>>));
RAD_S_ASSERT((rad::IsCtor<TW<const TO&>, TW<TO&>>));
RAD_S_ASSERT((rad::IsCtor<TW<const TO&>, TW<const TO&>>));
RAD_S_ASSERT((rad::IsCtor<TW<const TO&>, const TW<TO&>>));

TEST(TypeWrapperTests, DefaultCtor)
{
    EXPECT_THROW(TW<TO>(1), std::exception);
    EXPECT_NO_THROW(TW<NTO>());
}

TEST(TypeWrapperTests, Get)
{
    TW<TO> to(2);
    EXPECT_EQ(to.Get(), 2);

    const TW<TO> cto(3);
    EXPECT_EQ(cto.Get(), 3);
}

TEST(TypeWrapperTests, TypeCopyCtor)
{
    TO to(2);

    TW<TO> twto(to);
    EXPECT_EQ(twto.Get(), 2);
    EXPECT_EQ(to, 2);

    TW<const TO> twcto(to);
    EXPECT_EQ(twcto.Get(), 2);
    EXPECT_EQ(to, 2);

    const TW<TO> ctwto(to);
    EXPECT_EQ(ctwto.Get(), 2);
    EXPECT_EQ(to, 2);
}

TEST(TypeWrapperTests, CopyInit)
{
    TW<int> to = 1;
    EXPECT_EQ(to, 1);
}

TEST(TypeWrapperTests, CopyCtorFromRefTypeWrapper)
{
    int val = 2;
    TW<int&> twri(val);

    TW<int> twi(twri);
    EXPECT_EQ(twi, val);
}

TEST(TypeWrapperTests, TypeMoveCtor)
{
    TO to(2);
    TW<TO> twto(rad::Move(to));
    EXPECT_EQ(twto.Get(), 2);
    EXPECT_EQ(to, 0);

    TW<TO> twto2(TO(3));
    EXPECT_EQ(twto2.Get(), 3);
}

#if RAD_ENABLE_STD

TEST(TypeWrapperTests, TypeInitListCtor)
{
    TW<TO> twto({ 2, 3 });
    EXPECT_EQ(twto.Get(), 2);
    EXPECT_EQ(twto, 2);

    TW<TO> twtoe({ 0 });
    EXPECT_EQ(twtoe.Get(), 0);
    EXPECT_EQ(twtoe, 0);
}

#endif // RAD_ENABLE_STD

TEST(TypeWrapperTests, ConvertibleCtor)
{
    TW<DTO> twdto(2);
    TW<TO> twto(twdto);
    EXPECT_EQ(twto.Get(), 2);
}

TEST(TypeWrapperTests, ConvertibleMoveCtor)
{
    TW<TO> twto(TW<DTO>(2));
    EXPECT_EQ(twto.Get(), 2);

    TW<TO> twto2(rad::Move(twto));
    EXPECT_EQ(twto.Get(), 0);
    EXPECT_EQ(twto2, 2);
}

TEST(TypeWrapperTests, TypeCopyAssign)
{
    int val = 2;
    TW<TO> twto(3);
    twto = val;
    EXPECT_EQ(twto, 2);
    twto = 4;
    EXPECT_EQ(twto, 4);

    TO to(2);
    twto = to;
    EXPECT_EQ(twto, 2);
    EXPECT_EQ(twto, to);

    TW<TO> twto2(0);
    twto2 = to;
    EXPECT_EQ(twto2, 2);
    EXPECT_EQ(twto2, to);
}

TEST(TypeWrapperTests, TypeMoveAssign)
{
    TO to(2);

    TW<TO> twto;
    twto = rad::Move(to);
    EXPECT_EQ(twto, 2);
    EXPECT_EQ(to, 0);
}

TEST(TypeWrapperTests, ConvertibleCopyAssign)
{
    TW<TO> to(2);
    TW<DTO> dto(3);
    TW<TO> copy(0);

    copy = to;
    EXPECT_EQ(copy, 2);
    copy = dto;
    EXPECT_EQ(copy, 3);
}

TEST(TypeWrapperTests, ConvertibleMoveAssign)
{
    TW<TO> twto(0);
    twto = TW<TO>{ 2 };
    EXPECT_EQ(twto, 2);

    TW<TO> other(3);
    twto = rad::Move(other);
    EXPECT_EQ(twto, 3);
    EXPECT_EQ(other, 0);
}

TEST(TypeWrapperTests, RefCtor)
{
    TO to(2);
    TW<TO&> twto(to);
    EXPECT_EQ(&twto.Get(), &to);
    EXPECT_EQ(twto, to);

    TW<const TO&> twcto(to);
    EXPECT_EQ(&twcto.Get(), &to);
    EXPECT_EQ(twcto, to);

    const TW<TO&> ctwto(to);
    EXPECT_EQ(&ctwto.Get(), &to);
    EXPECT_EQ(ctwto, to);

    const TW<const TO&> ctwcto(to);
    EXPECT_EQ(&ctwcto.Get(), &to);
    EXPECT_EQ(ctwcto, to);
}

TEST(TypeWrapperTests, RefCtorFromValueTypeWrapper)
{
    TW<TO> twto(2);
    TW<const TO> twcto(2);

    TW<TO&> twrto(twto);
    EXPECT_EQ(&twrto.Get(), &twto.Get());

    TW<const TO&> twrtcto(twcto);
    EXPECT_EQ(&twrtcto.Get(), &twcto.Get());
}

TEST(TypeWrapperTests, RefCopyCtor)
{
    TO to;
    DTO dto;
    TW<TO&> twrto(to);
    TW<DTO&> twrdto(dto);
    const TW<TO&> ctwrto(to);

    TW<TO&> twto2(twrto);
    EXPECT_EQ(&twto2.Get(), &to);

    TW<TO&> twto3(twrdto);
    EXPECT_EQ(&twto3.Get(), &dto);

    TW<const TO&> twcto(twrto);
    EXPECT_EQ(&twcto.Get(), &to);

    const TW<const TO&> ctwcto(ctwrto);
    EXPECT_EQ(&ctwcto.Get(), &to);
}

TEST(TypeWrapperTests, RefMoveCtor)
{
    TO to;
    TW<TO&> twrto(to);

    TW<TO&> twrto2(rad::Move(twrto));
    EXPECT_EQ(&twrto2.Get(), &to);

    TW<TO&> twrto3(TW<TO&>{ to });
    EXPECT_EQ(&twrto3.Get(), &to);
}

TEST(TypeWrapperTests, RefAssignValue)
{
    TO to;
    TO other;
    TW<TO&> rto(to);
    EXPECT_EQ(&rto.Get(), &to);
    rto = other;
    EXPECT_EQ(&rto.Get(), &other);

    DTO dto;
    rto = dto;
    EXPECT_EQ(&rto.Get(), &dto);
}

TEST(TypeWrapperTests, RefAssignRef)
{
    TO to;
    TO other;
    TO& rto(to);

    DTO dto;
    DTO& rdto(dto);

    TW<TO&> twrto(other);
    EXPECT_EQ(&twrto.Get(), &other);
    twrto = rto;
    EXPECT_EQ(&twrto.Get(), &to);

    TW<TO&> twrto2(other);
    EXPECT_EQ(&twrto2.Get(), &other);
    twrto2 = rdto;
    EXPECT_EQ(&twrto2.Get(), &dto);
}

TEST(TypeWrapperTests, RefCopyAssign)
{
    TO to;
    TO other;
    TW<TO&> rto(to);

    TW<TO&> rto2(other);
    rto2 = rto;
    EXPECT_EQ(&rto2.Get(), &to);

    TW<const TO&> crto(other);
    crto = rto;
    EXPECT_EQ(&crto.Get(), &to);
}

TEST(TypeWrapperTests, RefMoveAssign)
{
    TO to;
    TW<TO&> twto(to);
    TW<TO&> twrto(rad::Move(twto));
    EXPECT_EQ(&twto.Get(), &to);
    EXPECT_EQ(&twrto.Get(), &to);

    TW<const TO&> twcrto(TW<TO&>{ to });
    EXPECT_EQ(&twcrto.Get(), &to);
}

TEST(TypeWrapperTests, ComparisonEqual)
{
    int val = 2;

    TW<TO> twto(val);
    TW<const TO> twcto(2);
    TW<int&> twrto(val);
    TW<const int&> twcrto(val);

    EXPECT_EQ(twto, 2);
    EXPECT_EQ(2, twto);
    EXPECT_EQ(twcto, 2);
    EXPECT_EQ(2, twcto);
    EXPECT_EQ(twto, twcto);
    EXPECT_EQ(twcto, twto);

    EXPECT_EQ(twrto, twto);
    EXPECT_EQ(twto, twrto);
    EXPECT_EQ(twrto, val);
    EXPECT_EQ(val, twrto);
    EXPECT_EQ(twcrto, twrto);
    EXPECT_EQ(twrto, twcrto);
    EXPECT_EQ(twcrto, twto);
    EXPECT_EQ(twto, twcrto);
}

TEST(TypeWrapperTests, ComparisonNotEqual)
{
    int val = 4;
    int other = 5;
    int other2 = 6;

    TW<TO> twto(val);
    TW<const TO> twcto(2);
    TW<int&> twrto(other);
    TW<const int&> twcrto(other2);

    EXPECT_NE(twto, 3);
    EXPECT_NE(3, twto);
    EXPECT_NE(twcto, 3);
    EXPECT_NE(3, twcto);
    EXPECT_NE(twto, twcto);
    EXPECT_NE(twcto, twto);

    EXPECT_NE(twrto, twto);
    EXPECT_NE(twto, twrto);
    EXPECT_NE(twrto, val);
    EXPECT_NE(val, twrto);
    EXPECT_NE(twcrto, twrto);
    EXPECT_NE(twrto, twcrto);
    EXPECT_NE(twcrto, twto);
    EXPECT_NE(twto, twcrto);
}

TEST(TypeWrapperTests, ComparisonLessThan)
{
    int val = 4;
    TW<TO> twto(2);
    TW<const TO> twcto(3);

    EXPECT_LT(twto, 3);
    EXPECT_LT(1, twto);
    EXPECT_LT(twcto, val);
    EXPECT_LT(2, twcto);
    EXPECT_LT(twto, twcto);
}

TEST(TypeWrapperTests, ComparisonLessThanOrEqual)
{
    int val = 4;
    int three = 3;
    TW<TO> twto(2);
    TW<const TO> twcto(3);

    EXPECT_LE(twto, 3);
    EXPECT_LE(twto, 2);
    EXPECT_LE(1, twto);
    EXPECT_LE(2, twto);
    EXPECT_LE(twcto, val);
    EXPECT_LE(twcto, three);
    EXPECT_LE(2, twcto);
    EXPECT_LE(3, twcto);
    EXPECT_LE(twto, twcto);
}

TEST(TypeWrapperTests, ComparisonGreaterThan)
{
    int val = 4;
    TW<TO> twto(5);
    TW<const TO> twcto(6);

    EXPECT_GT(twto, 4);
    EXPECT_GT(6, twto);
    EXPECT_GT(twcto, val);
    EXPECT_GT(7, twcto);
    EXPECT_GT(twcto, twto);
}

TEST(TypeWrapperTests, ComparisonGreaterThanOrEqual)
{
    int val = 1;
    int three = 3;
    TW<TO> twto(2);
    TW<const TO> twcto(3);

    EXPECT_GE(twto, 2);
    EXPECT_GE(twto, 1);
    EXPECT_GE(2, twto);
    EXPECT_GE(3, twto);
    EXPECT_GE(twcto, val);
    EXPECT_GE(twcto, three);
    EXPECT_GE(3, twcto);
    EXPECT_GE(4, twcto);
    EXPECT_GE(twcto, twto);
}
