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
#include "radiant/detail/VectorOperations.h"

namespace rad
{

/// @brief Stores a contiguous set of elements.
/// @tparam T The type of elements.
/// @tparam TAllocator Allocator type to use.
/// @tparam TInlineCount Optionally specifies a number of elements for inline
/// storage which may be used for small optimizations.
template <typename T,
          typename TAllocator RAD_ALLOCATOR_EQ(T),
          uint16_t TInlineCount = 0>
class Vector final
{
private:

    using OperationalType = detail::VectorOperations<T, TInlineCount>;
    using StorageType = EmptyOptimizedPair<TAllocator, OperationalType>;

public:

    using ThisType = Vector<T, TAllocator, TInlineCount>;
    using ValueType = T;
    using SizeType = uint32_t;
    static constexpr uint16_t InlineCount = TInlineCount;
    using AllocatorType = TAllocator;
    template <typename OtherTAllocator, uint16_t OtherTInlineCount = 0>
    using OtherType = Vector<T, OtherTAllocator, OtherTInlineCount>;

    RAD_S_ASSERT_NOTHROW_MOVE_T(T);
    RAD_S_ASSERT_ALLOCATOR_REQUIRES_T(TAllocator);

    ~Vector()
    {
        RAD_S_ASSERT_NOTHROW_DTOR(IsNoThrowDtor<T> &&
                                  IsNoThrowDtor<TAllocator>);

        Clear();
        Storage().Free(Allocator());
    }

    /// @brief Constructs empty container with default-constructed allocator.
    Vector() = default;

    /// @brief Constructs empty container with copy-constructed allocator.
    /// @param alloc Allocator to copy.
    explicit Vector(const AllocatorType& alloc)
        : m_storage(alloc)
    {
    }

    /// @brief Constructs empty container with move-constructed allocator.
    /// @param alloc Allocator to move.
    explicit Vector(AllocatorType&& alloc)
        : m_storage(Forward<AllocatorType>(alloc))
    {
    }

    ThisType& operator=(const ThisType& other) = delete;

    /// @brief Moves elements in another container into this.
    /// @param other Other container to move elements from.
    /// @return Reference to this container.
    ThisType& operator=(ThisType&& other) noexcept
    {
        other.Storage().Move(other.Allocator(), Storage());
        return *this;
    }

    /// @brief Checks if the container is empty.
    /// @return True if the container is empty, false otherwise.
    bool Empty() const noexcept
    {
        return (Storage().m_size == 0);
    }

    /// @brief Retrieves the number of elements in the container.
    /// @return Number of elements in the container.
    SizeType Size() const noexcept
    {
        return Storage().m_size;
    }

    /// @brief Retrieves the current number of elements that could be stored in
    /// the container without reallocating storage.
    /// @return Number of elements that could be stored in the container without
    /// reallocating.
    SizeType Capacity() const noexcept
    {
        return Storage().m_capacity;
    }

    /// @brief Retrieves a pointer to the first element in the contiguous set of
    /// elements. Pointer value is undefined if the container is empty.
    /// @return Pointer to the first element in the contiguous set of elements.
    ValueType* Data() noexcept
    {
        return Storage().Data();
    }

    /// @brief Retrieves a pointer to the first element in the contiguous set of
    /// elements. Pointer value is undefined if the container is empty.
    /// @return Pointer to the first element in the contiguous set of elements.
    const ValueType* Data() const noexcept
    {
        return Storage().Data();
    }

    /// @brief Returns a reference to the element at a given index. Behavior is
    /// undefined if the container is empty or if the index is outside the
    /// bounds of the contained elements.
    /// @param index Index into the contiguous set to get the element of.
    /// @return Reference to the element at the supplied index.
    ValueType& At(SizeType index) noexcept
    {
        RAD_ASSERT(index < Storage().m_size);

        return Storage().Data()[index];
    }

    /// @brief Returns a reference to the element at a given index. Behavior is
    /// undefined if the container is empty or if the index is outside the
    /// bounds of the contained elements.
    /// @param index Index into the contiguous set to get the element of.
    /// @return Reference to the element at the supplied index.
    ValueType& operator[](SizeType index) noexcept
    {
        return At(index);
    }

