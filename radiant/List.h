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
#include "radiant/Memory.h"
#include "radiant/EmptyOptimizedPair.h"
#include "radiant/Res.h"

#if RAD_ENABLE_STD
#include <iterator>         // TODO
#include <initializer_list> // TODO
#endif                      // RAD_ENABLE_STD

namespace rad
{

template <typename T, typename TAllocator RAD_ALLOCATOR_EQ(T)>
class List;

namespace detail
{
template <typename T>
class ListConstIterator;

class ListBasicNode
{
public:

    ListBasicNode() = default;
    ~ListBasicNode() = default;

    // immovable
    ListBasicNode(const ListBasicNode&) = delete;
    ListBasicNode(ListBasicNode&&) = delete;
    ListBasicNode& operator=(const ListBasicNode&) = delete;
    ListBasicNode& operator=(ListBasicNode&&) = delete;

    void Unlink()
    {
        m_prev = m_next = this;
    }

    void FixupHeadSwap(_In_ const ListBasicNode& old_head) noexcept
    {
        if (m_next == &old_head)
        {
            // This used to be an empty head, but now it
            // looks like the old head is the first element
            // in our list.  unlink to reestablish emptiness.
            Unlink();
        }
        else
        {
            // TODO: turn the asserts into fast fails.
            // if they fire, it indicates a heap overflow,
            // and someone is trying to build a write primitive
            RAD_ASSERT(m_next->m_prev == &old_head);
            RAD_ASSERT(m_prev->m_next == &old_head);
            m_next->m_prev = this;
            m_prev->m_next = this;
        }
    }

    void CheckSanityBeforeRelinking() const noexcept
    {
        // TODO: turn the asserts into fast fails.
        // if they fire, it indicates a heap overflow,
        // and someone is trying to build a write primitive
        RAD_ASSERT(m_next->m_prev == this);
        RAD_ASSERT(m_prev->m_next == this);
    }

    void Swap(_Inout_ ListBasicNode& x) noexcept
    {
        ListBasicNode* temp = m_next;
        m_next = x.m_next;
        x.m_next = temp;

        temp = m_prev;
        m_prev = x.m_prev;
        x.m_prev = temp;
    }

    void AssertOnEmpty() const noexcept
    {
        // TODO: turn the assert into fast fail.
        RAD_ASSERT(m_next != this);
    }

    ListBasicNode* m_next = this;
    ListBasicNode* m_prev = this;
};

template <typename T>
class ListNode : public ListBasicNode
{
public:

    ListNode() = default;
    ~ListNode() = default;

    // used by Emplace
    template <class... Args>
    explicit ListNode(Args&&... args)
        : ListBasicNode(),
          m_elt(static_cast<Args&&>(args)...)
    {
    }

    // immovable
    ListNode(const ListNode&) = delete;
    ListNode(ListNode&&) = delete;
    ListNode& operator=(const ListNode&) = delete;
    ListNode& operator=(ListNode&&) = delete;

    T m_elt;
};

template <typename T>
class ListIterator
{
public:

#if RAD_ENABLE_STD
    using iterator_category = std::bidirectional_iterator_tag;
#endif

    using ValueType = T;
    using DifferenceType = ptrdiff_t;
    using Pointer = T*;
    using Reference = T&;

    ListIterator() = default;

    explicit ListIterator(ListBasicNode* node)
        : m_node(node)
    {
    }

    ListIterator(const ListIterator&) = default;
    ListIterator(ListIterator&&) = default;
    ListIterator& operator=(const ListIterator&) = default;
    ListIterator& operator=(ListIterator&&) = default;
    ~ListIterator() = default;

    bool operator==(ListIterator rhs) const noexcept
    {
        return m_node == rhs.m_node;
    }

    bool operator!=(ListIterator rhs) const noexcept
    {
        return m_node != rhs.m_node;
    }

    Reference operator*() const noexcept
    {
        m_node->AssertOnEmpty();
        return static_cast<ListNode<T>*>(m_node)->m_elt;
    }

    Pointer operator->() const noexcept
    {
        m_node->AssertOnEmpty();
        return &static_cast<ListNode<T>*>(m_node)->m_elt;
    }

    ListIterator& operator++() noexcept
    {
        m_node = m_node->m_next;
        return *this;
    }

    ListIterator operator++(int) noexcept
    {
        ListIterator retval(m_node);
        m_node = m_node->m_next;
        return retval;
    }

    ListIterator& operator--() noexcept
    {
        m_node = m_node->m_prev;
        return *this;
    }

