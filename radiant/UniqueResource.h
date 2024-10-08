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

#pragma once

#include "radiant/TotallyRad.h"
#include "radiant/Utility.h"

namespace rad
{

/// @brief Container to manage lifetime of some resource.
/// @details This contains is semantically similar to a unique pointer however
/// tailored to managing things like file handles.
/// @tparam Policy The policy for the resource must define and implement static
/// types and functionality for how to validate or close the resource. See
/// UniqueResourcePolicy<>.
template <typename Policy>
class UniqueResource
{
public:

    using PolicyType = Policy;
    using ValueType = typename Policy::ValueType;
    static constexpr ValueType InvalidValue = Policy::InvalidValue;
    using ThisType = UniqueResource<Policy>;

    RAD_S_ASSERTMSG(IsTriv<ValueType>,
                    "UniqueResource manages a trivial type.");
    RAD_S_ASSERTMSG(noexcept(PolicyType::IsValid(DeclVal<const ValueType&>())),
                    "UniqueResource validator may not throw.");
    RAD_S_ASSERTMSG(noexcept(PolicyType::Close(DeclVal<ValueType&>())),
                    "UniqueResource closer may not throw.");

    RAD_NOT_COPYABLE(UniqueResource);

    ~UniqueResource()
    {
        if (Policy::IsValid(static_cast<const ValueType&>(m_value)))
        {
            Policy::Close(m_value);
        }
    }

    /// @brief Default constructs the resource with an invalid value.
    constexpr UniqueResource() noexcept
        : m_value(InvalidValue)
    {
    }

    /// @brief Explicit construction of the resource with some value.
    /// @param value The resource value to store.
    constexpr explicit UniqueResource(const ValueType& value) noexcept
        : m_value(value)
    {
    }

    /// @brief Move constructs this resource from another.
    /// @param other Other resource to move into this one.
    constexpr UniqueResource(ThisType&& other) noexcept
        : m_value(other.m_value)
    {
        other.m_value = InvalidValue;
    }

    /// @brief Move assigns a resource to this.
    /// @param other Other resource to move into this one.
    ThisType& operator=(ThisType&& other) noexcept
    {
        if RAD_LIKELY (AddrOf(other) != this)
        {
            Reset(other.m_value);
            other.m_value = InvalidValue;
        }
        return *this;
    }

    /// @brief Checks if contained resource is valid.
    /// @return True if the contained resource is valid, false otherwise.
    constexpr bool IsValid() const noexcept
    {
        return Policy::IsValid(static_cast<const ValueType&>(m_value));
    }

    /// @brief Checks if the contained resource is valid.
    constexpr explicit operator bool() const
    {
        return IsValid();
    }

    /// @brief Resets the contained resource with a different value.
    /// @param value The resource value to store.
    constexpr void Reset(const ValueType& value = InvalidValue) noexcept
    {
        auto prev = m_value;
        m_value = value;

        if (Policy::IsValid(static_cast<const ValueType&>(prev)))
        {
            Policy::Close(prev);
        }
    }

    /// @brief Retrieves the resource.
    constexpr ValueType& Get() noexcept
    {
        return m_value;
    }

    /// @brief Retrieves the resource.
    constexpr const ValueType& Get() const noexcept
    {
        return m_value;
    }

    /// @brief Releases ownership of the resource to the caller.
    /// @return The previously managed resource from this object.
    RAD_NODISCARD constexpr ValueType Release() noexcept
    {
        auto value = m_value;
        m_value = InvalidValue;
        return value;
    }

    /// @brief Swaps two resource object's managed resource.
    /// @param other Other resource object to swap with this.
    constexpr void Swap(ThisType& other) noexcept
    {
        auto value = m_value;
        m_value = other.m_value;
        other.m_value = value;
    }

    /// @brief Helper function for putting something into this object.
    /// @details Useful for opening resources using an API where a parameter is
    /// an output parameter.
    /// @return Pointer to the internal storage of this object.
    ValueType* Put() noexcept
    {
        RAD_ASSERT(m_value == InvalidValue);
        Reset();
        return &m_value;
    }

    /// @brief Overloaded address of operator to simplify the semantics of
    /// putting information into this object.
    /// @return Pointer to the internal storage of this object.
    ValueType* operator&() noexcept
    {
        return Put();
    }

private:

    ValueType m_value;
};

template <typename Policy>
constexpr typename Policy::ValueType UniqueResource<Policy>::InvalidValue;

template <typename Policy>
constexpr inline bool operator==(const UniqueResource<Policy>& l,
                                 const UniqueResource<Policy>& r)
{
    return l.Get() == r.Get();
}

template <typename Policy>
constexpr inline bool operator!=(const UniqueResource<Policy>& l,
                                 const UniqueResource<Policy>& r)
{
    return !(l == r);
}

template <typename Policy>
constexpr inline bool operator<(const UniqueResource<Policy>& l,
                                const UniqueResource<Policy>& r)
{
    return l.Get() < r.Get();
}

template <typename Policy>
constexpr inline bool operator>(const UniqueResource<Policy>& l,
                                const UniqueResource<Policy>& r)
{
    return r < l;
}

template <typename Policy>
constexpr inline bool operator<=(const UniqueResource<Policy>& l,
                                 const UniqueResource<Policy>& r)
{
    return !(l > r);
}

template <typename Policy>
constexpr inline bool operator>=(const UniqueResource<Policy>& l,
                                 const UniqueResource<Policy>& r)
{
    return !(l < r);
}

template <typename Policy>
constexpr inline bool operator==(const UniqueResource<Policy>& l,
                                 const typename Policy::ValueType& r)
{
    return l.Get() == r;
}

template <typename Policy>
constexpr inline bool operator!=(const UniqueResource<Policy>& l,
                                 const typename Policy::ValueType& r)
{
    return !(l == r);
}

template <typename Policy>
constexpr inline bool operator<(const UniqueResource<Policy>& l,
                                const typename Policy::ValueType& r)
{
    return l.Get() < r;
}

template <typename Policy>
constexpr inline bool operator>(const UniqueResource<Policy>& l,
                                const typename Policy::ValueType& r)
{
    return r < l;
}

template <typename Policy>
constexpr inline bool operator<=(const UniqueResource<Policy>& l,
                                 const typename Policy::ValueType& r)
{
    return !(l > r);
}

template <typename Policy>
constexpr inline bool operator>=(const UniqueResource<Policy>& l,
                                 const typename Policy::ValueType& r)
{
    return !(l < r);
}

template <typename Policy>
constexpr inline bool operator==(const typename Policy::ValueType& l,
                                 const UniqueResource<Policy>& r)
{
    return l == r.Get();
}

template <typename Policy>
constexpr inline bool operator!=(const typename Policy::ValueType& l,
                                 const UniqueResource<Policy>& r)
{
    return !(l == r);
}

template <typename Policy>
constexpr inline bool operator<(const typename Policy::ValueType& l,
                                const UniqueResource<Policy>& r)
{
    return l < r.Get();
}

template <typename Policy>
constexpr inline bool operator>(const typename Policy::ValueType& l,
                                const UniqueResource<Policy>& r)
{
    return r < l;
}

template <typename Policy>
constexpr inline bool operator<=(const typename Policy::ValueType& l,
                                 const UniqueResource<Policy>& r)
{
    return !(l > r);
}

template <typename Policy>
constexpr inline bool operator>=(const typename Policy::ValueType& l,
                                 const UniqueResource<Policy>& r)
{
    return !(l < r);
}

/// @brief Default resource policy for convenience.
/// @details The resource container enables users to define their own policy
/// verbosely. Which is useful in situations where a resource may have more than
/// one invalid states. This default policy enables a short-hand for most use
/// cases through UniqueResourceDef<>.
/// @tparam T The type of resource to manage.
/// @tparam Closer Structure which implements how to close the resource.
/// @tparam Invalid The value of the resource which indicates it is invalid.
template <typename T, typename Closer, T Invalid = static_cast<T>(0)>
struct UniqueResourcePolicy
{
    using ValueType = T;
    static constexpr T InvalidValue = Invalid;

    static constexpr bool IsValid(const T& value) noexcept
    {
        return value != InvalidValue;
    }

    static constexpr void Close(T& value) noexcept
    {
        RAD_S_ASSERTMSG(noexcept(Closer::Close(value)),
                        "UniqueResourcePolicy closer may not throw");

        Closer::Close(value);
    }
};

template <typename T, typename Closer, T Invalid>
constexpr T UniqueResourcePolicy<T, Closer, Invalid>::InvalidValue;

/// @brief Helper defining template for a resource.
/// @tparam T The type of resource to manage.
/// @tparam Closer Structure which implements how to close the resource.
/// @tparam Invalid The value of the resource which indicates it is invalid.
template <typename T, typename Closer, T Invalid = static_cast<T>(0)>
using UniqueResourceDef =
    UniqueResource<UniqueResourcePolicy<T, Closer, Invalid>>;

} // namespace rad