    /// @brief Returns a reference to the element at a given index. Behavior is
    /// undefined if the container is empty or if the index is outside the
    /// bounds of the contained elements.
    /// @param index Index into the contiguous set to get the element of.
    /// @return Reference to the element at the supplied index.
    const ValueType& At(SizeType index) const noexcept
    {
        RAD_ASSERT(index < Storage().m_size);

        return Storage().Data()[index];
    }

    /// @brief Returns a reference to the element at a given index. Behavior is
    /// undefined if the container is empty or if the index is outside the
    /// bounds of the contained elements.
    /// @param index Index into the contiguous set to get the element of.
    /// @return Reference to the element at the supplied index.
    const ValueType& operator[](SizeType index) const noexcept
    {
        return At(index);
    }

    /// @brief Returns a reference to the first element in the container.
    /// Behavior is undefined if the container is empty.
    /// @return Reference to the first element in the container.
    ValueType& Front() noexcept
    {
        return At(0);
    }

    /// @brief Returns a reference to the first element in the container.
    /// Behavior is undefined if the container is empty.
    /// @return Reference to the first element in the container.
    const ValueType& Front() const noexcept
    {
        return At(0);
    }

    /// @brief Returns a reference to the last element in the container.
    /// Behavior is undefined if the container is empty.
    /// @return Reference to the last element in the container.
    ValueType& Back() noexcept
    {
        return At(Storage().m_size - 1);
    }

    /// @brief Returns a reference to the last element in the container.
    /// Behavior is undefined if the container is empty.
    /// @return Reference to the last element in the container.
    const ValueType& Back() const noexcept
    {
        return At(Storage().m_size - 1);
    }

    /// @brief Reserves space for a number elements in the container.
    /// @param capacity Number of elements to reserve space for.
    /// @return Result reference to this container on success or an error.
    Res<ThisType&> Reserve(SizeType capacity) noexcept
    {
        return Storage().Reserve(Allocator(), capacity).OnOk(*this);
    }

    /// @brief Clears all elements from the container.
    /// @return Reference to this container.
    ThisType& Clear() noexcept
    {
        Storage().Clear();
        return *this;
    }

    /// @brief Shrinks the allocated storage to fit the current number of
    /// elements in the container.
    /// @return Result reference to this container on success or an error.
    Res<ThisType&> ShrinkToFit() noexcept
    {
        return Storage().ShrinkToFit(Allocator()).OnOk(*this);
    }

    /// @brief Swaps the elements in this container with another.
    /// @param other Other container to swap elements with.
    /// @return Reference to this container.
    ThisType& Swap(ThisType& other) noexcept
    {
        Storage().Swap(other.Storage());
        return *this;
    }

    /// @brief Resizes the number of elements in the container.
    /// @details If the current size is greater than the requested count, the
    /// container is reduced to the first count elements. If the current size is
    /// less than count a number of default-constructed elements are appended.
    /// @param count Requested size of the container.
    /// @return Result reference to this container on success or an error.
    Res<ThisType&> Resize(SizeType count) noexcept(IsNoThrowDefaultCtor<T>)
    {
        return Storage().Resize(Allocator(), count).OnOk(*this);
    }

    /// @brief Resizes the number of elements in the container.
    /// @details If the current size is greater than the requested count, the
    /// container is reduced to the first count elements. If the current size is
    /// less than count a number of copy-constructed elements are appended.
    /// @param count Requested size of the container.
    /// @param value Value to initialize new elements with.
    /// @return Result reference to this container on success or an error.
    Res<ThisType&> Resize(SizeType count,
                          const ValueType& value) noexcept(IsNoThrowCopyCtor<T>)
    {
        return Storage().Resize(Allocator(), count, value).OnOk(*this);
    }

    /// @brief Replaces the contents of the container.
    /// @param count Number of elements to assign to the container.
    /// @param value Value to initialize elements with.
    /// @return Result reference to this container on success or an error.
    Res<ThisType&> Assign(SizeType count,
                          const ValueType& value) noexcept(IsNoThrowCopyCtor<T>)
    {
        return Storage().Assign(Allocator(), count, value).OnOk(*this);
    }

#if RAD_ENABLE_STD
    /// @brief Replaces the contents of the container.
    /// @param Init List of values to initialize elements with.
    /// @return Result reference to this container on success or an error.
    Res<ThisType&> Assign(std::initializer_list<ValueType> init) noexcept(
        IsNoThrowCopyCtor<T>)
    {
        return Storage()
            .Assign(Allocator(),
                    Span<const ValueType>(init.begin(), init.end()))
            .OnOk(*this);
    }
#endif