    ListIterator operator--(int) noexcept
    {
        ListIterator retval(m_node);
        m_node = m_node->m_prev;
        return *this;
    }

private:

    ListBasicNode* m_node = nullptr;

    template <typename U, typename TAllocator>
    friend class List;

    friend class ListConstIterator<T>;
};

template <typename T>
class ListConstIterator
{
public:

#if RAD_ENABLE_STD
    using iterator_category = std::bidirectional_iterator_tag;
#endif

    using ValueType = const T;
    using DifferenceType = ptrdiff_t;
    using Pointer = const T*;
    using Reference = const T&;

    ListConstIterator() = default;

    explicit ListConstIterator(const ListBasicNode* node)
        : m_node(node)
    {
    }

    ListConstIterator(const ListConstIterator&) = default;
    ListConstIterator(ListConstIterator&&) = default;

    ListConstIterator& operator=(const ListConstIterator&) = default;
    ListConstIterator& operator=(ListConstIterator&&) = default;

    ~ListConstIterator() = default;

    /* implicit */ ListConstIterator(ListIterator<T> other) noexcept
        : m_node(other.m_node)
    {
    }

    bool operator==(ListConstIterator rhs) const noexcept
    {
        return m_node == rhs.m_node;
    }

    bool operator!=(ListConstIterator rhs) const noexcept
    {
        return m_node != rhs.m_node;
    }

    Reference operator*() const noexcept
    {
        m_node->AssertOnEmpty();
        return static_cast<const ListNode<T>*>(m_node)->m_elt;
    }

    Pointer operator->() const noexcept
    {
        m_node->AssertOnEmpty();
        return &static_cast<const ListNode<T>*>(m_node)->m_elt;
    }

    ListConstIterator& operator++() noexcept
    {
        m_node = m_node->m_next;
        return *this;
    }

    ListConstIterator operator++(int) noexcept
    {
        ListConstIterator retval(m_node);
        m_node = m_node->m_next;
        return retval;
    }

    ListConstIterator& operator--() noexcept
    {
        m_node = m_node->m_prev;
        return *this;
    }

    ListConstIterator operator--(int) noexcept
    {
        ListConstIterator retval(m_node);
        m_node = m_node->m_prev;
        return *this;
    }

private:

    const ListBasicNode* m_node = nullptr;

    template <typename U, typename TAllocator>
    friend class List;
};

class ListUntyped
{
public:

    ListUntyped() = default;

    ListUntyped(_In_ const ListUntyped& x) = delete;

    ListUntyped(_Inout_ ListUntyped&& x) noexcept = delete;

    ~ListUntyped() = default;
    ListUntyped& operator=(_In_ const ListUntyped& x) = delete;
    ListUntyped& operator=(_Inout_ ListUntyped&& x) noexcept = delete;

    // [list.capacity], capacity
    RAD_NODISCARD bool Empty() const noexcept
    {
        return m_head.m_next == &m_head;
    }

    // O(N) operation, renamed so that people don't
    // assume it is cheap and make things accidentally
    // quadratic.  Too useful in test code to omit
    // entirely.
    RAD_NODISCARD size_t ExpensiveSize() const noexcept
    {
        size_t count = 0;
        const ListBasicNode* cur = &m_head;
        while (cur->m_next != &m_head)
        {
            cur = cur->m_next;
            ++count;
        }
        return count;
    }

    void Swap(_Inout_ ListUntyped& x) noexcept
    {
        m_head.Swap(x.m_head);
        m_head.FixupHeadSwap(x.m_head);
        x.m_head.FixupHeadSwap(m_head);
    }

    void AttachNewNode(_Inout_ ListBasicNode* pos, _Inout_ ListBasicNode* i)
    {
        RAD_ASSERT(i->m_next == i);
        RAD_ASSERT(i->m_prev == i);
        i->m_next = pos;
        i->m_prev = pos->m_prev;

        pos->CheckSanityBeforeRelinking();
        pos->m_prev->m_next = i;
        pos->m_prev = i;
    }

    void SpliceOne(_Inout_ ListBasicNode* pos, _Inout_ ListBasicNode* i)
    {
        i->CheckSanityBeforeRelinking();
        i->m_next->m_prev = i->m_prev;
        i->m_prev->m_next = i->m_next;

        i->m_next = pos;
        i->m_prev = pos->m_prev;

        pos->CheckSanityBeforeRelinking();
        pos->m_prev->m_next = i;
        pos->m_prev = i;
    }

