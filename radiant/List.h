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

#pragma once

#include "radiant/TotallyRad.h"
// rad::Swap needs radiant/Utility.h
#include "radiant/Algorithm.h" // NOLINT(misc-include-cleaner)
#include "radiant/EmptyOptimizedPair.h"
#include "radiant/Iterator.h"
#include "radiant/Memory.h"
#include "radiant/Res.h"
#include "radiant/detail/ListOperations.h"

#include <stddef.h>

#if RAD_ENABLE_STD
#include <initializer_list>
#endif // RAD_ENABLE_STD

namespace rad
{

/*!
    @brief Cross platform (and kernel appropriate) doubly linked list
    implementation

    @details Deviations from std::list:

    The container isn't copiable using the copy constructor or copy assignment.
    This is because there is no way to indicate failure in these cases.  Move
    construction and move assignment work fine, and are guaranteed noexcept. Use
    Clone if you need a deep copy.

    The set of constructors has been greatly reduced, due to error handling
    difficulties when exceptions aren't available.

    The allocator returns nullptr on failure, rather than throwing an exception.
    That error is propagated.  Allocators are always propagated. "Fancy
    pointers" aren't supported, as the allocators used aren't std allocators. So
    you won't be able to use some offset based pointer to do shared memory
    things with this container.

    Removed size() related functions, as making size() O(1) interferes with
    efficient splicing, and having size() be O(N) is too much of a foot gun. The
    standard made a mistake here.  Empty() is still present and fine, and
    ExpensiveSize() exists largely for test code.

    The Assign functions now all return Err to signal errors.  The count
    and initializer_list overloads of Assign were renamed to AssignCount and
    AssignInitializerList to minimize overloading

    Renamed remove and remove_if to EraseValue and EraseIf, as the "remove"
    names are mistakes in the standard.  "remove" usually refers to algorithms
    that move unwanted elements out of the way, but don't change the size of the
    container. "erase" will get rid of unwanted elements and change the
    container size.

    Renamed iterator overloads of "erase" to "EraseOne" and "EraseSome" to
    better capture caller intent.  There's a common bug where devs call
    `erase(range_begin)` when they meant to call `erase(range_begin,
    range_end)`, and splitting the "erase" overload set into two names
    eliminates that bug.

    No Constructor Template Argument Deduction(CTAD), because that all requires
    constructors that will do some initial population of the container, and we
    don't have those constructors since we can't signal errors out of
    constructors.

    The splice family of functions were renamed to SpliceAll, SpliceOne, and
    SpliceSome, to better make their intent clear to readers without needing to
    carefully count the number of parameters.

    @tparam T - Value type held by the list
    @tparam TAllocator - Allocator to use.
*/
template <typename T, typename TAllocator RAD_ALLOCATOR_EQ(T)>
class List
{
public:

    // types
    using ValueType = T;
    using AllocatorType = TAllocator;
    using PointerType = T*;
    using ConstPointerType = const T*;
    using ReferenceType = ValueType&;
    using ConstReferenceType = const ValueType&;
    using SizeType = size_t;
    using DifferenceType = ptrdiff_t;
    using IteratorType = ::rad::detail::ListIterator<T>;
    using ConstIteratorType = ::rad::detail::ListConstIterator<T>;
    using ReverseIteratorType = ReverseIterator<IteratorType>;
    using ConstReverseIteratorType = ReverseIterator<ConstIteratorType>;

    using Rebound =
        typename TAllocator::template Rebind<::rad::detail::ListNode<T>>::Other;

    // Not asserting noexcept movability on T, as we mostly don't need to move
    // types once they are constructed.  We support immovable types like
    // mutexes.  The Take* functions assert on noexcept movability since they
    // do move contained elements.
    RAD_S_ASSERT_ALLOCATOR_REQUIRES_T(TAllocator);

    ~List()
    {
        Clear();
    }

    List() = default;
    RAD_NOT_COPYABLE(List);

    List(List&& x) noexcept
        : m_storage(::rad::Move(x.m_storage.First()))
    {
        m_storage.Second().Swap(x.m_storage.Second());
    }