    /// @brief Replaces the contents of the container.
    /// @param span Span of values to initialize elements with.
    /// @return Result reference to this container on success or an error.
    Res<ThisType&> Assign(Span<ValueType> span) noexcept(IsNoThrowCopyCtor<T>)
    {
        return Storage().Assign(Allocator(), span).OnOk(*this);
    }

    /// @brief Appends a new element to the end of the container.
    /// @tparam ...TArgs Argument types forward to the constructor of the
    /// element.
    /// @param ...args Arguments to forward to the constructor of the element.
    /// @return Result reference to this container on success or an error.
    template <typename... TArgs>
    Res<ThisType&> EmplaceBack(TArgs&&... args) noexcept(
        IsNoThrowCtor<T, TArgs...>)
    {
        return Storage()
            .EmplaceBack(Allocator(), Forward<TArgs>(args)...)
            .OnOk(*this);
    }

    /// @brief Appends a new element to the end of the container.
    /// @param value Value to be appended.
    /// @return Result reference to this container on success or an error.
    Res<ThisType&> PushBack(const ValueType& value) noexcept(
        IsNoThrowCopyCtor<T>)
    {
        return EmplaceBack(value);
    }

    /// @brief Appends a new element to the end of the container.
    /// @param value Value to be appended.
    /// @return Result reference to this container on success or an error.
    Res<ThisType&> PushBack(ValueType&& value) noexcept
    {
        return EmplaceBack(Forward<ValueType>(value));
    }

    /// @brief Removes the last element from the back of the container.
    /// @return Reference to this container.
    ThisType& PopBack() noexcept
    {
        Storage().PopBack();
        return *this;
    }

    /// @brief Returns the associated allocator.
    /// @return The associated allocator.
    AllocatorType GetAllocator() const noexcept
    {
        return AllocatorType(Allocator());
    }

    /// @brief Converts the container to a span of contiguous elements.
    /// @param offset Offset into the container to begin the span.
    /// @param count Number of elements to span.
    /// @return Span of elements in the container.
    Span<ValueType> ToSpan(SizeType offset = 0,
                           SizeType count = DynamicExtent) noexcept
    {
        if RAD_LIKELY (RAD_VERIFY(offset <= Size()))
        {
            if (count == DynamicExtent)
            {
                count = Size() - offset;
            }

            if RAD_LIKELY (RAD_VERIFY((count + offset) <= (Size() + 1)))
            {
                return Span<ValueType>(Data() + offset, count);
            }
        }

        return Span<ValueType>();
    }

    /// @brief Copies the elements in this container to another.
    /// @param to Container to copy elements to.
    /// @return Result reference to this container on success or an error.
    Res<ThisType&> Copy(ThisType& to) noexcept(IsNoThrowCopyCtor<T>)
    {
        // Note: for now we can only copy types with the same allocator because
        // the storage and allocator are private when the from and to vectors
        // are not exactly the same type. This problem can be solved, but
        // requires a bit of work.
        return Storage().Copy(to.Allocator(), to.Storage()).OnOk(*this);
    }

    /// @brief Moves the elements in this container to another.
    /// @param to Container to move elements to.
    /// @return Result reference to this container on success or an error.
    ThisType& Move(ThisType& to) noexcept
    {
        Storage().Move(Allocator(), to.Storage());
        return *this;
    }

    /// @brief Removes an element from back of the container.
    /// @return Removed element.
    ValueType RemoveBack() noexcept
    {
        auto value = rad::Move(Back());
        PopBack();
        return rad::Move(value);
    }

    /// @brief Seeks an element at a given index within the container.
    /// @param index Index to seek.
    /// @return Result reference to the element at requested index on success or
    /// an error if the index it outside the bounds of the container.
    Res<ValueType&> Seek(SizeType index) noexcept
    {
        if (index >= Storage().m_size)
        {
            return Error::OutOfRange;
        }

        return Storage().Data()[index];
    }

    /// @brief Seeks an element at a given index within the container.
    /// @param index Index to seek.
    /// @return Result reference to the element at requested index on success or
    /// an error if the index it outside the bounds of the container.
    Res<const ValueType&> Seek(SizeType index) const noexcept
    {
        if (index >= Storage().m_size)
        {
            return Error::OutOfRange;
        }

        return Storage().Data()[index];
    }

