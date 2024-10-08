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

#include "radiant/TotallyRad.h"
#include "radiant/TypeTraits.h"
#include "radiant/Res.h"

namespace rad
{

template <typename T>
struct IntegerTraits;

template <typename T, typename W, T TMin, T TMax, bool = IsSigned<T>>
struct GenericIntegerTraits;

template <typename T, typename W, T TMin, T TMax>
struct GenericIntegerTraits<T, W, TMin, TMax, true>
{
    static constexpr T MIN = TMin;
    static constexpr T MAX = TMax;

    RAD_S_ASSERT(IsSigned<T> == IsSigned<W>);

    static constexpr Res<T> Add(T lhs, T rhs) noexcept
    {
        W res = static_cast<W>(lhs) + static_cast<W>(rhs);

        if ((res > MAX) || (res < MIN))
        {
            return Error::IntegerOverflow;
        }

        return static_cast<T>(res);
    }

    static constexpr Res<T> Sub(T lhs, T rhs) noexcept
    {
        W res = static_cast<W>(lhs) - static_cast<W>(rhs);

        if ((res > MAX) || (res < MIN))
        {
            return Error::IntegerOverflow;
        }

        return static_cast<T>(res);
    }

    static constexpr Res<T> Mul(T lhs, T rhs) noexcept
    {
        W res = static_cast<W>(lhs) * static_cast<W>(rhs);

        if ((res > MAX) || (res < MIN))
        {
            return Error::IntegerOverflow;
        }

        return static_cast<T>(res);
    }

    static constexpr T SaturatingAdd(T lhs, T rhs) noexcept
    {
        if (rhs > 0)
        {
            return Add(lhs, rhs).Or(MAX);
        }

        return Add(lhs, rhs).Or(MIN);
    }

    static constexpr T SaturatingSub(T lhs, T rhs) noexcept
    {
        if (rhs > 0)
        {
            return Sub(lhs, rhs).Or(MIN);
        }

        return Sub(lhs, rhs).Or(MAX);
    }

    static constexpr T SaturatingMul(T lhs, T rhs) noexcept
    {
        if ((lhs > 0) && (rhs > 0))
        {
            return Mul(lhs, rhs).Or(MAX);
        }

        if ((lhs < 0) && (rhs < 0))
        {
            return Mul(lhs, rhs).Or(MAX);
        }

        return Mul(lhs, rhs).Or(MIN);
    }
};

template <typename T, typename W, T TMin, T TMax>
struct GenericIntegerTraits<T, W, TMin, TMax, false>
{
    static constexpr T MIN = TMin;
    static constexpr T MAX = TMax;

    RAD_S_ASSERT(IsUnsigned<T> == IsUnsigned<W>);

    static constexpr Res<T> Add(T lhs, T rhs) noexcept
    {
        T res = lhs + rhs;

        if (res < lhs)
        {
            return Error::IntegerOverflow;
        }

        return res;
    }

    static constexpr Res<T> Sub(T lhs, T rhs) noexcept
    {
        T res = lhs - rhs;

        if (res > lhs)
        {
            return Error::IntegerOverflow;
        }

        return static_cast<T>(res);
    }

    static constexpr Res<T> Mul(T lhs, T rhs) noexcept
    {
        W res = static_cast<W>(lhs) * static_cast<W>(rhs);

        if (res > MAX)
        {
            return Error::IntegerOverflow;
        }

        return static_cast<T>(res);
    }

    static constexpr T SaturatingAdd(T lhs, T rhs) noexcept
    {
        return Add(lhs, rhs).Or(MAX);
    }

    static constexpr T SaturatingSub(T lhs, T rhs) noexcept
    {
        return Sub(lhs, rhs).Or(MIN);
    }

    static constexpr T SaturatingMul(T lhs, T rhs) noexcept
    {
        return Mul(lhs, rhs).Or(MAX);
    }
};

template <typename T, typename W, T TMin, T TMax>
constexpr T GenericIntegerTraits<T, W, TMin, TMax, true>::MIN;
template <typename T, typename W, T TMin, T TMax>
constexpr T GenericIntegerTraits<T, W, TMin, TMax, false>::MIN;
template <typename T, typename W, T TMin, T TMax>
constexpr T GenericIntegerTraits<T, W, TMin, TMax, true>::MAX;
template <typename T, typename W, T TMin, T TMax>
constexpr T GenericIntegerTraits<T, W, TMin, TMax, false>::MAX;

template <>
struct IntegerTraits<int8_t>
    : public GenericIntegerTraits<int8_t, int16_t, INT8_MIN, INT8_MAX>
{
};

template <>
struct IntegerTraits<uint8_t>
    : public GenericIntegerTraits<uint8_t, uint16_t, 0, UINT8_MAX>
{
};

template <>
struct IntegerTraits<int16_t>
    : public GenericIntegerTraits<int16_t, int32_t, INT16_MIN, INT16_MAX>
{
};

template <>
struct IntegerTraits<uint16_t>
    : public GenericIntegerTraits<uint16_t, uint32_t, 0, UINT16_MAX>
{
};

template <>
struct IntegerTraits<int32_t>
    : public GenericIntegerTraits<int32_t, int64_t, INT32_MIN, INT32_MAX>
{
};

template <>
struct IntegerTraits<uint32_t>
    : public GenericIntegerTraits<uint32_t, uint64_t, 0, UINT32_MAX>
{
};

template <typename T>
class RAD_NODISCARD Integer
{
public:

    using ThisType = Integer<T>;
    using ValueType = T;
    using Traits = IntegerTraits<T>;

    static constexpr T MIN = Traits::MIN;
    static constexpr T MAX = Traits::MAX;

    ~Integer() = default;

    constexpr Integer() noexcept
        : m_value()
    {
    }

    constexpr Integer(T value) noexcept
        : m_value(value)
    {
    }

    constexpr Integer(const Integer& other) noexcept
        : m_value(other.m_value)
    {
    }

    constexpr Integer(Integer&& other) noexcept
        : m_value(other.m_value)
    {
        other.m_value = T();
    }

    constexpr Integer& operator=(T value) noexcept
    {
        m_value = value;
        return *this;
    }

    constexpr Integer& operator=(const Integer& other) noexcept
    {
        m_value = other.m_value;
        return *this;
    }

    constexpr Integer& operator=(Integer&& other) noexcept
    {
        m_value = other.m_value;
        other.m_value = T();
        return *this;
    }

    constexpr explicit operator T() const noexcept
    {
        return m_value;
    }

    constexpr Integer Max(T rhs) const noexcept
    {
        return m_value > rhs ? m_value : rhs;
    }

    constexpr Integer Min(T lhs) const noexcept
    {
        return m_value < lhs ? m_value : lhs;
    }

    // checked operations

    constexpr Res<Integer> Add(T rhs) const noexcept
    {
        return Traits::Add(m_value, rhs);
    }

    constexpr Res<Integer> Sub(T rhs) const noexcept
    {
        return Traits::Sub(m_value, rhs);
    }

    constexpr Res<Integer> Mul(T rhs) const noexcept
    {
        return Traits::Mul(m_value, rhs);
    }

    // saturating operations

    RAD_NODISCARD constexpr Integer SaturatingAdd(T rhs) const noexcept
    {
        return Traits::SaturatingAdd(m_value, rhs);
    }

    RAD_NODISCARD constexpr Integer SaturatingSub(T rhs) const noexcept
    {
        return Traits::SaturatingSub(m_value, rhs);
    }

    RAD_NODISCARD constexpr Integer SaturatingMul(T rhs) const noexcept
    {
        return Traits::SaturatingMul(m_value, rhs);
    }

    // unchecked operations

    RAD_NODISCARD constexpr Integer UncheckedAdd(T rhs) const noexcept
    {
        return m_value + rhs;
    }

    RAD_NODISCARD constexpr Integer UncheckedSub(T rhs) const noexcept
    {
        return m_value - rhs;
    }

    RAD_NODISCARD constexpr Integer UncheckedMul(T rhs) const noexcept
    {
        return m_value * rhs;
    }

private:

    T m_value;
};

template <typename T>
constexpr T Integer<T>::MIN;
template <typename T>
constexpr T Integer<T>::MAX;

using i8 = Integer<int8_t>;
using u8 = Integer<uint8_t>;
using i16 = Integer<int16_t>;
using u16 = Integer<uint16_t>;
using i32 = Integer<int32_t>;
using u32 = Integer<uint32_t>;

template <typename T>
constexpr inline bool operator==(const Integer<T>& lhs,
                                 const Integer<T>& rhs) noexcept
{
    return static_cast<T>(lhs) == static_cast<T>(rhs);
}

template <typename T>
constexpr inline bool operator!=(const Integer<T>& lhs,
                                 const Integer<T>& rhs) noexcept
{
    return static_cast<T>(lhs) != static_cast<T>(rhs);
}

template <typename T>
constexpr inline bool operator<(const Integer<T>& lhs,
                                const Integer<T>& rhs) noexcept
{
    return static_cast<T>(lhs) < static_cast<T>(rhs);
}

template <typename T>
constexpr inline bool operator<=(const Integer<T>& lhs,
                                 const Integer<T>& rhs) noexcept
{
    return static_cast<T>(lhs) <= static_cast<T>(rhs);
}

template <typename T>
constexpr inline bool operator>(const Integer<T>& lhs,
                                const Integer<T>& rhs) noexcept
{
    return static_cast<T>(lhs) > static_cast<T>(rhs);
}

template <typename T>
constexpr inline bool operator>=(const Integer<T>& lhs,
                                 const Integer<T>& rhs) noexcept
{
    return static_cast<T>(lhs) >= static_cast<T>(rhs);
}

template <typename T>
constexpr inline bool operator==(const Integer<T>& lhs, const T& rhs) noexcept
{
    return static_cast<T>(lhs) == rhs;
}

template <typename T>
constexpr inline bool operator!=(const Integer<T>& lhs, const T& rhs) noexcept
{
    return static_cast<T>(lhs) != rhs;
}

template <typename T>
constexpr inline bool operator<(const Integer<T>& lhs, const T& rhs) noexcept
{
    return static_cast<T>(lhs) < rhs;
}

template <typename T>
constexpr inline bool operator<=(const Integer<T>& lhs, const T& rhs) noexcept
{
    return static_cast<T>(lhs) <= rhs;
}

template <typename T>
constexpr inline bool operator>(const Integer<T>& lhs, const T& rhs) noexcept
{
    return static_cast<T>(lhs) > rhs;
}

template <typename T>
constexpr inline bool operator>=(const Integer<T>& lhs, const T& rhs) noexcept
{
    return static_cast<T>(lhs) >= rhs;
}

template <typename T, EnIf<!IsSame<T, int> && IsSigned<T>, int> = 0>
constexpr inline bool operator==(const Integer<T>& lhs, int rhs) noexcept
{
    return static_cast<T>(lhs) == rhs;
}

template <typename T, EnIf<!IsSame<T, int> && IsSigned<T>, int> = 0>
constexpr inline bool operator!=(const Integer<T>& lhs, int rhs) noexcept
{
    return static_cast<T>(lhs) != rhs;
}

template <typename T, EnIf<!IsSame<T, int> && IsSigned<T>, int> = 0>
constexpr inline bool operator<(const Integer<T>& lhs, int rhs) noexcept
{
    return static_cast<T>(lhs) < rhs;
}

template <typename T, EnIf<!IsSame<T, int> && IsSigned<T>, int> = 0>
constexpr inline bool operator<=(const Integer<T>& lhs, int rhs) noexcept
{
    return static_cast<T>(lhs) <= rhs;
}

template <typename T, EnIf<!IsSame<T, int> && IsSigned<T>, int> = 0>
constexpr inline bool operator>(const Integer<T>& lhs, int rhs) noexcept
{
    return static_cast<T>(lhs) > rhs;
}

template <typename T, EnIf<!IsSame<T, int> && IsSigned<T>, int> = 0>
constexpr inline bool operator>=(const Integer<T>& lhs, int rhs) noexcept
{
    return static_cast<T>(lhs) >= rhs;
}

template <typename T, EnIf<!IsSame<T, unsigned int> && IsUnsigned<T>, int> = 0>
constexpr inline bool operator==(const Integer<T>& lhs,
                                 unsigned int rhs) noexcept
{
    return static_cast<T>(lhs) == rhs;
}

template <typename T, EnIf<!IsSame<T, unsigned int> && IsUnsigned<T>, int> = 0>
constexpr inline bool operator!=(const Integer<T>& lhs,
                                 unsigned int rhs) noexcept
{
    return static_cast<T>(lhs) != rhs;
}

template <typename T, EnIf<!IsSame<T, unsigned int> && IsUnsigned<T>, int> = 0>
constexpr inline bool operator<(const Integer<T>& lhs,
                                unsigned int rhs) noexcept
{
    return static_cast<T>(lhs) < rhs;
}

template <typename T, EnIf<!IsSame<T, unsigned int> && IsUnsigned<T>, int> = 0>
constexpr inline bool operator<=(const Integer<T>& lhs,
                                 unsigned int rhs) noexcept
{
    return static_cast<T>(lhs) <= rhs;
}

template <typename T, EnIf<!IsSame<T, unsigned int> && IsUnsigned<T>, int> = 0>
constexpr inline bool operator>(const Integer<T>& lhs,
                                unsigned int rhs) noexcept
{
    return static_cast<T>(lhs) > rhs;
}

template <typename T, EnIf<!IsSame<T, unsigned int> && IsUnsigned<T>, int> = 0>
constexpr inline bool operator>=(const Integer<T>& lhs,
                                 unsigned int rhs) noexcept
{
    return static_cast<T>(lhs) >= rhs;
}

} // namespace rad
