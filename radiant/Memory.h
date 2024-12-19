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
#include "radiant/Algorithm.h"
#include "radiant/TypeTraits.h"
#include "radiant/Utility.h"
#include "radiant/detail/Meta.h"

#include <stddef.h>

#if RAD_ENABLE_STD
#include <memory>
#endif

//
// Users of Radiant may define their own default allocator. Radiant itself
// defines no default allocator. This is done to allow users of the library to
// enforce verbosity if they wish.
//
#ifdef RAD_DEFAULT_ALLOCATOR
#define RAD_ALLOCATOR(x)    RAD_DEFAULT_ALLOCATOR
#define RAD_ALLOCATOR_EQ(x) = RAD_DEFAULT_ALLOCATOR
#else
#define RAD_ALLOCATOR(x)
#define RAD_ALLOCATOR_EQ(x)
#endif

namespace rad
{

namespace detection
{

// helper macro for stamping out traits that default to false
#define RAD_TRAIT_DETECTOR(Trait)                                              \
    template <typename T, typename = void>                                     \
    struct Trait                                                               \
    {                                                                          \
        static constexpr bool Val = false;                                     \
    };                                                                         \
                                                                               \
    template <typename T>                                                      \
    struct Trait<T, meta::VoidT<decltype(T::Trait)>>                           \
    {                                                                          \
        static constexpr bool Val = T::Trait;                                  \
    }

RAD_TRAIT_DETECTOR(PropagateOnCopy);
RAD_TRAIT_DETECTOR(PropagateOnMoveAssignment);
RAD_TRAIT_DETECTOR(PropagateOnSwap);
RAD_TRAIT_DETECTOR(HasConstructAndDestroy);
RAD_TRAIT_DETECTOR(HasTypedAllocations);

#undef RAD_TRAIT_DETECTOR

// If your class is empty, then you can use the default of "True" for
// IsAlwaysEqual. Everyone else needs to define IsAlwaysEqual in their class.
template <typename T, typename = void>
struct IsAlwaysEqual
{
    RAD_S_ASSERTMSG(IsEmpty<T>,
                    "Non-empty Allocators must define IsAlwaysEqual");
    static constexpr bool Val = true;
};

template <typename T>
struct IsAlwaysEqual<T, meta::VoidT<decltype(T::IsAlwaysEqual)>>
{
    static constexpr bool Val = T::IsAlwaysEqual;
};

} // namespace detection

template <typename AllocT>
class AllocTraits
{
public:

    static constexpr bool PropagateOnCopy =
        detection::PropagateOnCopy<AllocT>::Val; // defaults to false
    static constexpr bool PropagateOnMoveAssignment =
        detection::PropagateOnMoveAssignment<AllocT>::Val; // defaults to false
    static constexpr bool PropagateOnSwap =
        detection::PropagateOnSwap<AllocT>::Val; // defaults to false
    static constexpr bool IsAlwaysEqual =
        detection::IsAlwaysEqual<AllocT>::Val; // defaults to true when is_empty
    static constexpr bool HasConstructAndDestroy =
        detection::HasConstructAndDestroy<AllocT>::Val; // defaults to false
    static constexpr bool HasTypedAllocations =
        detection::HasTypedAllocations<AllocT>::Val; // defaults to false

    static constexpr size_t MaxSize = ~size_t(0);

    // basis operations
    static void* AllocBytes(AllocT& a, size_t size)
    {
        return a.AllocBytes(size);
    }

    static void FreeBytes(AllocT& a, void* ptr, size_t size) noexcept
    {
        RAD_S_ASSERTMSG(noexcept(a.FreeBytes(ptr, size)),
                        "Allocator::FreeBytes must be noexcept");
        a.FreeBytes(ptr, size);
    }

    // typed operations
    template <typename T>
    static constexpr T* Alloc(AllocT& a, size_t n)
    {
        return AllocImpl<T>(IntegralConstant<bool, HasTypedAllocations>{},
                            a,
                            n);
    }

    template <typename T>
    static constexpr void Free(AllocT& a, T* p, size_t n) noexcept
    {
        FreeImpl(IntegralConstant<bool, HasTypedAllocations>{}, a, p, n);
    }

    // constexpr construct and destroy operations
    template <class T, class... Args>
    static constexpr T* Construct(AllocT& a, T* p, Args&&... args)
    {
        return ConstructImpl(IntegralConstant<bool, HasConstructAndDestroy>{},
                             a,
                             p,
                             Forward<Args>(args)...);
    }

    template <class T>
    static constexpr void Destroy(AllocT& a, T* p) noexcept
    {
        DestroyImpl(IntegralConstant<bool, HasConstructAndDestroy>{}, a, p);
    }