    /// @brief Seeks the first element.
    /// @return Result reference to the element at at the first element on
    /// success or an error if the container is empty.
    Res<ValueType&> SeekFront() noexcept
    {
        return Seek(0);
    }

    /// @brief Seeks the first element.
    /// @return Result reference to the element at at the first element on
    /// success or an error if the container is empty.
    Res<const ValueType&> SeekFront() const noexcept
    {
        return Seek(0);
    }

    /// @brief Seeks the last element.
    /// @return Result reference to the element at at the last element on
    /// success or an error if the container is empty.
    Res<ValueType&> SeekBack() noexcept
    {
        return Seek(Storage().m_size - 1);
    }

    /// @brief Seeks the last element.
    /// @return Result reference to the element at at the last element on
    /// success or an error if the container is empty.
    Res<const ValueType&> SeekBack() const noexcept
    {
        return Seek(Storage().m_size - 1);
    }

private:

    AllocatorType& Allocator() noexcept
    {
        return m_storage.First();
    }

    const AllocatorType& Allocator() const noexcept
    {
        return m_storage.First();
    }

    OperationalType& Storage() noexcept
    {
        return m_storage.Second();
    }

    const OperationalType& Storage() const noexcept
    {
        return m_storage.Second();
    }

    StorageType m_storage;
};

template <typename T, typename Allocator, uint16_t TInlineCount>
constexpr uint16_t Vector<T, Allocator, TInlineCount>::InlineCount;

/// @brief Stores a contiguous set of elements.
/// @tparam T The type of elements.
/// @tparam TInlineCount Specifies a number of elements for inline storage which
/// may be used for small optimizations.
/// @tparam TAllocator Allocator type to use.
template <typename T,
          uint16_t TInlineCount,
          typename TAllocator RAD_ALLOCATOR_EQ(T)>
using InlineVector = Vector<T, TAllocator, TInlineCount>;

template <typename T,
          typename TAllocator,
          uint16_t TInlineCount,
          typename U,
          typename UAllocator,
          uint16_t UInlineCount>
constexpr inline bool operator==(
    const Vector<T, TAllocator, TInlineCount>& left,
    const Vector<U, UAllocator, UInlineCount>& right)
{
    if (left.Size() != right.Size())
    {
        return false;
    }

    for (uint32_t i = 0; i < left.Size(); i++)
    {
        if (left[i] != right[i])
        {
            return false;
        }
    }

    return true;
}

template <typename T,
          typename TAllocator,
          uint16_t TInlineCount,
          typename U,
          typename UAllocator,
          uint16_t UInlineCount>
constexpr inline bool operator!=(
    const Vector<T, TAllocator, TInlineCount>& left,
    const Vector<U, UAllocator, UInlineCount>& right)
{
    return !(left == right);
}

template <typename T,
          typename TAllocator,
          uint16_t TInlineCount,
          typename U,
          typename UAllocator,
          uint16_t UInlineCount>
constexpr inline bool operator<(
    const Vector<T, TAllocator, TInlineCount>& left,
    const Vector<U, UAllocator, UInlineCount>& right)
{
    auto min = Min(left.Size(), right.Size());
    for (uint32_t i = 0; i < min; i++)
    {
        if (left[i] < right[i])
        {
            return true;
        }

        if (left[i] > right[i])
        {
            return false;
        }
    }

    return left.Size() < right.Size();
}

template <typename T,
          typename TAllocator,
          uint16_t TInlineCount,
          typename U,
          typename UAllocator,
          uint16_t UInlineCount>
constexpr inline bool operator>(
    const Vector<T, TAllocator, TInlineCount>& left,
    const Vector<U, UAllocator, UInlineCount>& right)
{
    return right < left;
}

template <typename T,
          typename TAllocator,
          uint16_t TInlineCount,
          typename U,
          typename UAllocator,
          uint16_t UInlineCount>
constexpr inline bool operator<=(
    const Vector<T, TAllocator, TInlineCount>& left,
    const Vector<U, UAllocator, UInlineCount>& right)
{
    return !(right < left);
}

template <typename T,
          typename TAllocator,
          uint16_t TInlineCount,
          typename U,
          typename UAllocator,
          uint16_t UInlineCount>
constexpr inline bool operator>=(
    const Vector<T, TAllocator, TInlineCount>& left,
    const Vector<U, UAllocator, UInlineCount>& right)
{
    return !(left < right);
}

} // namespace rad