    List& operator=(List&& x) noexcept
    {
        Clear();
        Swap(x);
        return *this;
    }

    explicit List(const TAllocator& alloc)
        : m_storage(alloc)
    {
    }

    Res<List> Clone()
    {
        List local(m_storage.First());
        return local.AssignSomeImpl(this->begin(), this->end())
            .OnOk(::rad::Move(local));
    }

    AllocatorType GetAllocator() const noexcept
    {
        return m_storage.First();
    }

    // iterators
    RAD_NODISCARD IteratorType begin() noexcept
    {
        return IteratorType(m_storage.Second().m_head.m_next);
    }

    RAD_NODISCARD ConstIteratorType begin() const noexcept
    {
        return ConstIteratorType(m_storage.Second().m_head.m_next);
    }

    RAD_NODISCARD IteratorType end() noexcept
    {
        return IteratorType(&m_storage.Second().m_head);
    }

    RAD_NODISCARD ConstIteratorType end() const noexcept
    {
        return ConstIteratorType(&m_storage.Second().m_head);
    }

    RAD_NODISCARD ConstIteratorType cbegin() const noexcept
    {
        return ConstIteratorType(m_storage.Second().m_head.m_next);
    }

    RAD_NODISCARD ConstIteratorType cend() const noexcept
    {
        return ConstIteratorType(&m_storage.Second().m_head);
    }

    RAD_NODISCARD ReverseIteratorType rbegin() noexcept
    {
        return ReverseIteratorType(end());
    }

    RAD_NODISCARD ReverseIteratorType rend() noexcept
    {
        return ReverseIteratorType(begin());
    }

    RAD_NODISCARD ConstReverseIteratorType crbegin() noexcept
    {
        return ConstReverseIteratorType(cend());
    }

    RAD_NODISCARD ConstReverseIteratorType crend() noexcept
    {
        return ConstReverseIteratorType(cbegin());
    }

    RAD_NODISCARD bool Empty() const noexcept
    {
        // debug perf optimization:  Duplicate "Empty"
        // implementation, rather than forward the call down
        // three levels.
        return m_storage.Second().m_head.m_next == &m_storage.Second().m_head;
    }

    // O(N) operation, renamed so that people don't
    // assume it is cheap and make things accidentally
    // quadratic.  Too useful in test code to omit
    // entirely.
    RAD_NODISCARD size_t ExpensiveSize() const noexcept
    {
        return m_storage.Second().ExpensiveSize();
    }

    template <class InputIterator>
    Res<List&> AssignSome(InputIterator first, InputIterator last)
    {
        return AssignSomeImpl(first, last);
    }

    template <typename InputRange>
    Res<List&> AssignRange(InputRange&& rg)
    {
        return AssignSomeImpl(rg.begin(), rg.end());
    }

    Res<List&> AssignCount(SizeType n, const T& t)
    {
        List local(m_storage.First());
        auto end_iter = local.cend();
        for (SizeType i = 0; i < n; ++i)
        {
            void* ptr = local.EmplacePtr(end_iter, t);
            if (ptr == nullptr)
            {
                return Error::NoMemory;
            }
        }
        // using m_list Swap so that we don't need to swap allocators
        m_storage.Second().Swap(local.m_storage.Second());
        return *this;
    }

#if RAD_ENABLE_STD
    Res<List&> AssignInitializerList(std::initializer_list<T> il)
    {
        return AssignSomeImpl(il.begin(), il.end());
    }
#endif

    Res<ReferenceType> Front()
    {
        if (Empty())
        {
            return Res<ReferenceType>(ResErrTag, Error::OutOfRange);
        }
        return Res<ReferenceType>(ResOkTag, *begin());
    }

    Res<ConstReferenceType> Front() const
    {
        if (Empty())
        {
            return Res<ConstReferenceType>(ResErrTag, Error::OutOfRange);
        }
        return Res<ConstReferenceType>(ResOkTag, *begin());
    }

