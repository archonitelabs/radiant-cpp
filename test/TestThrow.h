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

#include "radiant/Utility.h"

#include <initializer_list>
#include <exception>

namespace radtest
{

/// @brief Will throw if constructed with a value of 1
struct ThrowingObject
{
    ~ThrowingObject()
    {
        m_value = 0;
    }

    ThrowingObject()
        : m_value(0)
    {
    }

    ThrowingObject(int Value)
        : m_value(Value)
    {
        if (m_value == 1)
        {
            throw std::exception();
        }
    }

    ThrowingObject(std::initializer_list<int> Init)
        : m_value(*Init.begin())
    {
    }

    ThrowingObject(const ThrowingObject& Other)
        : m_value(Other.m_value)
    {
    }

    ThrowingObject(ThrowingObject&& Other)
        : m_value(rad::Move(Other.m_value))
    {
        Other.m_value = 0;
    }

    ThrowingObject& operator=(const ThrowingObject& Other)
    {
        m_value = Other.m_value;
        return *this;
    }

    ThrowingObject& operator=(ThrowingObject&& Other)
    {
        m_value = rad::Move(Other.m_value);
        Other.m_value = 0;
        return *this;
    }

    operator int() const
    {
        return m_value;
    }

    int m_value = 0;
};

struct DerivedThrowingObject : public ThrowingObject
{
    using ThrowingObject::ThrowingObject;
    using ThrowingObject::operator=;
};

// clang-format off

RAD_S_ASSERT(noexcept(rad::DeclVal<ThrowingObject>().~ThrowingObject()));
RAD_S_ASSERT(!noexcept(ThrowingObject()));
RAD_S_ASSERT(!noexcept(ThrowingObject(1)));
RAD_S_ASSERT(!noexcept(ThrowingObject(ThrowingObject())));
RAD_S_ASSERT(!noexcept(ThrowingObject(rad::Move(rad::DeclVal<ThrowingObject&>()))));
RAD_S_ASSERT(!noexcept(rad::DeclVal<ThrowingObject>().operator=(ThrowingObject())));
RAD_S_ASSERT(!noexcept(rad::DeclVal<ThrowingObject>().operator=(rad::Move(rad::DeclVal<ThrowingObject&>()))));

// clang-format on

struct ThrowingObjectTwo
{
    ~ThrowingObjectTwo()
    {
        m_value = 0;
    }

    ThrowingObjectTwo()
        : m_value(0)
    {
    }

    ThrowingObjectTwo(int Value)
        : m_value(Value)
    {
    }

    ThrowingObjectTwo(std::initializer_list<int> Init)
        : m_value(*Init.begin())
    {
    }

    ThrowingObjectTwo(const ThrowingObjectTwo& Other)
        : m_value(Other.m_value)
    {
    }

    ThrowingObjectTwo(ThrowingObjectTwo&& Other)
        : m_value(rad::Move(Other.m_value))
    {
    }

    ThrowingObjectTwo& operator=(const ThrowingObjectTwo& Other)
    {
        m_value = Other.m_value;
        return *this;
    }

    ThrowingObjectTwo& operator=(ThrowingObjectTwo&& Other)
    {
        m_value = rad::Move(Other.m_value);
        return *this;
    }

    long long m_value = 0;
};

// clang-format off

RAD_S_ASSERT(noexcept(rad::DeclVal<ThrowingObjectTwo>().~ThrowingObjectTwo()));
RAD_S_ASSERT(!noexcept(ThrowingObjectTwo()));
RAD_S_ASSERT(!noexcept(ThrowingObjectTwo(1)));
RAD_S_ASSERT(!noexcept(ThrowingObjectTwo(ThrowingObjectTwo())));
RAD_S_ASSERT(!noexcept(ThrowingObjectTwo(rad::Move(rad::DeclVal<ThrowingObjectTwo&>()))));
RAD_S_ASSERT(!noexcept(rad::DeclVal<ThrowingObjectTwo>().operator=(ThrowingObjectTwo())));
RAD_S_ASSERT(!noexcept(rad::DeclVal<ThrowingObjectTwo>().operator=(rad::Move(rad::DeclVal<ThrowingObjectTwo&>()))));

// clang-format on

struct NonThrowingObject
{
    ~NonThrowingObject() noexcept
    {
        m_value = 0;
    }

    NonThrowingObject() noexcept
        : m_value(0)
    {
    }