    void SpliceSome(_Inout_ ListBasicNode* position,
                    _Inout_ ListBasicNode* first,
                    _Inout_ ListBasicNode* last);

    void Reverse() noexcept;

    ListBasicNode m_head;
};
} // namespace detail

/*!
    @brief Cross platform (and kernel appropriate) doubly linked list
    implementation

    @details Deviations from std::list:

    The container isn't copiable using the copy constructor or copy assignment.
    This is because there is no way to indicate failure in these cases.  Move
    construction and move assignment work fine, and are guaranteed noexcept. Use
    AssignRange if you need a deep copy.

    The set of constructors has been greatly reduced, due to error handling
    difficulties when exceptions aren't available.

    The allocator returns nullptr on failure, rather than throws an exception.
    That error is propogated.  Allocators are always propagated. "Fancy
    pointers" aren't supported, as the allocators used aren't std allocators. So
    you won't be able to use some offset based pointer to do shared memory
    things with this container.

    Reverse iterators aren't currently supported, though there isn't any reason
    that they couldn't be supported.  The return on investment isn't currently
    there. Same story for unique, merge, and sort. TODO: reverse iterators are
    cheap in radiant.

    Removed size() related functions, as making size() O(1) interferes with
    efficient splicing, and having size() be O(N) is too much of a foot gun. The
    standard made a mistake here.  Empty() is still present and fine, and
    ExpensiveSize() exists largely for test code.

    The Assign functions now all return Res to signal errors.  The count
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

    TODO: implementation, doxygen, formatting, use Res / Result instead of
    NTSTATUS

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

    using Rebound =
        typename TAllocator::template Rebind<::rad::detail::ListNode<T>>::Other;

    List() = default;

    explicit List(_In_ const TAllocator& alloc)
        : m_storage(alloc)
    {
    }

    List(_In_ const List& x) = delete;

    List(_Inout_ List&& x) noexcept
    {
        Swap(x);
    }

    ~List()
    {
        Clear();
    }

    List& operator=(_In_ const List& x) = delete;

    List& operator=(_Inout_ List&& x) noexcept
    {
        Clear();
        Swap(x);
        return *this;
    }

    template <class InputIterator>
    Err AssignSome(InputIterator first, InputIterator last)
    {
        List local(m_storage.First());
        for (; first != last; ++first)
        {
            Err res = local.EmplaceBack(*first);
            if (res.IsErr())
            {
                return res;
            }
        }
        // using m_list Swap so that we don't need to swap allocators
        m_storage.Second().Swap(local.m_storage.Second());
        return EmptyOkType{};
    }

    template <typename InputRange>
    Err AssignRange(InputRange&& rg)
    {
        return AssignSome(rg.begin(), rg.end());
    }

    Err AssignCount(SizeType n, _In_ const T& t)
    {
        List local(m_storage.First());
        for (SizeType i = 0; i < n; ++i)
        {
            Err res = local.EmplaceBack(t);
            if (res.IsErr())
            {
                return res;
            }
        }
        // using m_list Swap so that we don't need to swap allocators
        m_storage.Second().Swap(local.m_storage.Second());
        return EmptyOkType{};
    }

#if RAD_ENABLE_STD
    Err AssignInitializerList(std::initializer_list<T> il)
    {
        return AssignSome(il.begin(), il.end());
    }
#endif

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

    // [list.capacity], capacity
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

    // element access: TODO error handling on these
    // Alternative, force users to do this themselves.
    // But *my_list.begin() is no safer, and *(--m_list.end()) is annoying and
    // unsafe. Heavily encourage people to do `for (auto &elt : list)
    // {stuff(elt); break;}` That's also gross, and doesn't work well for back()
    // without reverse iterators Return an optional or an expected?  Well, how
    // do those handle dereferencing? my_list.front().value() is still unsafe.
    // pattern matching to save the day, one day?
    // monadic / visitation APIs?
    // Deal with this later.  Maybe by having Res support reference types
    // Reference      front();
    // ConstReference front() const;
    // Reference      back();
    // ConstReference back() const;

    // [list.modifiers], modifiers
    template <class... Args>
    Err EmplaceFront(Args&&... args);

    template <class... Args>
    Err EmplaceBack(Args&&... args)
    {
        ::rad::detail::ListNode<T>* storage = ReboundAlloc().Alloc(1);
        if (storage == nullptr)
        {
            return Error::NoMemory;
        }
        // forward to placement new
        ::rad::detail::ListNode<T>* new_node = new (
            storage)::rad::detail::ListNode<T>(static_cast<Args&&>(args)...);
        // attach the new node before the end node.
        m_storage.Second().AttachNewNode(&m_storage.Second().m_head, new_node);

        return EmptyOkType{};
    }

    Err PushFront(_In_ const T& x);
    Err PushFront(_Inout_ T&& x);

    template <typename InputRange>
    Err PrependRange(InputRange&& rg);

    // Calling PopFront while the container is empty is erroneous behavior.
    // It's wrong to do it, and we can diagnose it in debug mode, but in
    // release mode we will instead do nothing.
    void PopFront();

    Err PushBack(_In_ const T& x)
    {
        return EmplaceBack(x);
    }

    Err PushBack(_Inout_ T&& x)
    {
        return EmplaceBack(static_cast<T&&>(x));
    }

    template <typename InputRange>
    Err AppendRange(InputRange&& rg);

    // Calling PopBack while the container is empty is erroneous behavior.
    // It's wrong to do it, and we can diagnose it in debug mode, but in
    // release mode we will instead do nothing.
    void PopBack();

    // Insert and Emplace functions return the end iterator on failure.
    // The Insert and Emplace functions provide the strong error
    // guarantee.  If they fail, then the function returns without
    // changing the container, invalidating iterators, or invalidating
    // references.
    template <class... Args>
    RAD_NODISCARD Iterator Emplace(ConstIterator position, Args&&... args)
    {
        ::rad::detail::ListNode<T>* storage = ReboundAlloc().Alloc(1);
        if (storage == nullptr)
        {
            return Iterator(&m_storage.Second().m_head);
        }
        // forward to placement new
        ::rad::detail::ListNode<T>* new_node = new (
            storage)::rad::detail::ListNode<T>(static_cast<Args&&>(args)...);
        // insert the new node before the end node.
        m_storage.Second().AttachNewNode(&position.m_node, new_node);

        return Iterator(new_node);
    }

    RAD_NODISCARD Iterator Insert(ConstIterator position, _In_ const T& x);
    RAD_NODISCARD Iterator Insert(ConstIterator position, _Inout_ T&& x);

    RAD_NODISCARD Iterator InsertCount(ConstIterator position,
                                       SizeType n,
                                       _In_ const T& x);

    template <class InputIterator>
    RAD_NODISCARD Iterator InsertSome(ConstIterator position,
                                      InputIterator first,
                                      InputIterator last);

    template <typename InputRange>
    RAD_NODISCARD Iterator InsertRange(ConstIterator position, InputRange&& rg);

#if RAD_ENABLE_STD
    RAD_NODISCARD Iterator InsertInitializerList(ConstIterator position,
                                                 std::initializer_list<T> il);
#endif

    Iterator EraseOne(ConstIterator position);
    Iterator EraseSome(ConstIterator position, ConstIterator last);

    SizeType EraseValue(const T& value);
    template <typename Predicate>
    SizeType EraseIf(Predicate pred);

    void Swap(_Inout_ List& x) noexcept
    {
        {
            TAllocator temp = m_storage.First();
            m_storage.First() = x.m_storage.First();
            x.m_storage.First() = temp;
        }
        m_storage.Second().Swap(x.m_storage.Second());
    }

    void Clear() noexcept
    {
        ::rad::detail::ListBasicNode* cur = m_storage.Second().m_head.m_next;
        while (cur != &m_storage.Second().m_head)
        {
            ::rad::detail::ListNode<T>* typed =
                static_cast<::rad::detail::ListNode<T>*>(cur);
            cur = cur->m_next; // TODO suppress C6001 uninit memory warning?

            typed->~ListNode();
            ReboundAlloc().Free(typed);
        }
        m_storage.Second().m_head.Unlink();
    }

    // [list.ops], list operations
    void SpliceAll(ConstIterator position, _Inout_ List& x);
    void SpliceAll(ConstIterator position, _Inout_ List&& x);

    void SpliceOne(ConstIterator position, _Inout_ List& x, ConstIterator i);
    void SpliceOne(ConstIterator position, _Inout_ List&& x, ConstIterator i);

    void SpliceSome(ConstIterator position,
                    _Inout_ List& x,
                    ConstIterator first,
                    ConstIterator last);
    void SpliceSome(ConstIterator position,
                    _Inout_ List&& x,
                    ConstIterator first,
                    ConstIterator last);

    void Reverse() noexcept;

private:

    Rebound ReboundAlloc()
    {
        return m_storage.First();
    }

    EmptyOptimizedPair<TAllocator, ::rad::detail::ListUntyped> m_storage;
};

} // namespace rad