    Res<ReferenceType> Back()
    {
        if (Empty())
        {
            return Res<ReferenceType>(ResErrTag, Error::OutOfRange);
        }
        return Res<ReferenceType>(ResOkTag, *(--end()));
    }

    Res<ConstReferenceType> Back() const
    {
        if (Empty())
        {
            return Res<ConstReferenceType>(ResErrTag, Error::OutOfRange);
        }
        return Res<ConstReferenceType>(ResOkTag, *(--end()));
    }

    template <class... Args>
    Res<List&> EmplaceFront(Args&&... args)
    {
        return ToResSelf(EmplacePtr(cbegin(), Forward<Args>(args)...));
    }

    template <class... Args>
    Res<List&> EmplaceBack(Args&&... args)
    {
        return ToResSelf(EmplacePtr(cend(), Forward<Args>(args)...));
    }

    Res<List&> PushFront(const T& x)
    {
        return ToResSelf(EmplacePtr(cbegin(), x));
    }

    Res<List&> PushFront(T&& x)
    {
        return ToResSelf(EmplacePtr(cbegin(), ::rad::Move(x)));
    }

    Res<List&> PushBack(const T& x)
    {
        return ToResSelf(EmplacePtr(cend(), x));
    }

    Res<List&> PushBack(T&& x)
    {
        return ToResSelf(EmplacePtr(cend(), ::rad::Move(x)));
    }

    template <typename InputRange>
    Res<List&> PrependRange(InputRange&& rg)
    {
        return ToResSelf(InsertSomeImpl(cbegin(), rg.begin(), rg.end()));
    }

    template <typename InputRange>
    Res<List&> AppendRange(InputRange&& rg)
    {
        return ToResSelf(InsertSomeImpl(cend(), rg.begin(), rg.end()));
    }

    // Calling PopFront or PopBack while the container is empty is erroneous
    // behavior. It's wrong to do it, and we can diagnose it in debug mode, but
    // in release mode we will instead do nothing.
    List& PopFront()
    {
        RAD_ASSERT(!Empty());
        EraseOne(begin());
        return *this;
    }

    List& PopBack()
    {
        RAD_ASSERT(!Empty());
        EraseOne(--end());
        return *this;
    }

    // move the first T into the return value and pop it from the list
    Res<T> TakeFront()
    {
        // careful!  T could be rad::Error
        RAD_S_ASSERT_NOTHROW_MOVE_T(T);
        if (Empty())
        {
            return Res<T>(ResErrTag, Error::OutOfRange);
        }
        List local(m_storage.First());
        local.m_storage.Second().SpliceOne(local.end().m_node, begin().m_node);
        return Res<T>(ResOkTag, ::rad::Move(*local.begin()));
    }

    // move the last T into the return value and pop it from the list
    Res<T> TakeBack()
    {
        // careful!  T could be rad::Error
        RAD_S_ASSERT_NOTHROW_MOVE_T(T);
        if (Empty())
        {
            return Res<T>(ResErrTag, Error::OutOfRange);
        }
        List local(m_storage.First());
        local.m_storage.Second().SpliceOne(local.end().m_node,
                                           (--end()).m_node);
        return Res<T>(ResOkTag, ::rad::Move(*local.begin()));
    }

    // The Insert and Emplace functions provide the strong error
    // guarantee.  If they fail, then the function returns without
    // changing the container, invalidating iterators, or invalidating
    // references.
    template <class... Args>
    RAD_NODISCARD Res<IteratorType> Emplace(ConstIteratorType position,
                                            Args&&... args)
    {
        return ToResIter(EmplacePtr(position, Forward<Args>(args)...));
    }

    RAD_NODISCARD Res<IteratorType> Insert(ConstIteratorType position,
                                           const T& x)
    {
        return ToResIter(EmplacePtr(position, x));
    }

    RAD_NODISCARD Res<IteratorType> Insert(ConstIteratorType position, T&& x)
    {
        return ToResIter(EmplacePtr(position, ::rad::Move(x)));
    }

