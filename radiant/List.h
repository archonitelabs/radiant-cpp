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

#if _MSC_VER >= 1929 // VS2019 v16.10 and later
// C4848: msvc::no_unique_address in C++17 and earlier is a vendor extension
#define RAD_NO_UNIQUE_ADDRESS                                                  \
    _Pragma("warning(suppress : 4848)") [[msvc::no_unique_address]]
#else
#define RAD_NO_UNIQUE_ADDRESS [[no_unique_address]]
#endif

namespace rad
{

template <typename T, typename TAllocator RAD_ALLOCATOR_EQ(T)>
class List;

template <typename T>
class List_const_iterator;

class List_basic_node
{
public:

    List_basic_node() = default;
    ~List_basic_node() = default;

    // immovable
    List_basic_node(const List_basic_node&) = delete;
    List_basic_node(List_basic_node&&) = delete;
    List_basic_node& operator=(const List_basic_node&) = delete;
    List_basic_node& operator=(List_basic_node&&) = delete;

    void unlink()
    {
        m_prev = m_next = this;
    }

    void fixup_head_swap(_In_ const List_basic_node& old_head) noexcept
    {
        if (m_next == &old_head)
        {
            // This used to be an empty head, but now it
            // looks like the old head is the first element
            // in our list.  unlink to reestablish emptiness.
            unlink();
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

    void check_sanity_before_relinking() const noexcept
    {
        // TODO: turn the asserts into fast fails.
        // if they fire, it indicates a heap overflow,
        // and someone is trying to build a write primitive
        RAD_ASSERT(m_next->m_prev == this);
        RAD_ASSERT(m_prev->m_next == this);
    }

    void swap(_Inout_ List_basic_node& x) noexcept
    {
        List_basic_node* temp = m_next;
        m_next = x.m_next;
        x.m_next = temp;

        temp = m_prev;
        m_prev = x.m_prev;
        x.m_prev = temp;
    }

    List_basic_node* m_next = this;
    List_basic_node* m_prev = this;
};

template <typename T>
class List_node : public List_basic_node
{
public:

    List_node() = default;
    ~List_node() = default;

    // used by emplace
    template <class... Args>
    explicit List_node(Args&&... args)
        : List_basic_node(),
          m_elt(static_cast<Args&&>(args)...)
    {
    }

    // immovable
    List_node(const List_node&) = delete;
    List_node(List_node&&) = delete;
    List_node& operator=(const List_node&) = delete;
    List_node& operator=(List_node&&) = delete;

    T m_elt;
};

template <typename T>
class List_iterator
{
public:

#if RAD_ENABLE_STD
    using iterator_category = std::bidirectional_iterator_tag;
#endif

    using value_type = T;
    using difference_type = ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    List_iterator() = default;

    explicit List_iterator(List_basic_node* node)
        : m_node(node)
    {
    }

    List_iterator(const List_iterator&) = default;
    List_iterator(List_iterator&&) = default;
    List_iterator& operator=(const List_iterator&) = default;
    List_iterator& operator=(List_iterator&&) = default;
    ~List_iterator() = default;

    bool operator==(List_iterator rhs) const noexcept
    {
        return m_node == rhs.m_node;
    }

    bool operator!=(List_iterator rhs) const noexcept
    {
        return m_node != rhs.m_node;
    }

    reference operator*() const noexcept
    {
        return static_cast<List_node<T>*>(m_node)->m_elt;
    }

    pointer operator->() const noexcept
    {
        return &static_cast<List_node<T>*>(m_node)->m_elt;
    }

    List_iterator& operator++() noexcept
    {
        m_node = m_node->m_next;
        return *this;
    }

    List_iterator operator++(int) noexcept
    {
        List_iterator retval(m_node);
        m_node = m_node->m_next;
        return retval;
    }

    List_iterator& operator--() noexcept
    {
        m_node = m_node->m_prev;
        return *this;
    }

    List_iterator operator--(int) noexcept
    {
        List_iterator retval(m_node);
        m_node = m_node->m_prev;
        return *this;
    }

private:

    List_basic_node* m_node = nullptr;

    template <typename U, typename TAllocator>
    friend class List;

    friend class List_const_iterator<T>;
};

template <typename T>
class List_const_iterator
{
public:

#if RAD_ENABLE_STD
    using iterator_category = std::bidirectional_iterator_tag;
#endif

    using value_type = const T;
    using difference_type = ptrdiff_t;
    using pointer = const T*;
    using reference = const T&;

    List_const_iterator() = default;

    explicit List_const_iterator(const List_basic_node* node)
        : m_node(node)
    {
    }

    List_const_iterator(const List_const_iterator&) = default;
    List_const_iterator(List_const_iterator&&) = default;

    List_const_iterator& operator=(const List_const_iterator&) = default;
    List_const_iterator& operator=(List_const_iterator&&) = default;

    ~List_const_iterator() = default;

    /* implicit */ List_const_iterator(List_iterator<T> other) noexcept
        : m_node(other.m_node)
    {
    }

    bool operator==(List_const_iterator rhs) const noexcept
    {
        return m_node == rhs.m_node;
    }

    bool operator!=(List_const_iterator rhs) const noexcept
    {
        return m_node != rhs.m_node;
    }

    reference operator*() const noexcept
    {
        return static_cast<const List_node<T>*>(m_node)->m_elt;
    }

    pointer operator->() const noexcept
    {
        return &static_cast<const List_node<T>*>(m_node)->m_elt;
    }

    List_const_iterator& operator++() noexcept
    {
        m_node = m_node->m_next;
        return *this;
    }

    List_const_iterator operator++(int) noexcept
    {
        List_const_iterator retval(m_node);
        m_node = m_node->m_next;
        return retval;
    }

    List_const_iterator& operator--() noexcept
    {
        m_node = m_node->m_prev;
        return *this;
    }

    List_const_iterator operator--(int) noexcept
    {
        List_const_iterator retval(m_node);
        m_node = m_node->m_prev;
        return *this;
    }

private:

    const List_basic_node* m_node = nullptr;

    template <typename U, typename TAllocator>
    friend class List;
};

class List_untyped
{
public:

    List_untyped() = default;

    List_untyped(_In_ const List_untyped& x) = delete;

    List_untyped(_Inout_ List_untyped&& x) noexcept
    {
        m_head.m_prev = x.m_head.m_prev;
        m_head.m_next = x.m_head.m_next;
        x.m_head.unlink();
    }

    ~List_untyped() = default;
    List_untyped& operator=(_In_ const List_untyped& x) = delete;
    List_untyped& operator=(_Inout_ List_untyped&& x) noexcept;

    // [list.capacity], capacity
    RAD_NODISCARD bool empty() const noexcept
    {
        return m_head.m_next == &m_head;
    }

    // O(N) operation, renamed so that people don't
    // assume it is cheap and make things accidentally
    // quadratic.  Too useful in test code to omit
    // entirely.
    RAD_NODISCARD size_t expensive_size() const noexcept
    {
        size_t count = 0;
        const List_basic_node* cur = &m_head;
        while (cur->m_next != &m_head)
        {
            cur = cur->m_next;
            ++count;
        }
        return count;
    }

    void swap(_Inout_ List_untyped& x) noexcept
    {
        m_head.swap(x.m_head);
        m_head.fixup_head_swap(x.m_head);
        x.m_head.fixup_head_swap(m_head);
    }

    void attach_new_node(_Inout_ List_basic_node* pos,
                         _Inout_ List_basic_node* i)
    {
        RAD_ASSERT(i->m_next == i);
        RAD_ASSERT(i->m_prev == i);
        i->m_next = pos;
        i->m_prev = pos->m_prev;

        pos->check_sanity_before_relinking();
        pos->m_prev->m_next = i;
        pos->m_prev = i;
    }

    void splice_one(_Inout_ List_basic_node* pos, _Inout_ List_basic_node* i)
    {
        i->check_sanity_before_relinking();
        i->m_next->m_prev = i->m_prev;
        i->m_prev->m_next = i->m_next;

        i->m_next = pos;
        i->m_prev = pos->m_prev;

        pos->check_sanity_before_relinking();
        pos->m_prev->m_next = i;
        pos->m_prev = i;
    }

    void splice_some(_Inout_ List_basic_node* position,
                     _Inout_ List_basic_node* first,
                     _Inout_ List_basic_node* last);

    void reverse() noexcept;

    List_basic_node m_head;
};

/*!
    @brief Cross platform (and kernel appropriate) doubly linked list
    implementation

    @details Deviations from std::list:

    The container isn't copiable using the copy constructor or copy assignment.
    This is because there is no way to indicate failure in these cases.  Move
    construction and move assignment work fine, and are guaranteed noexcept. Use
    assign_range if you need a deep copy.

    The set of constructors has been greatly reduced, due to error handling
    difficulties when exceptions aren't available.

    The allocator returns nullptr on failure, rather than throws an exception.
    That error is propogated.  Allocators are always propagated. "Fancy
   pointers" aren't supported, as the allocators used aren't std allocators.  So
   you won't be able to use some offset based pointer to do shared memory things
   with this container.

    Reverse iterators aren't currently supported, though there isn't any reason
    that they couldn't be supported.  The return on investment isn't currently
    there. Same story for unique, merge, and sort.

    Removed size() related functions, as making size() O(1) interferes with
    efficient splicing, and having size() be O(N) is too much of a foot gun. The
    standard made a mistake here.  empty() is still present and fine, and
    expensive_size() exists largely for test code.

    The assign functions now all return NTSTATUS to signal errors.  The count
    and initializer_list overloads of assign were renamed to assign_count and
    assign_initializer_list to minimize overloading

    Renamed remove and remove_if to erase_value and erase_if, as the "remove"
    names are mistakes in the standard.  "remove" usually refers to algorithms
    that move unwanted elements out of the way, but don't change the size of the
    container. "erase" will get rid of unwanted elements and change the
   container size.

    Renamed iterator overloads of "erase" to "erase_one" and "erase_some" to
    better capture caller intent.  There's a common bug where devs call
    `erase(range_begin)` when they meant to call `erase(range_begin,
    range_end)`, and splitting the "erase" overload set into two names
   eliminates that bug.

    No Constructor Template Argument Deduction(CTAD), because that all requires
    constructors that will do some initial population of the container, and we
    don't have those constructors since we can't signal errors out of
    constructors.

    The splice family of functions were renamed to splice_all, splice_one, and
    splice_some, to better make their intent clear to readers without needing to
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
    using value_type = T;
    using allocator_type = TAllocator;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = value_type&;
    using const_reference = const value_type&;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using iterator = List_iterator<T>;
    using const_iterator = List_const_iterator<T>;

    using Rebound = typename TAllocator::template Rebind<List_node<T>>::Other;

    List()
        : List(TAllocator{}) {};

    explicit List(_In_ const TAllocator& alloc)
        : m_alloc(alloc)
    {
    }

    List(_In_ const List& x) = delete;

    List(_Inout_ List&& x) noexcept
    {
        swap(x);
    }

    ~List()
    {
        clear();
    }

    List& operator=(_In_ const List& x) = delete;

    List& operator=(_Inout_ List&& x) noexcept
    {
        clear();
        swap(x);
        return *this;
    }

    template <class InputIterator>
    Err assign_some(InputIterator first, InputIterator last)
    {
        List local(m_alloc);
        for (; first != last; ++first)
        {
            Err res = local.emplace_back(*first);
            if (res.IsErr())
            {
                return res;
            }
        }
        // using m_list swap so that we don't need to swap allocators
        m_list.swap(local.m_list);
        return EmptyOkType{};
    }

    template <typename InputRange>
    Err assign_range(InputRange&& rg);

    Err assign_count(size_type n, _In_ const T& t);

#if RAD_ENABLE_STD
    Err assign(std::initializer_list<T> il);
#endif

    allocator_type get_allocator() const noexcept
    {
        return m_alloc;
    }

    // iterators
    RAD_NODISCARD iterator begin() noexcept
    {
        return iterator(m_list.m_head.m_next);
    }

    RAD_NODISCARD const_iterator begin() const noexcept
    {
        return const_iterator(m_list.m_head.m_next);
    }

    RAD_NODISCARD iterator end() noexcept
    {
        return iterator(&m_list.m_head);
    }

    RAD_NODISCARD const_iterator end() const noexcept
    {
        return const_iterator(&m_list.m_head);
    }

    RAD_NODISCARD const_iterator cbegin() const noexcept
    {
        return const_iterator(m_list.m_head.m_next);
    }

    RAD_NODISCARD const_iterator cend() const noexcept
    {
        return const_iterator(&m_list.m_head);
    }

    // [list.capacity], capacity
    RAD_NODISCARD bool empty() const noexcept
    {
        // debug perf optimization:  Duplicate "empty"
        // implementation, rather than forward the call down
        // three levels.
        return m_list.m_head.m_next == &m_list.m_head;
    }

    // O(N) operation, renamed so that people don't
    // assume it is cheap and make things accidentally
    // quadratic.  Too useful in test code to omit
    // entirely.
    RAD_NODISCARD size_t expensive_size() const noexcept
    {
        return m_list.expensive_size();
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
    // Deal with this later.
    // reference       front();
    // const_reference front() const;
    // reference       back();
    // const_reference back() const;

    // [list.modifiers], modifiers
    template <class... Args>
    Err emplace_front(Args&&... args);

    template <class... Args>
    Err emplace_back(Args&&... args)
    {
        List_node<T>* storage = rebound_alloc().Alloc(1);
        if (storage == nullptr)
        {
            return Error::NoMemory;
        }
        // forward to placement new
        List_node<T>* new_node =
            new (storage) List_node<T>(static_cast<Args&&>(args)...);
        // attach the new node before the end node.
        m_list.attach_new_node(&m_list.m_head, new_node);

        return EmptyOkType{};
    }

    Err push_front(_In_ const T& x);
    Err push_front(_Inout_ T&& x);

    template <typename InputRange>
    Err prepend_range(InputRange&& rg);

    // Calling pop_front while the container is empty is erroneous behavior.
    // It's wrong to do it, and we can diagnose it in debug mode, but in
    // release mode we will instead do nothing.
    void pop_front();

    Err push_back(_In_ const T& x)
    {
        return emplace_back(x);
    }

    Err push_back(_Inout_ T&& x)
    {
        return emplace_back(static_cast<T&&>(x));
    }

    template <typename InputRange>
    Err append_range(InputRange&& rg);

    // Calling pop_back while the container is empty is erroneous behavior.
    // It's wrong to do it, and we can diagnose it in debug mode, but in
    // release mode we will instead do nothing.
    void pop_back();

    // insert and emplace functions return the end iterator on failure.
    // The insert and emplace functions provide the strong error
    // guarantee.  If they fail, then the function returns without
    // changing the container, invalidating iterators, or invalidating
    // references.
    template <class... Args>
    RAD_NODISCARD iterator emplace(const_iterator position, Args&&... args)
    {
        List_node<T>* storage = rebound_alloc().Alloc(1);
        if (storage == nullptr)
        {
            return iterator(&m_list.m_head);
        }
        // forward to placement new
        List_node<T>* new_node =
            new (storage) List_node<T>(static_cast<Args&&>(args)...);
        // insert the new node before the end node.
        m_list.attach_new_node(&position.m_node, new_node);

        return iterator(new_node);
    }

    RAD_NODISCARD iterator insert(const_iterator position, _In_ const T& x);
    RAD_NODISCARD iterator insert(const_iterator position, _Inout_ T&& x);

    RAD_NODISCARD iterator insert_count(const_iterator position,
                                        size_type n,
                                        _In_ const T& x);

    template <class InputIterator>
    RAD_NODISCARD iterator insert_some(const_iterator position,
                                       InputIterator first,
                                       InputIterator last);

    template <typename InputRange>
    RAD_NODISCARD iterator insert_range(const_iterator position,
                                        InputRange&& rg);

#if RAD_ENABLE_STD
    RAD_NODISCARD iterator insert_initializer_list(const_iterator position,
                                                   std::initializer_list<T> il);
#endif

    iterator erase_one(const_iterator position);
    iterator erase_some(const_iterator position, const_iterator last);

    size_type erase_value(const T& value);
    template <typename Predicate>
    size_type erase_if(Predicate pred);

    void swap(_Inout_ List& x) noexcept
    {
        {
            TAllocator temp = m_alloc;
            m_alloc = x.m_alloc;
            x.m_alloc = temp;
        }
        m_list.swap(x.m_list);
    }

    void clear() noexcept
    {
        List_basic_node* cur = m_list.m_head.m_next;
        while (cur != &m_list.m_head)
        {
            List_node<T>* typed = static_cast<List_node<T>*>(cur);
            cur = cur->m_next; // TODO suppress C6001 uninit memory warning?

            typed->~List_node();
            rebound_alloc().Free(typed);
        }
        m_list.m_head.unlink();
    }

    // [list.ops], list operations
    void splice_all(const_iterator position, _Inout_ List& x);
    void splice_all(const_iterator position, _Inout_ List&& x);

    void splice_one(const_iterator position, _Inout_ List& x, const_iterator i);
    void splice_one(const_iterator position,
                    _Inout_ List&& x,
                    const_iterator i);

    void splice_some(const_iterator position,
                     _Inout_ List& x,
                     const_iterator first,
                     const_iterator last);
    void splice_some(const_iterator position,
                     _Inout_ List&& x,
                     const_iterator first,
                     const_iterator last);

    void reverse() noexcept;

private:

    Rebound rebound_alloc()
    {
        return m_alloc;
    }

    List_untyped m_list;
    RAD_NO_UNIQUE_ADDRESS TAllocator m_alloc;
};

} // namespace rad
