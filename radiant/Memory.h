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

//
// Users of Radiant may define their own default allocator. Radiant itself
// defines no default allocator. This is done to allow users of the library to
// enforce verbosity if they wish.
//
#ifdef RAD_DEFAULT_ALLOCATOR
#define _RAD_DEFAULT_ALLOCATOR(x)    RAD_DEFAULT_ALLOCATOR<x>
#define _RAD_DEFAULT_ALLOCATOR_EQ(x) = RAD_DEFAULT_ALLOCATOR<x>
#else
#define _RAD_DEFAULT_ALLOCATOR(x)
#define _RAD_DEFAULT_ALLOCATOR_EQ(x)
#endif

namespace rad
{

#if 0
/// @brief Example Radiant compatible allocator.
/// @details Note that this allocator has no implementation and exists only as
/// an example for allocator implementors. Consider it a concept contract for an
/// allocator compatible with Radiant.
/// @tparam T The type of object to allocate.
template <typename T>
class Allocator
{
public:

    /// @brief Trait indicating if freeing memory is required.
    /// @details If this is false, users of the allocator need not free memory
    /// that was allocated. This trait enables certain run or compile time
    /// optimizations. An example of an allocator that might leverage this trait
    /// is a stack allocator. When false, an allocator implementor may implement
    /// a no-op Free function.
    static constexpr bool NeedsFree = true;

    /// @brief Trait indicating if the allocator supports reallocations.
    /// @details If this is false, users of the allocator should not call
    /// Realloc as the allocator does not support it. In some scenarios it may
    /// be possible and more efficient to reallocate memory in place rather than
    /// allocating new memory. When false, an allocator implementor may
    /// implement a no-op Realloc function.
    static constexpr bool HasRealloc = true;

    /// @brief Trait indicating if the allocator supports allocating by bytes.
    /// @details If this is true, users of the allocator may call the Bytes
    /// suffixed functions to allocate and free memory as bytes rather than as
    /// number of T elements. An example is cases where some memory needs
    /// allocated before or after T. However it may not be possible for all types
    /// of allocators, such as a slab/lookaside allocator. When false, an
    /// allocator may implement appropriate no-op functions for the various Bytes
    /// suffixes function.
    static constexpr bool HasAllocBytes = true;

    using ThisType = Allocator<T>;
    using ValueType = T;
    using SizeType = uint32_t;
    using DifferenceType = ptrdiff_t;

    ~Allocator() = default;

    constexpr Allocator() noexcept = default;

    constexpr Allocator(const Allocator&) noexcept = default;

    template <typename U>
    constexpr Allocator(const Allocator<U>&) noexcept
    {
    }

    template <typename U>
    struct Rebind
    {
        using Other = Allocator<U>;
    };

    /// @brief Frees memory allocated by Alloc.
    /// @param ptr Pointer to the memory to free.
    void Free(ValueType* ptr) noexcept;

    /// @brief Allocates memory for count number of T.
    /// @param count The number of T to allocate memory for.
    /// @return Pointer to the allocated memory.
    ValueType* Alloc(SizeType count);

    /// @brief Reallocates memory for count number of T.
    /// @param ptr Pointer to the memory to reallocate. If nullptr a new memory block is allocated.
    /// @param count The number of T to allocate memory for.
    /// @return Pointer to the reallocated memory.
    ValueType* Realloc(ValueType* ptr, SizeType count);

    /// @brief Frees memory allocated by AllocBytes.
    /// @param ptr Pointer to the memory to free.
    void FreeBytes(void* ptr) noexcept;

    /// @brief Allocates memory for size number of bytes.
    /// @param size The number of bytes to allocate memory for.
    /// @return Pointer to the allocated memory.
    void* AllocBytes(SizeType size);

    /// @brief Reallocates memory for size number of bytes.
    /// @param ptr Pointer to the memory to reallocate. If nullptr a new memory block is allocated.
    /// @param size The number of bytes to allocate memory for.
    /// @return Pointer to the reallocated memory.
    void* ReallocBytes(void* ptr, SizeType size);
};
#endif

} // namespace rad