    NonThrowingObject(int Value) noexcept
        : m_value(Value)
    {
    }

    NonThrowingObject(std::initializer_list<int> Init) noexcept
        : m_value(*Init.begin())
    {
    }

    NonThrowingObject(const NonThrowingObject& Other) noexcept
        : m_value(Other.m_value)
    {
    }

    NonThrowingObject(NonThrowingObject&& Other) noexcept
        : m_value(rad::Move(Other.m_value))
    {
        Other.m_value = 0;
    }

    NonThrowingObject& operator=(const NonThrowingObject& Other) noexcept
    {
        m_value = Other.m_value;
        return *this;
    }

    NonThrowingObject& operator=(NonThrowingObject&& Other) noexcept
    {
        m_value = rad::Move(Other.m_value);
        Other.m_value = 0;
        return *this;
    }

    operator int() const noexcept
    {
        return m_value;
    }

    int m_value = 0;
};

struct DerivedNonThrowingObject : public NonThrowingObject
{
    using NonThrowingObject::NonThrowingObject;
    using NonThrowingObject::operator=;
};

// clang-format off

RAD_S_ASSERT(noexcept(rad::DeclVal<NonThrowingObject>().~NonThrowingObject()));
RAD_S_ASSERT(noexcept(NonThrowingObject()));
RAD_S_ASSERT(noexcept(NonThrowingObject(1)));
RAD_S_ASSERT(noexcept(NonThrowingObject(NonThrowingObject())));
RAD_S_ASSERT(noexcept(NonThrowingObject(rad::Move(rad::DeclVal<NonThrowingObject&>()))));
RAD_S_ASSERT(noexcept(rad::DeclVal<NonThrowingObject>().operator=(NonThrowingObject())));
RAD_S_ASSERT(noexcept(rad::DeclVal<NonThrowingObject>().operator=(rad::Move(rad::DeclVal<NonThrowingObject&>()))));

// clang-format on

struct NonThrowingObjectTwo
{
    ~NonThrowingObjectTwo() noexcept
    {
        m_value = 0;
    }

    NonThrowingObjectTwo() noexcept
        : m_value(0)
    {
    }

    NonThrowingObjectTwo(int Value) noexcept
        : m_value(Value)
    {
    }

    NonThrowingObjectTwo(std::initializer_list<int> Init) noexcept
        : m_value(*Init.begin())
    {
    }

    NonThrowingObjectTwo(const NonThrowingObjectTwo& Other) noexcept
        : m_value(Other.m_value)
    {
    }

    NonThrowingObjectTwo(NonThrowingObjectTwo&& Other) noexcept
        : m_value(rad::Move(Other.m_value))
    {
    }

    NonThrowingObjectTwo& operator=(const NonThrowingObjectTwo& Other) noexcept
    {
        m_value = Other.m_value;
        return *this;
    }

    NonThrowingObjectTwo& operator=(NonThrowingObjectTwo&& Other) noexcept
    {
        m_value = rad::Move(Other.m_value);
        return *this;
    }

    int m_value = 0;
};

// clang-format off

RAD_S_ASSERT(noexcept(rad::DeclVal<NonThrowingObjectTwo>().~NonThrowingObjectTwo()));
RAD_S_ASSERT(noexcept(NonThrowingObjectTwo()));
RAD_S_ASSERT(noexcept(NonThrowingObjectTwo(1)));
RAD_S_ASSERT(noexcept(NonThrowingObjectTwo(NonThrowingObjectTwo())));
RAD_S_ASSERT(noexcept(NonThrowingObjectTwo(rad::Move(rad::DeclVal<NonThrowingObjectTwo&>()))));
RAD_S_ASSERT(noexcept(rad::DeclVal<NonThrowingObjectTwo>().operator=(NonThrowingObjectTwo())));
RAD_S_ASSERT(noexcept(rad::DeclVal<NonThrowingObjectTwo>().operator=(rad::Move(rad::DeclVal<NonThrowingObjectTwo&>()))));

// clang-format on

struct ThrowingObjectTrivDtor
{
    ~ThrowingObjectTrivDtor() = default;

    ThrowingObjectTrivDtor()
        : m_value(0)
    {
    }

    ThrowingObjectTrivDtor(int Value)
        : m_value(Value)
    {
    }

    ThrowingObjectTrivDtor(std::initializer_list<int> Init)
        : m_value(*Init.begin())
    {
    }