    static constexpr bool Equal(const AllocT& a, const AllocT& b) noexcept
    {
        return EqualImpl(IntegralConstant<bool, IsAlwaysEqual>{}, a, b);
    }

    static constexpr void PropagateOnMoveIfNeeded(AllocT& dest,
                                                  AllocT& src) noexcept
    {
        return PropagateIfNeededImpl(IntegralConstant < bool,
                                     PropagateOnMoveAssignment &&
                                         !IsAlwaysEqual > {},
                                     dest,
                                     src);
    }

    static constexpr void PropagateOnCopyIfNeeded(AllocT& a, AllocT& b) noexcept
    {
        return PropagateIfNeededImpl(IntegralConstant < bool,
                                     PropagateOnCopy && !IsAlwaysEqual > {},
                                     a,
                                     b);
    }

    static constexpr void PropagateOnSwapIfNeeded(AllocT& a, AllocT& b) noexcept
    {
        return PropagateOnSwapIfNeededImpl(IntegralConstant < bool,
                                           PropagateOnSwap &&
                                               !IsAlwaysEqual > {},
                                           a,
                                           b);
    }

    static constexpr AllocT SelectAllocOnCopy(const AllocT& src_alloc) noexcept
    {
        RAD_S_ASSERTMSG(PropagateOnCopy || IsAlwaysEqual ||
                            IsDefaultCtor<AllocT>,
                        "You are attempting to copy / clone a container, but "
                        "your allocator doesn't want to PropagateOnCopy, and "
                        "we can't default construct your allocator.");
        return SelectAllocOnCopyImpl(IntegralConstant < bool,
                                     IsAlwaysEqual || PropagateOnCopy > {},
                                     src_alloc);
    }

private:

    // static assert the primary requirements.  There are additional
    // requirements in some of the customization points, and we static assert
    // if you provide the customization without meeting the additional
    // requirements.
    RAD_S_ASSERTMSG(IsNoThrowDtor<AllocT>                    //
                        && IsNoThrowCopyCtor<AllocT>         //
                            && IsNoThrowCopyAssign<AllocT>   //
                                && IsNoThrowMoveCtor<AllocT> //
                                    && IsNoThrowMoveAssign<AllocT>,
                    "Allocator requirements not met");

    template <typename T>
    static constexpr T* AllocImpl(TrueType, // HasTypedAllocations
                                  AllocT& a,
                                  size_t n)
    {
        return a.template Alloc<T>(n);
    }

    template <typename T>
    static constexpr T* AllocImpl(FalseType, // !HasTypedAllocations
                                  AllocT& a,
                                  size_t n)
    {
        if (n > MaxSize / sizeof(T))
        {
            // If you want to keep returning nullptr, make your
            // HandleSizeOverflow a no-op. If you want an exception or an
            // assertion, then do that instead.
            a.HandleSizeOverflow();
            return nullptr;
        }

        void* mem = a.AllocBytes(n * sizeof(T));
        return static_cast<T*>(mem);
    }

    template <typename T>
    static constexpr void FreeImpl(TrueType, // HasTypedAllocations
                                   AllocT& a,
                                   T* p,
                                   size_t n) noexcept
    {
        RAD_S_ASSERTMSG(noexcept(a.Free(p, n)),
                        "Allocator::Free must be noexcept");
        return a.Free(p, n);
    }

    template <typename T>
    static constexpr void FreeImpl(FalseType, // !HasTypedAllocations
                                   AllocT& a,
                                   T* p,
                                   size_t n) noexcept
    {
        RAD_FAST_FAIL((p == nullptr) || (n <= MaxSize / sizeof(T)));
        a.FreeBytes(p, n * sizeof(T));
    }

    template <class T, class... Args>
    static constexpr T* ConstructImpl(TrueType, // HasConstructAndDestroy
                                      AllocT& a,
                                      T* p,
                                      Args&&... args)
    {
        return a.Construct(p, Forward<Args>(args)...);
    }

    template <class T, class... Args>
    static constexpr T* ConstructImpl(FalseType, // !HasConstructAndDestroy
                                      AllocT& a,
                                      T* p,
                                      Args&&... args)
    {
        RAD_UNUSED(a);
        return ::new (static_cast<void*>(p)) T(Forward<Args>(args)...);
    }

    template <class T>
    static constexpr void DestroyImpl(TrueType, // HasConstructAndDestroy
                                      AllocT& a,
                                      T* p) noexcept
    {
        RAD_S_ASSERTMSG(noexcept(a.Destroy(p)),
                        "Allocator::Destroy must be noexcept");
        a.Destroy(p);
    }