    RAD_NODISCARD Res<IteratorType> InsertCount(ConstIteratorType position,
                                                SizeType n,
                                                const T& x)
    {
        List local(m_storage.First());
        auto end_iter = local.cend();
        for (SizeType i = 0; i < n; ++i)
        {
            void* ptr = local.EmplacePtr(end_iter, x);
            if (ptr == nullptr)
            {
                return Error::NoMemory;
            }
        }
        ::rad::detail::ListBasicNode* new_pos =
            m_storage.Second().SpliceSome(position.m_node,
                                          local.begin().m_node,
                                          local.end().m_node);
        return IteratorType{ new_pos };
    }

    template <class InputIterator>
    RAD_NODISCARD Res<IteratorType> InsertSome(ConstIteratorType position,
                                               InputIterator first,
                                               InputIterator last)

    {
        return ToResIter(InsertSomeImpl(position, first, last));
    }

    template <typename InputRange>
    RAD_NODISCARD Res<IteratorType> InsertRange(ConstIteratorType position,
                                                InputRange&& rg)
    {
        return ToResIter(InsertSomeImpl(position, rg.begin(), rg.end()));
    }

#if RAD_ENABLE_STD
    RAD_NODISCARD Res<IteratorType> InsertInitializerList(
        ConstIteratorType position, std::initializer_list<T> il)
    {
        return ToResIter(InsertSomeImpl(position, il.begin(), il.end()));
    }
#endif

    List& Clear() noexcept
    {
        EraseSome(begin(), end());
        return *this;
    }

    IteratorType EraseOne(ConstIteratorType position)
    {
        if (position == cend())
        {
            return end();
        }
        ::rad::detail::ListBasicNode* cur = position.m_node;
        ::rad::detail::ListBasicNode* retnode = cur->m_next;
        cur->CheckSanityBeforeRelinking();
        cur->m_prev->m_next = cur->m_next;
        cur->m_next->m_prev = cur->m_prev;

        ::rad::detail::ListNode<T>* typed =
            static_cast<::rad::detail::ListNode<T>*>(cur);
        typed->~ListNode();
        ReboundAlloc().Free(typed);

        return IteratorType(retnode);
    }

    IteratorType EraseSome(ConstIteratorType position, ConstIteratorType last)
    {
        ::rad::detail::ListBasicNode* cur = position.m_node;
        ::rad::detail::ListBasicNode* end = last.m_node;
        if (cur == end)
        {
            return IteratorType(end);
        }
        cur->CheckSanityBeforeRelinking();
        end->CheckSanityBeforeRelinking();
        cur->m_prev->m_next = end;
        end->m_prev = cur->m_prev;

        while (cur != end)
        {
            ::rad::detail::ListNode<T>* typed =
                static_cast<::rad::detail::ListNode<T>*>(cur);
            cur = cur->m_next;

            typed->~ListNode();
            ReboundAlloc().Free(typed);
        }
        return IteratorType(end);
    }

    SizeType EraseValue(const T& value)
    {
        SizeType count = 0;
        IteratorType i = begin();
        while (i != end())
        {
            if (*i == value)
            {
                i = EraseOne(i);
                ++count;
            }
            else
            {
                ++i;
            }
        }
        return count;
    }

    template <typename Predicate>
    SizeType EraseIf(Predicate pred)
    {
        SizeType count = 0;
        IteratorType i = begin();
        while (i != end())
        {
            if (pred(*i))
            {
                i = EraseOne(i);
                ++count;
            }
            else
            {
                ++i;
            }
        }
        return count;
    }

    List& Swap(List& x) noexcept
    {
        {
            rad::Swap(m_storage.First(), x.m_storage.First());
        }
        m_storage.Second().Swap(x.m_storage.Second());
        return *this;
    }

    // The list parameter to the splice functions is mostly unused.  It's
    // important to keep it though as a way to attest that you have mutable
    // access to the source list.  If we want to support unequal allocators,
    // then we'll need access to the source list.  We'll also need to add an
    // error channel if we support unequal allocators.
    List& SpliceAll(ConstIteratorType position, List& x)
    {
        m_storage.Second().SpliceSome(position.m_node,
                                      x.begin().m_node,
                                      x.end().m_node);
        return *this;
    }