    ThrowingObjectTrivDtor(const ThrowingObjectTrivDtor& Other)
        : m_value(Other.m_value)
    {
    }

    ThrowingObjectTrivDtor(ThrowingObjectTrivDtor&& Other)
        : m_value(rad::Move(Other.m_value))
    {
    }

    ThrowingObjectTrivDtor& operator=(const ThrowingObjectTrivDtor& Other)
    {
        m_value = Other.m_value;
        return *this;
    }

    ThrowingObjectTrivDtor& operator=(ThrowingObjectTrivDtor&& Other)
    {
        m_value = rad::Move(Other.m_value);
        return *this;
    }

    int m_value = 0;
};

// clang-format off

RAD_S_ASSERT(noexcept(rad::DeclVal<ThrowingObjectTrivDtor>().~ThrowingObjectTrivDtor()));
RAD_S_ASSERT(!noexcept(ThrowingObjectTrivDtor()));
RAD_S_ASSERT(!noexcept(ThrowingObjectTrivDtor(1)));
RAD_S_ASSERT(!noexcept(ThrowingObjectTrivDtor(ThrowingObjectTrivDtor())));
RAD_S_ASSERT(!noexcept(ThrowingObjectTrivDtor(rad::Move(rad::DeclVal<ThrowingObjectTrivDtor&>()))));
RAD_S_ASSERT(!noexcept(rad::DeclVal<ThrowingObjectTrivDtor>().operator=(ThrowingObjectTrivDtor())));
RAD_S_ASSERT(!noexcept(rad::DeclVal<ThrowingObjectTrivDtor>().operator=(rad::Move(rad::DeclVal<ThrowingObjectTrivDtor&>()))));

// clang-format on

struct ThrowingObjectTrivDtorTwo
{
    ~ThrowingObjectTrivDtorTwo() = default;

    ThrowingObjectTrivDtorTwo()
        : m_value(0)
    {
    }

    ThrowingObjectTrivDtorTwo(int Value)
        : m_value(Value)
    {
    }

    ThrowingObjectTrivDtorTwo(std::initializer_list<int> Init)
        : m_value(*Init.begin())
    {
    }

    ThrowingObjectTrivDtorTwo(const ThrowingObjectTrivDtorTwo& Other)
        : m_value(Other.m_value)
    {
    }

    ThrowingObjectTrivDtorTwo(ThrowingObjectTrivDtorTwo&& Other)
        : m_value(rad::Move(Other.m_value))
    {
    }

    ThrowingObjectTrivDtorTwo& operator=(const ThrowingObjectTrivDtorTwo& Other)
    {
        m_value = Other.m_value;
        return *this;
    }

    ThrowingObjectTrivDtorTwo& operator=(ThrowingObjectTrivDtorTwo&& Other)
    {
        m_value = rad::Move(Other.m_value);
        return *this;
    }

    int m_value = 0;
};

// clang-format off

RAD_S_ASSERT(noexcept(rad::DeclVal<ThrowingObjectTrivDtorTwo>().~ThrowingObjectTrivDtorTwo()));
RAD_S_ASSERT(!noexcept(ThrowingObjectTrivDtorTwo()));
RAD_S_ASSERT(!noexcept(ThrowingObjectTrivDtorTwo(1)));
RAD_S_ASSERT(!noexcept(ThrowingObjectTrivDtorTwo(ThrowingObjectTrivDtorTwo())));
RAD_S_ASSERT(!noexcept(ThrowingObjectTrivDtorTwo(rad::Move(rad::DeclVal<ThrowingObjectTrivDtorTwo&>()))));
RAD_S_ASSERT(!noexcept(rad::DeclVal<ThrowingObjectTrivDtorTwo>().operator=(ThrowingObjectTrivDtorTwo())));
RAD_S_ASSERT(!noexcept(rad::DeclVal<ThrowingObjectTrivDtorTwo>().operator=(rad::Move(rad::DeclVal<ThrowingObjectTrivDtorTwo&>()))));

// clang-format on

struct NonThrowingObjectTrivDtor
{
    ~NonThrowingObjectTrivDtor() = default;

    NonThrowingObjectTrivDtor() noexcept
        : m_value(0)
    {
    }

    NonThrowingObjectTrivDtor(int Value) noexcept
        : m_value(Value)
    {
    }