    template <class T>
    static constexpr void DestroyImpl(FalseType, // !HasConstructAndDestroy
                                      AllocT& a,
                                      T* p) noexcept
    {
        RAD_UNUSED(a);
        p->~T();
    }

    static constexpr bool EqualImpl(TrueType, // IsAlwaysEqual
                                    const AllocT& a,
                                    const AllocT& b) noexcept
    {
        RAD_UNUSED(a);
        RAD_UNUSED(b);
        return true;
    }

    static constexpr bool EqualImpl(FalseType, // !IsAlwaysEqual
                                    const AllocT& a,
                                    const AllocT& b) noexcept
    {
        RAD_S_ASSERTMSG(noexcept(a == b),
                        "Allocator::operator== must be noexcept");
        return a == b;
    }

    static constexpr void PropagateIfNeededImpl(
        TrueType, // PropagateOn* && !IsAlwaysEqual
        AllocT& dest,
        AllocT& src) noexcept
    {
        if (!(dest == src)) // avoid using !=, as AllocT may not define it
        {
            dest = src;
        }
    }

    static constexpr void PropagateIfNeededImpl(
        FalseType, // ! (PropagateOn* && !IsAlwaysEqual)
        AllocT& dest,
        AllocT& src) noexcept
    {
        RAD_UNUSED(dest);
        RAD_UNUSED(src);
    }

    static constexpr void PropagateOnSwapIfNeededImpl(
        TrueType, // PropagateOnSwap && !IsAlwaysEqual
        AllocT& a,
        AllocT& b) noexcept
    {
        using rad::Swap;
        Swap(a, b);
    }

    static constexpr void PropagateOnSwapIfNeededImpl(
        FalseType, // ! (PropagateOnSwap && !IsAlwaysEqual)
        AllocT& a,
        AllocT& b) noexcept
    {
        RAD_UNUSED(a);
        RAD_UNUSED(b);
    }

    static constexpr AllocT SelectAllocOnCopyImpl(
        TrueType, // IsAlwaysEqual || PropagateOnCopy
        const AllocT& src_alloc) noexcept
    {
        return src_alloc;
    }

    static constexpr AllocT SelectAllocOnCopyImpl(
        FalseType, // !(IsAlwaysEqual || PropagateOnCopy)
        const AllocT& src_alloc) noexcept
    {
        RAD_UNUSED(src_alloc);
        return AllocT();
    }
};

#if RAD_ENABLE_STD && RAD_USER_MODE
class StdAllocator
{
public:

    static constexpr bool PropagateOnCopy = false;
    static constexpr bool PropagateOnMoveAssignment = false;
    static constexpr bool PropagateOnSwap = false;
    static constexpr bool IsAlwaysEqual = true;

    static constexpr bool HasTypedAllocations = true;
    static constexpr bool HasConstructAndDestroy = true;

    constexpr bool operator==(const StdAllocator& /*rhs*/) const noexcept
    {
        return true;
    }

    constexpr bool operator!=(const StdAllocator& /*rhs*/) const noexcept
    {
        return false;
    }

    // Allocate and free are intentionally not constexpr.  Constexpr evaluation
    // requires typed parameters.
    static void* AllocBytes(size_t size)
    {
        std::allocator<char> al;
        return al.allocate(size);
    }

    static void FreeBytes(void* ptr, size_t size) noexcept
    {
        std::allocator<char> al;
        return al.deallocate(static_cast<char*>(ptr), size);
    }

    template <typename T>
    static constexpr T* Alloc(size_t count)
    {
        std::allocator<T> al;
        return al.allocate(count);
    }

    template <typename T>
    static constexpr void Free(T* ptr, size_t count) noexcept
    {
        std::allocator<T> al;
        return al.deallocate(ptr, count);
    }

    template <class T, class... Args>
    static constexpr T* Construct(T* p, Args&&... args)
    {
        // use allocator_traits, because std::allocator::construct was removed
        // in C++20, and construct_at wasn't added until C++20.
        std::allocator<T> al;
        std::allocator_traits<std::allocator<T>>::construct(
            al,
            p,
            Forward<Args>(args)...);
        return p;
    }

    template <class T>
    static constexpr void Destroy(T* p) noexcept
    {
        // use allocator_traits, because std::allocator::destroy was removed in
        // C++20, and destroy_at wasn't added until C++17
        std::allocator<T> al;
        std::allocator_traits<std::allocator<T>>::destroy(al, p);
    }
};
#endif // RAD_ENABLE_STD && RAD_USER_MODE ^^^

} // namespace rad
