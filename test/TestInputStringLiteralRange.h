// Copyright 2024 The Radiant Authors.
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

#include <stdint.h>

namespace radtest
{

class StringLiteralSentinel
{
};

class StringLiteralIterator
{
public:

    using difference_type = ptrdiff_t;
    using value_type = char;

    StringLiteralIterator() = default;

    explicit StringLiteralIterator(const char* data)
        : m_data(data)
    {
    }

    value_type operator*() const
    {
        return m_data ? *m_data : 0;
    }

    StringLiteralIterator& operator++()
    {
        ++m_data;
        return *this;
    }

    void operator++(int)
    {
        ++m_data;
    }

    friend bool operator==(const StringLiteralIterator& lhs,
                           StringLiteralSentinel)
    {
        return *lhs == 0;
    }

    friend bool operator==(StringLiteralSentinel,
                           const StringLiteralIterator& rhs)
    {
        return *rhs == 0;
    }

    friend bool operator!=(const StringLiteralIterator& lhs,
                           StringLiteralSentinel)
    {
        return *lhs != 0;
    }

    friend bool operator!=(StringLiteralSentinel,
                           const StringLiteralIterator& rhs)
    {
        return *rhs != 0;
    }

private:

    const char* m_data = nullptr;
};

// forward only, non-common range, as a stress on range member functions
class TestInputStringLiteralRange
{
public:

    explicit TestInputStringLiteralRange(const char* data)
        : m_data(data)
    {
    }

    StringLiteralIterator begin()
    {
        return StringLiteralIterator(m_data);
    }

    StringLiteralSentinel end()
    {
        return StringLiteralSentinel{};
    }

private:

    const char* m_data = nullptr;
};

} // namespace radtest
