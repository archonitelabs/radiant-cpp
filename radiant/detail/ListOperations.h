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

#if RAD_ENABLE_STD
#include <iterator>
#include <initializer_list>
#endif // RAD_ENABLE_STD

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
        return retval;
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
        : m_node(const_cast<ListBasicNode*>(node))
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
        return retval;
    }

private:

    ListBasicNode* m_node = nullptr;

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

    ListBasicNode* SpliceSome(_Inout_ ListBasicNode* position,
                              _Inout_ ListBasicNode* first,
                              _Inout_ ListBasicNode* last)
    {
        if (first == last)
        {
            return position;
        }
        last = last->m_prev;
        last->CheckSanityBeforeRelinking();
        first->CheckSanityBeforeRelinking();

        first->m_prev->m_next = last->m_next;
        last->m_next->m_prev = first->m_prev;

        last->m_next = position;
        first->m_prev = position->m_prev;

        position->CheckSanityBeforeRelinking();
        position->m_prev->m_next = first;
        position->m_prev = last;

        return first;
    }

    void Reverse() noexcept
    {
        if (m_head.m_next == m_head.m_prev)
        {
            return;
        }
        ListBasicNode* cur = &m_head;
        do
        {
            ListBasicNode* temp = cur->m_next;
            cur->m_next = cur->m_prev;
            cur->m_prev = temp;
            cur = cur->m_prev;
        }
        while (cur != &m_head);
        // do I get the job?
    }

    ListBasicNode m_head;
};

inline Err ToErr(void* ptr)
{
    if (ptr == nullptr)
    {
        return Error::NoMemory;
    }
    return EmptyOkType{};
}

} // namespace detail

} // namespace rad