    List& SpliceAll(ConstIteratorType position, List&& x)
    {
        m_storage.Second().SpliceSome(position.m_node,
                                      x.begin().m_node,
                                      x.end().m_node);
        return *this;
    }

    List& SpliceOne(ConstIteratorType position, List& x, ConstIteratorType i)
    {
        RAD_UNUSED(x);
        m_storage.Second().SpliceOne(position.m_node, i.m_node);
        return *this;
    }

    List& SpliceOne(ConstIteratorType position, List&& x, ConstIteratorType i)
    {
        RAD_UNUSED(x);
        m_storage.Second().SpliceOne(position.m_node, i.m_node);
        return *this;
    }

    List& SpliceSome(ConstIteratorType position,
                     List& x,
                     ConstIteratorType first,
                     ConstIteratorType last)
    {
        RAD_UNUSED(x);
        m_storage.Second().SpliceSome(position.m_node,
                                      first.m_node,
                                      last.m_node);
        return *this;
    }

    List& SpliceSome(ConstIteratorType position,
                     List&& x,
                     ConstIteratorType first,
                     ConstIteratorType last)
    {
        RAD_UNUSED(x);
        m_storage.Second().SpliceSome(position.m_node,
                                      first.m_node,
                                      last.m_node);
        return *this;
    }

    List& Reverse() noexcept
    {
        m_storage.Second().Reverse();
        return *this;
    }

private:

    static Res<IteratorType> ToResIter(::rad::detail::ListBasicNode* ptr)
    {
        if (ptr == nullptr)
        {
            return Error::NoMemory;
        }
        return IteratorType(ptr);
    }

    Res<List&> ToResSelf(const void* ptr)
    {
        if (ptr == nullptr)
        {
            return Error::NoMemory;
        }
        return *this;
    }

    template <class... Args>
    RAD_NODISCARD ::rad::detail::ListNode<T>* EmplacePtr(
        ConstIteratorType position, Args&&... args)
    {
        ::rad::detail::ListNode<T>* storage = ReboundAlloc().Alloc(1);
        if (storage == nullptr)
        {
            return nullptr;
        }
        // forward to placement new
        ::rad::detail::ListNode<T>* new_node =
            new (storage)::rad::detail::ListNode<T>(Forward<Args>(args)...);

        // insert the new node before passed in position
        m_storage.Second().AttachNewNode(position.m_node, new_node);

        return new_node;
    }

    template <typename InputIter1, typename InputIter2>
    RAD_NODISCARD ::rad::detail::ListBasicNode* InsertSomeImpl(
        ConstIteratorType position, InputIter1 first, InputIter2 last)
    {
        List local(m_storage.First());
        auto end_iter = local.cend();
        for (; first != last; ++first)
        {
            void* ptr = local.EmplacePtr(end_iter, *first);
            if (ptr == nullptr)
            {
                return nullptr;
            }
        }
        ::rad::detail::ListBasicNode* new_pos =
            m_storage.Second().SpliceSome(position.m_node,
                                          local.begin().m_node,
                                          local.end().m_node);
        return new_pos;
    }

    template <typename InputIter1, typename InputIter2>
    Res<List&> AssignSomeImpl(InputIter1 first, InputIter2 last)
    {
        List local(m_storage.First());
        auto end_iter = local.cend();
        for (; first != last; ++first)
        {
            void* ptr = local.EmplacePtr(end_iter, *first);
            if (ptr == nullptr)
            {
                return Error::NoMemory;
            }
        }
        // using m_list Swap so that we don't need to swap allocators
        m_storage.Second().Swap(local.m_storage.Second());
        return *this;
    }

    Rebound ReboundAlloc()
    {
        return m_storage.First();
    }

    EmptyOptimizedPair<TAllocator, ::rad::detail::ListUntyped> m_storage;
};

} // namespace rad
