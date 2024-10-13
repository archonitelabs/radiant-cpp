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
#include "test/TestThrow.h"

#include "radiant/Span.h"
#include "radiant/Utility.h"

static constexpr auto g_SpanString = rad::MakeSpan("Span String");
static constexpr const uint8_t g_Bytes[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
                                             0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
                                             0xcc, 0xdd, 0xee, 0xff };
static constexpr auto g_SpanBytes = rad::MakeSpan(g_Bytes);

RAD_S_ASSERT(sizeof(g_SpanString) == sizeof(void*));
RAD_S_ASSERT(g_SpanString.Size() == rad::Size("Span String"));
RAD_S_ASSERT(sizeof(g_SpanBytes) == sizeof(void*));
RAD_S_ASSERT(g_SpanBytes.Size() == rad::Size(g_Bytes));

// clang-format off

RAD_S_ASSERT(noexcept(rad::Span<const char>("derp")));

RAD_S_ASSERT(noexcept(rad::Span<radtest::ThrowingObject>()));
RAD_S_ASSERT(noexcept(rad::Span<radtest::ThrowingObject>(rad::DeclVal<radtest::ThrowingObject*>(), 1)));
RAD_S_ASSERT(noexcept(rad::Span<radtest::ThrowingObject, 1>(rad::DeclVal<radtest::ThrowingObject*>(), 1)));
RAD_S_ASSERT(noexcept(rad::Span<radtest::ThrowingObject>(rad::DeclVal<radtest::ThrowingObject*>(), rad::DeclVal<radtest::ThrowingObject*>())));
RAD_S_ASSERT(noexcept(rad::Span<radtest::ThrowingObject, 1>(rad::DeclVal<radtest::ThrowingObject*>(), rad::DeclVal<radtest::ThrowingObject*>())));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::ThrowingObject>>().~Span()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::ThrowingObject>>().Data()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::ThrowingObject>>().Size()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::ThrowingObject>>().SizeBytes()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::ThrowingObject>>().Empty()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::ThrowingObject>>().Front()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::ThrowingObject>>().Back()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::ThrowingObject>>().AsBytes()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::ThrowingObject>>().Subspan(1, 1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::ThrowingObject>>().Subspan<1>()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::ThrowingObject>>().First(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::ThrowingObject>>().First<1>()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::ThrowingObject>>().Last(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::ThrowingObject>>().Last<1>()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::ThrowingObject>>().begin()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::ThrowingObject>>().end()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::ThrowingObject>>().rbegin()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::ThrowingObject>>().rend()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::ThrowingObject>>().operator[](1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::ThrowingObject>>().operator=(rad::DeclVal<const rad::Span<radtest::ThrowingObject>&>())));

RAD_S_ASSERT(noexcept(rad::Span<radtest::NonThrowingObject>()));
RAD_S_ASSERT(noexcept(rad::Span<radtest::NonThrowingObject>(rad::DeclVal<radtest::NonThrowingObject*>(), 1)));
RAD_S_ASSERT(noexcept(rad::Span<radtest::NonThrowingObject, 1>(rad::DeclVal<radtest::NonThrowingObject*>(), 1)));
RAD_S_ASSERT(noexcept(rad::Span<radtest::NonThrowingObject>(rad::DeclVal<radtest::NonThrowingObject*>(), rad::DeclVal<radtest::NonThrowingObject*>())));
RAD_S_ASSERT(noexcept(rad::Span<radtest::NonThrowingObject, 1>(rad::DeclVal<radtest::NonThrowingObject*>(), rad::DeclVal<radtest::NonThrowingObject*>())));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::NonThrowingObject>>().~Span()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::NonThrowingObject>>().Data()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::NonThrowingObject>>().Size()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::NonThrowingObject>>().SizeBytes()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::NonThrowingObject>>().Empty()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::NonThrowingObject>>().Front()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::NonThrowingObject>>().Back()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::NonThrowingObject>>().AsBytes()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::NonThrowingObject>>().Subspan(1, 1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::NonThrowingObject>>().Subspan<1>()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::NonThrowingObject>>().First(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::NonThrowingObject>>().First<1>()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::NonThrowingObject>>().Last(1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::NonThrowingObject>>().Last<1>()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::NonThrowingObject>>().begin()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::NonThrowingObject>>().end()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::NonThrowingObject>>().rbegin()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::NonThrowingObject>>().rend()));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::NonThrowingObject>>().operator[](1)));
RAD_S_ASSERT(noexcept(rad::DeclVal<rad::Span<radtest::NonThrowingObject>>().operator=(rad::DeclVal<const rad::Span<radtest::NonThrowingObject>&>())));

// clang-format on

TEST(SpanTests, ConstructBySize)
{
    const uint8_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    rad::Span<const uint8_t> span(data, rad::Size(data));

    EXPECT_EQ(rad::Size(span), rad::Size(data));

    EXPECT_EQ(span.Front(), 0u);
    EXPECT_EQ(span.Back(), 10u);
    EXPECT_EQ(span.Size(), rad::Size(data));

    for (uint32_t i = 0; i < rad::Size(data); i++)
    {
        EXPECT_EQ(span[i], data[i]);
    }

    uint32_t index = 0;
    for (const auto& item : span)
    {
        EXPECT_EQ(item, data[index]);
        index++;
    }
}

TEST(SpanTests, ConstructByPointers)
{
    const uint8_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    rad::Span<const uint8_t> span(data, data + rad::Size(data));

    EXPECT_EQ(span.Front(), 0u);
    EXPECT_EQ(span.Back(), 10u);
    EXPECT_EQ(span.Size(), rad::Size(data));

    for (uint32_t i = 0; i < rad::Size(data); i++)
    {
        EXPECT_EQ(span[i], data[i]);
    }

    uint32_t index = 0;
    for (const auto& item : span)
    {
        EXPECT_EQ(item, data[index]);
        index++;
    }
}

TEST(SpanTests, LastStaticExtentByPointer)
{
    const uint8_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    rad::Span<const uint8_t, 3> span(data, data + 3);
    RAD_S_ASSERT(sizeof(span) == sizeof(void*));

    EXPECT_EQ(span.Size(), 3u);
    EXPECT_EQ(span.Data(), data);

    for (uint32_t i = 0; i < span.Size(); i++)
    {
        EXPECT_EQ(span[i], data[i]);
    }
}

TEST(SpanTests, FirstStaticExtent)
{
    const uint8_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    rad::Span<const uint8_t> span(data, rad::Size(data));

    auto sub = span.First<3>();
    RAD_S_ASSERT(sizeof(sub) == sizeof(void*));

    EXPECT_EQ(sub.Front(), 0u);
    EXPECT_EQ(sub.Back(), 2u);
    EXPECT_EQ(sub.Size(), 3u);
}

TEST(SpanTests, LastStaticExtent)
{
    const uint8_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    rad::Span<const uint8_t> span(data, rad::Size(data));

    auto sub = span.Last<3>();
    RAD_S_ASSERT(sizeof(sub) == sizeof(void*));

    EXPECT_EQ(sub.Front(), 8u);
    EXPECT_EQ(sub.Back(), 10u);
    EXPECT_EQ(sub.Size(), 3u);
}

TEST(SpanTests, FirstDynamicExtent)
{
    const uint8_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    rad::Span<const uint8_t> span(data, rad::Size(data));

    auto sub = span.First(3);
    RAD_S_ASSERT(sizeof(sub) > sizeof(void*));

    EXPECT_EQ(sub.Front(), 0u);
    EXPECT_EQ(sub.Back(), 2u);
    EXPECT_EQ(sub.Size(), 3u);
}

TEST(SpanTests, LastDynamicExtent)
{
    const uint8_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    rad::Span<const uint8_t> span(data, rad::Size(data));

    auto sub = span.Last(3);
    RAD_S_ASSERT(sizeof(sub) > sizeof(void*));

    EXPECT_EQ(sub.Front(), 8u);
    EXPECT_EQ(sub.Back(), 10u);
    EXPECT_EQ(sub.Size(), 3u);
}

TEST(SpanTests, SubspanStaticExtent)
{
    const uint8_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    rad::Span<const uint8_t> span(data, rad::Size(data));

    auto sub = span.Subspan<3, 4>();
    RAD_S_ASSERT(sizeof(sub) == sizeof(void*));

    EXPECT_EQ(sub.Front(), 3u);
    EXPECT_EQ(sub.Back(), 6u);
    EXPECT_EQ(sub.Size(), 4u);
}

TEST(SpanTests, SubspanDynamicExtent)
{
    const uint8_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    rad::Span<const uint8_t> span(data, rad::Size(data));

    auto sub = span.Subspan(3, 4);
    RAD_S_ASSERT(sizeof(sub) > sizeof(void*));

    EXPECT_EQ(sub.Front(), 3u);
    EXPECT_EQ(sub.Back(), 6u);
    EXPECT_EQ(sub.Size(), 4u);
}

TEST(SpanTests, SizeInBytes)
{
    const uint32_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    rad::Span<const uint32_t> span(data, rad::Size(data));

    EXPECT_EQ(span.SizeBytes(), 44u);
}

TEST(SpanTests, Iterate)
{
    const uint32_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    rad::Span<const uint32_t> span(data, rad::Size(data));

    uint32_t i = 0;
    for (auto it = span.begin(); it != span.end(); ++it)
    {
        EXPECT_EQ(*it, data[i++]);
    }
    EXPECT_EQ(i, rad::Size(data));
}

TEST(SpanTests, ReverseIterate)
{
    const uint32_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    rad::Span<const uint32_t> span(data, rad::Size(data));

    int i = rad::Size(data);
    for (auto it = span.rbegin(); it != span.rend(); ++it)
    {
        EXPECT_EQ(*it, data[--i]);
    }
    EXPECT_EQ(i, 0);
}

TEST(SpanTests, IterateEmpty)
{
    rad::Span<const uint32_t> span;

    int i = 0;
    for (auto it = span.begin(); it != span.end(); ++it)
    {
        i++;
    }
    EXPECT_EQ(i, 0);
}

TEST(SpanTests, ReverseIterateEmpty)
{
    rad::Span<const uint32_t> span;

    int i = 0;
    for (auto it = span.rbegin(); it != span.rend(); ++it)
    {
        i++;
    }
    EXPECT_EQ(i, 0);
}

TEST(SpanTests, AsBytes)
{
    uint32_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    rad::Span<uint32_t> span(data, rad::Size(data));

    for (auto& b : span.AsBytes())
    {
        b = rad::Byte(1);
    }

    uint32_t expected = 0x1010101;
    for (const auto& d : data)
    {
        EXPECT_EQ(d, expected);
    }

    rad::Span<const uint32_t> spanTwo(data, rad::Size(data));
    for (auto& v : spanTwo.AsBytes())
    {
        EXPECT_EQ(v, rad::Byte(1));
    }
}

TEST(SpanTests, Literal)
{
    auto string = rad::MakeSpan("derp");

    EXPECT_EQ(string.Size(), 5u);
    EXPECT_EQ(string[0], 'd');
    EXPECT_EQ(string[1], 'e');
    EXPECT_EQ(string[2], 'r');
    EXPECT_EQ(string[3], 'p');
}

TEST(SpanTests, LiteralConvert)
{
    auto string = rad::MakeSpan("derp");

    rad::Span<const char> other(string);

    EXPECT_EQ(string.Size(), other.Size());
    EXPECT_EQ(string.Data(), other.Data());
    EXPECT_EQ(string[0], other[0]);
    EXPECT_EQ(string[1], other[1]);
    EXPECT_EQ(string[2], other[2]);
    EXPECT_EQ(string[3], other[3]);
}

TEST(SpanTests, DynamicConvertStatic)
{
    uint32_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    rad::Span<uint32_t> span(data, rad::Size(data));

    rad::Span<uint32_t, 3> other(span);

    EXPECT_EQ(other.Size(), 3u);
    EXPECT_EQ(span.Data(), other.Data());
    EXPECT_EQ(span[0], other[0]);
    EXPECT_EQ(span[1], other[1]);
    EXPECT_EQ(span[2], other[2]);
}

TEST(SpanTests, MakeSpanStatic)
{
    uint32_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    auto span = rad::MakeSpan<uint32_t, 4>(data);
    RAD_S_ASSERT(sizeof(span) == sizeof(void*));
    RAD_S_ASSERT(span.Extent == 4u);

    EXPECT_EQ(span.Size(), 4u);
    EXPECT_EQ(span.Data(), data);
    EXPECT_EQ(span[0], data[0]);
    EXPECT_EQ(span[1], data[1]);
    EXPECT_EQ(span[2], data[2]);
    EXPECT_EQ(span[3], data[3]);
}

TEST(SpanTests, MakeSpanDynamic)
{
    uint32_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    auto span = rad::MakeSpan<uint32_t>(data, rad::Size(data));

    EXPECT_EQ(span.Data(), data);
    for (uint32_t i = 0; i < rad::Size(data); i++)
    {
        EXPECT_EQ(span[i], data[i]);
    }
}

TEST(SpanTests, MakeSpanDynamicByPointer)
{
    uint32_t data[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    auto span = rad::MakeSpan<uint32_t>(data, data + rad::Size(data));

    EXPECT_EQ(span.Data(), data);
    for (uint32_t i = 0; i < rad::Size(data); i++)
    {
        EXPECT_EQ(span[i], data[i]);
    }
}
