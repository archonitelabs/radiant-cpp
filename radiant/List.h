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
#include "radiant/EmptyOptimizedPair.h"
#include "radiant/Iterator.h"
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

    The allocator returns nullptr on failure, rather than throws an exception.
    That error is propogated.  Allocators are always propagated. "Fancy
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
template <typename T, typename TAllocator>
class List
{
public:

    // types
    using ValueType = T;
    using AllocatorType = TAllocator;
    using Pointer = T*;
    using ConstPointer = const T*;
    using Reference = ValueType&;
    using ConstReference = const ValueType&;
    using SizeType = size_t;
    using DifferenceType = ptrdiff_t;
    using Iterator = ::rad::detail::ListIterator<T>;
    using ConstIterator = ::rad::detail::ListConstIterator<T>;
    using ReverseIterator = ::rad::ReverseIterator<Iterator>;
    using ConstReverseIterator = ::rad::ReverseIterator<ConstIterator>;

    using Rebound =
        typename TAllocator::template Rebind<::rad::detail::ListNode<T>>::Other;

    ~List()
    {
        Clear();
    }

    List() = default;
    RAD_NOT_COPYABLE(List);

    List(List&& x) noexcept
        : m_storage(static_cast<AllocatorType&&>(x.m_storage.First()))
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
            .OnOk(static_cast<List&&>(local));
    }

    AllocatorType GetAllocator() const noexcept
    {
        return m_storage.First();
    }

    // iterators
    RAD_NODISCARD Iterator begin() noexcept
    {
        return Iterator(m_storage.Second().m_head.m_next);
    }

    RAD_NODISCARD ConstIterator begin() const noexcept
    {
        return ConstIterator(m_storage.Second().m_head.m_next);
    }

    RAD_NODISCARD Iterator end() noexcept
    {
        return Iterator(&m_storage.Second().m_head);
    }

    RAD_NODISCARD ConstIterator end() const noexcept
    {
        return ConstIterator(&m_storage.Second().m_head);
    }

    RAD_NODISCARD ConstIterator cbegin() const noexcept
    {
        return ConstIterator(m_storage.Second().m_head.m_next);
    }

    RAD_NODISCARD ConstIterator cend() const noexcept
    {
        return ConstIterator(&m_storage.Second().m_head);
    }

    RAD_NODISCARD ReverseIterator rbegin() noexcept
    {
        return ReverseIterator(end());
    }

    RAD_NODISCARD ReverseIterator rend() noexcept
    {
        return ReverseIterator(begin());
    }

    RAD_NODISCARD ConstReverseIterator crbegin() noexcept
    {
        return ConstReverseIterator(cend());
    }

    RAD_NODISCARD ConstReverseIterator crend() noexcept
    {
        return ConstReverseIterator(cbegin());
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
    ::rad::Err AssignSome(InputIterator first, InputIterator last)
    {
        return AssignSomeImpl(first, last);
    }

    template <typename InputRange>
    ::rad::Err AssignRange(InputRange&& rg)
    {
        return AssignSomeImpl(rg.begin(), rg.end());
    }

    ::rad::Err AssignCount(SizeType n, const T& t)
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
        return EmptyOkType{};
    }

#if RAD_ENABLE_STD
    ::rad::Err AssignInitializerList(std::initializer_list<T> il)
    {
        return AssignSomeImpl(il.begin(), il.end());
    }
#endif

    // Error handling needs work on these.  Deal with this later.
    // Maybe by having Res support reference types
    // Reference      front();
    // ConstReference front() const;
    // Reference      back();
    // ConstReference back() const;

    template <class... Args>
    ::rad::Err EmplaceFront(Args&&... args)
    {
        return ::rad::detail::ToErr(
            EmplacePtr(cbegin(), static_cast<Args&&>(args)...));
    }

    template <class... Args>
    ::rad::Err EmplaceBack(Args&&... args)
    {
        return ::rad::detail::ToErr(
            EmplacePtr(cend(), static_cast<Args&&>(args)...));
    }

    ::rad::Err PushFront(const T& x)
    {
        return ::rad::detail::ToErr(EmplacePtr(cbegin(), x));
    }

    ::rad::Err PushFront(T&& x)
    {
        return ::rad::detail::ToErr(EmplacePtr(cbegin(), static_cast<T&&>(x)));
    }

    ::rad::Err PushBack(const T& x)
    {
        return ::rad::detail::ToErr(EmplacePtr(cend(), x));
    }

    ::rad::Err PushBack(T&& x)
    {
        return ::rad::detail::ToErr(EmplacePtr(cend(), static_cast<T&&>(x)));
    }

    template <typename InputRange>
    ::rad::Err PrependRange(InputRange&& rg)
    {
        return ::rad::detail::ToErr(
            InsertSomeImpl(cbegin(), rg.begin(), rg.end()));
    }

    template <typename InputRange>
    ::rad::Err AppendRange(InputRange&& rg)
    {
        return ::rad::detail::ToErr(
            InsertSomeImpl(cend(), rg.begin(), rg.end()));
    }

    // Calling PopFront or PopBack while the container is empty is erroneous
    // behavior. It's wrong to do it, and we can diagnose it in debug mode, but
    // in release mode we will instead do nothing.
    void PopFront()
    {
        EraseOne(begin());
    }

    void PopBack()
    {
        EraseOne(--end());
    }

    // The Insert and Emplace functions provide the strong error
    // guarantee.  If they fail, then the function returns without
    // changing the container, invalidating iterators, or invalidating
    // references.
    template <class... Args>
    RAD_NODISCARD Res<Iterator> Emplace(ConstIterator position, Args&&... args)
    {
        return ToRes(EmplacePtr(position, static_cast<Args&&>(args)...));
    }

    RAD_NODISCARD Res<Iterator> Insert(ConstIterator position, const T& x)
    {
        return ToRes(EmplacePtr(position, x));
    }

    RAD_NODISCARD Res<Iterator> Insert(ConstIterator position, T&& x)
    {
        return ToRes(EmplacePtr(position, static_cast<T&&>(x)));
    }

    RAD_NODISCARD Res<Iterator> InsertCount(ConstIterator position,
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
        return Iterator{ new_pos };
    }

    template <class InputIterator>
    RAD_NODISCARD Res<Iterator> InsertSome(ConstIterator position,
                                           InputIterator first,
                                           InputIterator last)

    {
        return ToRes(InsertSomeImpl(position, first, last));
    }

    template <typename InputRange>
    RAD_NODISCARD Res<Iterator> InsertRange(ConstIterator position,
                                            InputRange&& rg)
    {
        return ToRes(InsertSomeImpl(position, rg.begin(), rg.end()));
    }