    NonThrowingObjectTrivDtor(std::initializer_list<int> Init) noexcept
        : m_value(*Init.begin())
    {
    }

    NonThrowingObjectTrivDtor(const NonThrowingObjectTrivDtor& Other) noexcept
        : m_value(Other.m_value)
    {
    }

    NonThrowingObjectTrivDtor(NonThrowingObjectTrivDtor&& Other) noexcept
        : m_value(rad::Move(Other.m_value))
    {
    }

    NonThrowingObjectTrivDtor& operator=(
        const NonThrowingObjectTrivDtor& Other) noexcept
    {
        m_value = Other.m_value;
        return *this;
    }

    NonThrowingObjectTrivDtor& operator=(
        NonThrowingObjectTrivDtor&& Other) noexcept
    {
        m_value = rad::Move(Other.m_value);
        return *this;
    }

    int m_value = 0;
};

// clang-format off

RAD_S_ASSERT(noexcept(rad::DeclVal<NonThrowingObjectTrivDtor>().~NonThrowingObjectTrivDtor()));
RAD_S_ASSERT(noexcept(NonThrowingObjectTrivDtor()));
RAD_S_ASSERT(noexcept(NonThrowingObjectTrivDtor(1)));
RAD_S_ASSERT(noexcept(NonThrowingObjectTrivDtor(NonThrowingObjectTrivDtor())));
RAD_S_ASSERT(noexcept(NonThrowingObjectTrivDtor(rad::Move(rad::DeclVal<NonThrowingObjectTrivDtor&>()))));
RAD_S_ASSERT(noexcept(rad::DeclVal<NonThrowingObjectTrivDtor>().operator=(NonThrowingObjectTrivDtor())));
RAD_S_ASSERT(noexcept(rad::DeclVal<NonThrowingObjectTrivDtor>().operator=(rad::Move(rad::DeclVal<NonThrowingObjectTrivDtor&>()))));

// clang-format on

struct NonThrowingObjectTrivDtorTwo
{
    ~NonThrowingObjectTrivDtorTwo() = default;

    NonThrowingObjectTrivDtorTwo() noexcept
        : m_value(0)
    {
    }

    NonThrowingObjectTrivDtorTwo(int Value) noexcept
        : m_value(Value)
    {
    }

    NonThrowingObjectTrivDtorTwo(std::initializer_list<int> Init) noexcept
        : m_value(*Init.begin())
    {
    }

    NonThrowingObjectTrivDtorTwo(
        const NonThrowingObjectTrivDtorTwo& Other) noexcept
        : m_value(Other.m_value)
    {
    }

    NonThrowingObjectTrivDtorTwo(NonThrowingObjectTrivDtorTwo&& Other) noexcept
        : m_value(rad::Move(Other.m_value))
    {
    }

    NonThrowingObjectTrivDtorTwo& operator=(
        const NonThrowingObjectTrivDtorTwo& Other) noexcept
    {
        m_value = Other.m_value;
        return *this;
    }

    NonThrowingObjectTrivDtorTwo& operator=(
        NonThrowingObjectTrivDtorTwo&& Other) noexcept
    {
        m_value = rad::Move(Other.m_value);
        return *this;
    }

    int m_value = 0;
};

// clang-format off

RAD_S_ASSERT(noexcept(rad::DeclVal<NonThrowingObjectTrivDtorTwo>().~NonThrowingObjectTrivDtorTwo()));
RAD_S_ASSERT(noexcept(NonThrowingObjectTrivDtorTwo()));
RAD_S_ASSERT(noexcept(NonThrowingObjectTrivDtorTwo(1)));
RAD_S_ASSERT(noexcept(NonThrowingObjectTrivDtorTwo(NonThrowingObjectTrivDtorTwo())));
RAD_S_ASSERT(noexcept(NonThrowingObjectTrivDtorTwo(rad::Move(rad::DeclVal<NonThrowingObjectTrivDtorTwo&>()))));
RAD_S_ASSERT(noexcept(rad::DeclVal<NonThrowingObjectTrivDtorTwo>().operator=(NonThrowingObjectTrivDtorTwo())));
RAD_S_ASSERT(noexcept(rad::DeclVal<NonThrowingObjectTrivDtorTwo>().operator=(rad::Move(rad::DeclVal<NonThrowingObjectTrivDtorTwo&>()))));

// clang-format on

} // namespace radtest
