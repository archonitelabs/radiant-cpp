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

#include "radiant/Memory.h"
#include "radiant/EmptyOptimizedPair.h"

namespace rad
{

namespace detail
{

template <typename T, bool = IsPoly<T>>
class _UniqueStorage;

template <typename T>
class _UniqueStorage<T, true>
{
public:

    using ThisType = _UniqueStorage<T, true>;
    using ValueType = T;
    using PointerType = T*;

    ~_UniqueStorage() = default;

    _UniqueStorage() noexcept = default;

    _UniqueStorage(PointerType value, void* fin) noexcept
        : m_value(value),
          m_fin(fin)
    {
    }

    _UniqueStorage(const ThisType& other) noexcept
        : m_value(other.m_value),
          m_fin(other.m_fin)
    {
    }

    ThisType& operator=(const ThisType& other) noexcept
    {
        m_value = other.m_value;
        m_fin = other.m_fin;
        return *this;
    }

    void* Destruct() noexcept
    {
        RAD_S_ASSERT_NOTHROW_DTOR_T(ValueType);

        m_value->~ValueType();
        return m_fin;
    }

    PointerType Value() noexcept
    {
        return m_value;
    }

    const PointerType Value() const noexcept
    {
        return m_value;
    }

private:

    PointerType m_value{ nullptr };
    void* m_fin{ nullptr };
};

template <typename T>
struct _UniqueStorage<T, false>
{
public:

    using ThisType = _UniqueStorage<T, true>;
    using ValueType = T;
    using PointerType = T*;

    ~_UniqueStorage() = default;

    _UniqueStorage() noexcept = default;

    _UniqueStorage(PointerType value, void* fin) noexcept
        : m_value(value)
    {
        RAD_UNUSED(fin);
    }

    _UniqueStorage(const ThisType& other) noexcept
        : m_value(other.m_value)
    {
    }

    ThisType& operator=(const ThisType& other) noexcept
    {
        m_value = other.m_value;
        return *this;
    }

    void* Destruct() noexcept
    {
        RAD_S_ASSERT_NOTHROW_DTOR_T(ValueType);

        m_value->~ValueType();
        return m_value;
    }

    PointerType Value() noexcept
    {
        return m_value;
    }

    const PointerType Value() const noexcept
    {
        return m_value;
    }

private:

    PointerType m_value{ nullptr };
};

} // namespace detail

template <typename T, typename TAlloc _RAD_DEFAULT_ALLOCATOR_EQ(T)>
class UniquePtr
{
private:

    using StorageType = detail::_UniqueStorage<T>;
    using PairType = EmptyOptimizedPair<TAlloc, StorageType>;

public:

    using ThisType = UniquePtr<StorageType, TAlloc>;
    using ValueType = T;
    using PointerType = T*;
    using AllocatorType = TAlloc;

    RAD_NOT_COPYABLE(UniquePtr);

    ~UniquePtr() noexcept
    {
        Reset();
    }

    UniquePtr() noexcept = default;

    UniquePtr(rad::nullptr_t) noexcept
        : m_pair()
    {
    }

    operator bool() const noexcept
    {
        return (Get() != nullptr);
    }

    bool operator!=(rad::nullptr_t) const noexcept
    {
        return (Get() != nullptr);
    }

    bool operator==(rad::nullptr_t) const noexcept
    {
        return (Get() == nullptr);
    }

    PointerType Get() noexcept
    {
        return Storage().Value();
    }

    const PointerType Get() const noexcept
    {
        return Storage().Value();
    }

    PointerType operator->() noexcept
    {
        return Get();
    }

    const PointerType operator->() const noexcept
    {
        return Get();
    }

    ValueType& operator*() noexcept
    {
        return *Get();
    }

    const ValueType& operator*() const noexcept
    {
        return *Get();
    }

    AllocatorType GetAllocator() const noexcept
    {
        return AllocatorType(Allocator());
    }

    PointerType Release() noexcept
    {
        PointerType p = Get();
        m_pair.Second() = StorageType(nullptr, nullptr);
        return p;
    }

    void Reset(rad::nullptr_t = nullptr) noexcept
    {
        if (Get() != nullptr)
        {
            Allocator().Free(static_cast<PointerType>(Storage().Destruct()));
            Storage() = StorageType(nullptr, nullptr);
        }
    }

private:

    StorageType& Storage()
    {
        return m_pair.Second();
    }

    const StorageType& Storage() const
    {
        return m_pair.Second();
    }

    AllocatorType& Allocator()
    {
        return m_pair.First();
    }

    PairType m_pair;
};

} // namespace rad