#if RAD_ENABLE_STD
    RAD_NODISCARD Res<Iterator> InsertInitializerList(
        ConstIterator position, std::initializer_list<T> il)
    {
        return ToRes(InsertSomeImpl(position, il.begin(), il.end()));
    }
#endif

    void Clear() noexcept
    {
        EraseSome(begin(), end());
    }

    Iterator EraseOne(ConstIterator position)
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

        return Iterator(retnode);
    }

    Iterator EraseSome(ConstIterator position, ConstIterator last)
    {
        ::rad::detail::ListBasicNode* cur = position.m_node;
        ::rad::detail::ListBasicNode* end = last.m_node;
        if (cur == end)
        {
            return Iterator(end);
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
        return Iterator(end);
    }

    SizeType EraseValue(const T& value)
    {
        SizeType count = 0;
        Iterator i = begin();
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
        Iterator i = begin();
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

    void Swap(List& x) noexcept
    {
        {
            TAllocator temp = static_cast<TAllocator&&>(m_storage.First());
            m_storage.First() = static_cast<TAllocator&&>(x.m_storage.First());
            x.m_storage.First() = static_cast<TAllocator&&>(temp);
        }
        m_storage.Second().Swap(x.m_storage.Second());
    }

    // The list parameter to the splice functions is mostly unused.  It's
    // important to keep it though as a way to attest that you have mutable
    // access to the source list.  If we want to support unequal allocators,
    // then we'll need access to the source list.  We'll also need to add an
    // error channel if we support unequal allocators.
    void SpliceAll(ConstIterator position, List& x)
    {
        m_storage.Second().SpliceSome(position.m_node,
                                      x.begin().m_node,
                                      x.end().m_node);
    }

    void SpliceAll(ConstIterator position, List&& x)
    {
        m_storage.Second().SpliceSome(position.m_node,
                                      x.begin().m_node,
                                      x.end().m_node);
    }

    void SpliceOne(ConstIterator position, List& x, ConstIterator i)
    {
        RAD_UNUSED(x);
        m_storage.Second().SpliceOne(position.m_node, i.m_node);
    }

    void SpliceOne(ConstIterator position, List&& x, ConstIterator i)
    {
        RAD_UNUSED(x);
        m_storage.Second().SpliceOne(position.m_node, i.m_node);
    }

    void SpliceSome(ConstIterator position,
                    List& x,
                    ConstIterator first,
                    ConstIterator last)
    {
        RAD_UNUSED(x);
        m_storage.Second().SpliceSome(position.m_node,
                                      first.m_node,
                                      last.m_node);
    }

    void SpliceSome(ConstIterator position,
                    List&& x,
                    ConstIterator first,
                    ConstIterator last)
    {
        RAD_UNUSED(x);
        m_storage.Second().SpliceSome(position.m_node,
                                      first.m_node,
                                      last.m_node);
    }

    void Reverse() noexcept
    {
        m_storage.Second().Reverse();
    }

private:

    Res<Iterator> ToRes(::rad::detail::ListBasicNode* ptr)
    {
        if (ptr == nullptr)
        {
            return Error::NoMemory;
        }
        return Iterator(ptr);
    }

    template <class... Args>
    RAD_NODISCARD ::rad::detail::ListNode<T>* EmplacePtr(ConstIterator position,
                                                         Args&&... args)
    {
        ::rad::detail::ListNode<T>* storage = ReboundAlloc().Alloc(1);
        if (storage == nullptr)
        {
            return nullptr;
        }
        // forward to placement new
        ::rad::detail::ListNode<T>* new_node = new (
            storage)::rad::detail::ListNode<T>(static_cast<Args&&>(args)...);

        // insert the new node before passed in position
        m_storage.Second().AttachNewNode(position.m_node, new_node);

        return new_node;
    }

    template <typename InputIter1, typename InputIter2>
    RAD_NODISCARD ::rad::detail::ListBasicNode* InsertSomeImpl(
        ConstIterator position, InputIter1 first, InputIter2 last)
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
    ::rad::Err AssignSomeImpl(InputIter1 first, InputIter2 last)
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
        return EmptyOkType{};
    }

    Rebound ReboundAlloc()
    {
        return m_storage.First();
    }

    ::rad::EmptyOptimizedPair<TAllocator, ::rad::detail::ListUntyped> m_storage;
};

} // namespace rad
