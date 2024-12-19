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

#define RAD_DEFAULT_ALLOCATOR radtest::Mallocator

#include "gtest/gtest.h"
#include "test/TestAlloc.h"
#include "test/TestThrow.h"

#include "radiant/SharedPtr.h"

namespace sptestobjs
{
// clang-format off
using NoThrowAllocSp = rad::SharedPtr<int>;
using NoThrowObjBlock = rad::detail::PtrBlock<int, radtest::Mallocator>;
using ThrowObjBlock = rad::detail::PtrBlock<radtest::ThrowingObject, radtest::Mallocator>;
using NoThrowPair = NoThrowObjBlock::PairType;
using ThrowPair = ThrowObjBlock::PairType;

using NoThrowObjSp = NoThrowAllocSp;
using ThrowObjSp = rad::SharedPtr<radtest::ThrowingObject>;
// clang-format on

// PtrBlock::PairType construction noexcept
RAD_S_ASSERT(noexcept(NoThrowPair(rad::DeclVal<NoThrowPair::FirstType&>(), 1)));
RAD_S_ASSERT(!noexcept(ThrowPair(rad::DeclVal<ThrowPair::FirstType&>())));

// PtrBlock construction noexcept (EopPair constructor passthrough)
RAD_S_ASSERT(noexcept(NoThrowObjBlock(NoThrowObjBlock::AllocatorType())));
RAD_S_ASSERT(!noexcept(ThrowObjBlock(ThrowObjBlock::AllocatorType())));

// allocate shared noexcept
RAD_S_ASSERT(!noexcept(rad::AllocateShared<int>(radtest::Mallocator(), 0)));

// allocate shared throwing constructor
RAD_S_ASSERT(!noexcept(
    rad::AllocateShared<radtest::ThrowingObject>(radtest::Mallocator())));

// make shared noexcept
RAD_S_ASSERT(!noexcept(rad::MakeShared<int>(0)));
RAD_S_ASSERT(!noexcept(rad::MakeShared<radtest::ThrowingObject>()));

class Base
{
public:

    virtual ~Base()
    {
    }

    size_t val;
};

struct Extra
{
    int extra;
};

class Derived : public Base,
                public Extra

{
public:

    virtual ~Derived()
    {
    }
};
} // namespace sptestobjs

struct DestructCounter
{
    ~DestructCounter()
    {
        ++counter;
    }

    static int counter;
};

int DestructCounter::counter = 0;

namespace
{
sptestobjs::NoThrowObjBlock* AllocTestBlock(int val)
{
    typename sptestobjs::NoThrowObjBlock::AllocatorType alloc;
    auto block = alloc.AllocBytes(sizeof(sptestobjs::NoThrowObjBlock));
    return new (block) sptestobjs::NoThrowObjBlock(alloc, val);
}

void FreeTestBlock(sptestobjs::NoThrowObjBlock* block)
{
    block->Allocator().FreeBytes(block, sizeof(sptestobjs::NoThrowObjBlock));
}
} // namespace

TEST(TestSharedPtr, RefCountCtor)
{
    rad::detail::PtrRefCount rc;
    EXPECT_EQ(rc.StrongCount(), 1u);
    EXPECT_EQ(rc.WeakCount(), 1u);
}

TEST(TestSharedPtr, RefCountIncrement)
{
    rad::detail::PtrRefCount rc;
    rc.Increment();
    EXPECT_EQ(rc.StrongCount(), 2u);
    EXPECT_EQ(rc.WeakCount(), 1u);
}

TEST(TestSharedPtr, RefCountIncrementWeak)
{
    rad::detail::PtrRefCount rc;
    rc.IncrementWeak();
    EXPECT_EQ(rc.StrongCount(), 1u);
    EXPECT_EQ(rc.WeakCount(), 2u);
}

TEST(TestSharedPtr, RefCountDrecment)
{
    rad::detail::PtrRefCount rc;
    rc.Increment();
    EXPECT_FALSE(rc.Decrement());
    EXPECT_EQ(rc.StrongCount(), 1u);
    EXPECT_EQ(rc.WeakCount(), 1u);

    EXPECT_TRUE(rc.Decrement());
    EXPECT_EQ(rc.StrongCount(), 0u);
    EXPECT_EQ(rc.WeakCount(), 1u);
}

TEST(TestSharedPtr, RefCountDecrementWeak)
{
    rad::detail::PtrRefCount rc;
    rc.IncrementWeak();
    EXPECT_FALSE(rc.DecrementWeak());
    EXPECT_EQ(rc.StrongCount(), 1u);
    EXPECT_EQ(rc.WeakCount(), 1u);

    EXPECT_TRUE(rc.DecrementWeak());
    EXPECT_EQ(rc.StrongCount(), 1u);
    EXPECT_EQ(rc.WeakCount(), 0u);
}

TEST(TestSharedPtr, RefCountLockWeak)
{
    rad::detail::PtrRefCount rc;
    EXPECT_TRUE(rc.LockWeak());
    EXPECT_EQ(rc.StrongCount(), 2u);
    EXPECT_EQ(rc.WeakCount(), 1u);
}

class MockAtomic
{
public:

    MockAtomic(uint32_t)
    {
    }

    uint32_t Load(rad::detail::atomic::RelaxedTag)
    {
        static uint32_t ret = 0;
        return ret > 1 ? 1 : ret++;
    }

    bool CompareExchangeWeak(uint32_t& expected,
                             uint32_t,
                             rad::detail::atomic::AcqRelTag,
                             rad::detail::atomic::RelaxedTag)
    {
        expected = 0;
        return false;
    }
};

TEST(TestSharedPtr, RefCountLockWeakFailExchange)
{
    rad::detail::TPtrRefCount<MockAtomic> rc;

    EXPECT_FALSE(rc.LockWeak());
    EXPECT_FALSE(rc.LockWeak());
    EXPECT_EQ(rc.StrongCount(), 1u);
    EXPECT_EQ(rc.WeakCount(), 1u);
}

TEST(TestSharedPtr, PtrBlockCtor)
{
    using PtrBlock = rad::detail::PtrBlock<int, radtest::Mallocator>;
    PtrBlock::AllocatorType alloc;
    PtrBlock block(alloc, 2);
    EXPECT_EQ(block.Value(), 2);

    using StatefulPtrBlock =
        rad::detail::PtrBlock<int, radtest::StatefulAllocator>;
    StatefulPtrBlock::AllocatorType statefulAlloc;
    StatefulPtrBlock statefulBlock(statefulAlloc, 4);
    RAD_S_ASSERT(sizeof(statefulBlock) >
                 sizeof(decltype(statefulBlock)::PairType::SecondType));
}

TEST(TestSharedPtr, PtrBlockAcquire)
{
    auto block = AllocTestBlock(2);
    ASSERT_NE(block, nullptr);

    block->Acquire();
    ASSERT_EQ(block->UseCount(), 2u);
    ASSERT_EQ(block->WeakCount(), 1u);

    FreeTestBlock(block);
}

TEST(TestSharedPtr, PtrBlockAcquireWeak)
{
    auto block = AllocTestBlock(2);
    ASSERT_NE(block, nullptr);

    block->AcquireWeak();
    ASSERT_EQ(block->UseCount(), 1u);
    ASSERT_EQ(block->WeakCount(), 2u);

    FreeTestBlock(block);
}

TEST(TestSharedPtr, LockWeak)
{
    auto block = AllocTestBlock(2);
    ASSERT_NE(block, nullptr);

    block->LockWeak();
    ASSERT_EQ(block->UseCount(), 2u);
    ASSERT_EQ(block->WeakCount(), 1u);

    FreeTestBlock(block);
}

TEST(TestSharedPtr, ReleaseDestruct)
{
    using PtrBlock =
        rad::detail::PtrBlock<DestructCounter, radtest::Mallocator>;
    PtrBlock::AllocatorType alloc;
    PtrBlock block(alloc);
    EXPECT_EQ(DestructCounter::counter, 0);

    block.AcquireWeak();
    block.Release();
    EXPECT_EQ(block.UseCount(), 0u);
    EXPECT_EQ(DestructCounter::counter, 1);
}

TEST(TestSharedPtr, ReleaseFree)
{
    using PtrBlock =
        rad::detail::PtrBlock<int, radtest::StatefulCountingAllocator>;
    PtrBlock::AllocatorType alloc;
    alloc.ResetCounts();

    void* mem = alloc.AllocBytes(sizeof(PtrBlock));
    PtrBlock* block = new (mem) PtrBlock(alloc);

    block->Release();
    EXPECT_EQ(alloc.AllocCount(), 1u);
    alloc.VerifyCounts();
}

TEST(TestSharedPtr, Value)
{
    auto block = AllocTestBlock(2);
    ASSERT_NE(block, nullptr);

    ASSERT_EQ(block->Value(), 2);

    const sptestobjs::NoThrowObjBlock* constBlock = block;
    ASSERT_EQ(constBlock->Value(), 2);

    FreeTestBlock(block);
}

TEST(TestSharedPtr, NullCtor)
{
    rad::SharedPtr<int> ptr;
    EXPECT_EQ(ptr, false);

    rad::SharedPtr<int> ptr2(nullptr);
    EXPECT_EQ(ptr2, false);
}

TEST(TestSharedPtr, AllocateShared)
{
    radtest::Mallocator alloc;
    auto ptr = rad::AllocateShared<int>(alloc, 2);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(*ptr, 2);
}

TEST(TestSharedPtr, AllocateSharedFail)
{
    radtest::FailingAllocator alloc;
    auto ptr = rad::AllocateShared<int>(alloc, 2);
    EXPECT_FALSE(ptr);
}

TEST(TestSharedPtr, AllocateSharedThrows)
{
    radtest::StatefulCountingAllocator alloc;
    alloc.ResetCounts();

    EXPECT_THROW(rad::AllocateShared<radtest::ThrowingObject>(alloc, 1),
                 std::exception);
    EXPECT_EQ(alloc.AllocCount(), 1u);
    alloc.VerifyCounts();
}

TEST(TestSharedPtr, MakeShared)
{
    auto ptr = rad::MakeShared<int>(2);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(*ptr, 2);
}

TEST(TestSharedPtr, CopyCtor)
{
    auto ptr = rad::MakeShared<int>(2);

    decltype(ptr) ptr2(ptr);
    EXPECT_EQ(ptr.UseCount(), 2u);
    EXPECT_EQ(ptr2.UseCount(), 2u);
    EXPECT_EQ(*ptr, *ptr2);
}

TEST(TestSharedPtr, MoveCtor)
{
    auto ptr = rad::MakeShared<int>(2);

    decltype(ptr) ptr2(Move(ptr));
    EXPECT_FALSE(ptr);
    EXPECT_TRUE(ptr2);
    EXPECT_EQ(ptr2.UseCount(), 1u);
    EXPECT_EQ(*ptr2, 2);
}

TEST(TestSharedPtr, EqualNull)
{
    rad::SharedPtr<int> ptr;
    EXPECT_FALSE(ptr);
    EXPECT_EQ(ptr, nullptr);

    EXPECT_EQ(nullptr, ptr);
}

TEST(TestSharedPtr, NotEqualNull)
{
    auto ptr = rad::MakeShared<int>(2);
    EXPECT_TRUE(ptr);
    EXPECT_NE(ptr, nullptr);

    EXPECT_NE(nullptr, ptr);
}

TEST(TestSharedPtr, Equal)
{
    auto left = rad::MakeShared<int>();
    auto right = rad::MakeShared<int>();

    EXPECT_FALSE(left == right);
    left = right;
    EXPECT_TRUE(right == left);
}

TEST(TestSharedPtr, NotEqual)
{
    auto left = rad::MakeShared<int>();
    auto right = rad::MakeShared<int>();

    EXPECT_TRUE(left != right);
    left = right;
    EXPECT_FALSE(right != left);
}

TEST(TestSharedPtr, LessThan)
{
    auto a = rad::MakeShared<int>();
    auto b = rad::MakeShared<int>();
    auto& left = a.Get() < b.Get() ? a : b;
    auto& right = a.Get() < b.Get() ? b : a;

    EXPECT_TRUE(left < right);
    EXPECT_FALSE(right < left);
}

TEST(TestSharedPtr, LessThanOrEqual)
{
    auto a = rad::MakeShared<int>();
    auto b = rad::MakeShared<int>();
    auto& left = a.Get() < b.Get() ? a : b;
    auto& right = a.Get() < b.Get() ? b : a;

    EXPECT_TRUE(left <= right);
    EXPECT_FALSE(right <= left);
    left = right;
    EXPECT_TRUE(left <= right);
    EXPECT_TRUE(right <= left);
}

TEST(TestSharedPtr, GreaterThan)
{
    auto a = rad::MakeShared<int>();
    auto b = rad::MakeShared<int>();
    auto& left = a.Get() > b.Get() ? a : b;
    auto& right = a.Get() > b.Get() ? b : a;

    EXPECT_TRUE(left > right);
    EXPECT_FALSE(right > left);
}

TEST(TestSharedPtr, GreaterThanOrEqual)
{
    auto a = rad::MakeShared<int>();
    auto b = rad::MakeShared<int>();
    auto& left = a.Get() > b.Get() ? a : b;
    auto& right = a.Get() > b.Get() ? b : a;

    EXPECT_TRUE(left >= right);
    EXPECT_FALSE(right >= left);
    left = right;
    EXPECT_TRUE(left >= right);
    EXPECT_TRUE(right >= left);
}

TEST(TestSharedPtr, Deref)
{
    auto ptr = rad::MakeShared<int>(2);
    EXPECT_NE(ptr.Get(), nullptr);
    EXPECT_EQ(*ptr.Get(), 2);

    EXPECT_EQ(*ptr, 2);
    EXPECT_EQ(*ptr.operator->(), 2);

    const rad::SharedPtr<int> constPtr(ptr);
    EXPECT_EQ(*constPtr, 2);
    EXPECT_EQ(*constPtr.operator->(), 2);
}

TEST(TestSharedPtr, Reset)
{
    auto ptr = rad::MakeShared<int>(2);

    EXPECT_NE(ptr, nullptr);

    ptr.Reset();

    EXPECT_EQ(ptr, nullptr);
}

TEST(TestSharedPtr, Swap)
{
    auto ptr = rad::MakeShared<int>(2);

    ptr.Swap(ptr);

    EXPECT_EQ(*ptr, 2);

    auto other = rad::MakeShared<int>(3);

    ptr.Swap(other);

    EXPECT_EQ(*other, 2);
    EXPECT_EQ(*ptr, 3);
}

TEST(TestSharedPtr, SelfCopyAssign)
{
    radtest::StatefulCountingAllocator alloc;
    alloc.ResetCounts();

    auto ptr = rad::AllocateShared<int>(alloc, 2);
    auto* somePtr = &ptr; // clang workaround -Wself-assign-overloaded
    ptr = *somePtr;
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.UseCount(), 1u);

    EXPECT_EQ(alloc.AllocCount(), 1u);
}

TEST(TestSharedPtr, CopyAssignNoReset)
{
    radtest::StatefulCountingAllocator alloc;
    alloc.ResetCounts();

    auto ptr = rad::AllocateShared<int>(alloc, 2);
    decltype(ptr) ptr2;

    ptr2 = ptr;
    EXPECT_TRUE(ptr2);
    EXPECT_EQ(ptr.UseCount(), 2u);
    EXPECT_EQ(ptr.WeakCount(), 1u);

    EXPECT_EQ(alloc.AllocCount(), 1u);
}

TEST(TestSharedPtr, CopyAssignReset)
{
    radtest::StatefulCountingAllocator alloc;
    alloc.ResetCounts();

    auto ptr = rad::AllocateShared<int>(alloc, 2);
    auto ptr2 = rad::AllocateShared<int>(alloc, 3);

    ptr2 = ptr;
    EXPECT_TRUE(ptr2);
    EXPECT_EQ(*ptr2, 2);
    EXPECT_EQ(*ptr2, *ptr);

    alloc.VerifyCounts(2, 1);
}

TEST(TestSharedPtr, CopyAssignNull)
{
    radtest::StatefulCountingAllocator alloc;
    alloc.ResetCounts();

    auto ptr = rad::AllocateShared<int>(alloc, 2);
    ptr = nullptr;
    EXPECT_FALSE(ptr);

    EXPECT_EQ(alloc.FreeCount(), 1u);
}

TEST(TestSharedPtr, SelfMoveAssign)
{
    radtest::StatefulCountingAllocator alloc;
    alloc.ResetCounts();

    auto ptr = rad::AllocateShared<int>(alloc, 2);
    auto* somePtr = &ptr; // clang workaround -Wself-move
    ptr = Move(*somePtr);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(ptr.UseCount(), 1u);

    alloc.VerifyCounts(1, 0);
}

TEST(TestSharedPtr, MoveAssignNoReset)
{
    radtest::StatefulCountingAllocator alloc;
    alloc.ResetCounts();

    auto ptr = rad::AllocateShared<int>(alloc, 2);
    decltype(ptr) ptr2;

    ptr2 = Move(ptr);
    EXPECT_TRUE(ptr2);
    EXPECT_FALSE(ptr);
    EXPECT_EQ(ptr2.UseCount(), 1u);
    EXPECT_EQ(ptr2.WeakCount(), 1u);

    alloc.VerifyCounts(1, 0);
}

TEST(TestSharedPtr, MoveAssignReset)
{
    radtest::StatefulCountingAllocator alloc;
    alloc.ResetCounts();

    auto ptr = rad::AllocateShared<int>(alloc, 2);
    auto ptr2 = rad::AllocateShared<int>(alloc, 3);

    ptr2 = Move(ptr);
    EXPECT_TRUE(ptr2);
    EXPECT_FALSE(ptr);
    EXPECT_EQ(*ptr2, 2);
    EXPECT_EQ(ptr2.UseCount(), 1u);
    EXPECT_EQ(ptr2.WeakCount(), 1u);

    alloc.VerifyCounts(2, 1);
}

TEST(TestSharedPtr, PolymorphicCtor)
{
    sptestobjs::Derived d;
    sptestobjs::Base* b = &d;
    sptestobjs::Extra* e = &d;
    EXPECT_EQ(static_cast<void*>(&d), static_cast<void*>(b));
    EXPECT_NE(static_cast<void*>(b), static_cast<void*>(e));

    radtest::StatefulCountingAllocator alloc;
    alloc.ResetCounts();

    {
        auto ptr = rad::AllocateShared<sptestobjs::Derived>(alloc);
        ptr->val = 1;
        ptr->extra = 2;

        rad::SharedPtr<sptestobjs::Base> bptr(ptr);
        rad::SharedPtr<sptestobjs::Extra> eptr(ptr);

        EXPECT_EQ(ptr.UseCount(), 3u);
        EXPECT_EQ(bptr->val, 1u);
        EXPECT_EQ(eptr->extra, 2);
    }

    EXPECT_EQ(alloc.AllocCount(), 1u);
    alloc.VerifyCounts();
}

TEST(TestSharedPtr, PolymorphicAssign)
{
    radtest::StatefulCountingAllocator alloc;
    alloc.ResetCounts();

    {
        rad::SharedPtr<sptestobjs::Extra> eptr;
        {
            auto ptr = rad::AllocateShared<sptestobjs::Derived>(alloc);
            ptr->extra = 2;

            eptr = ptr;

            EXPECT_EQ(ptr.UseCount(), 2u);
            EXPECT_EQ(eptr->extra, 2);

            eptr = ptr;

            EXPECT_EQ(ptr.UseCount(), 2u);
            EXPECT_EQ(eptr->extra, 2);
        }
    }

    EXPECT_EQ(alloc.AllocCount(), 1u);
    alloc.VerifyCounts();
}

TEST(TestSharedPtr, StatefulAllocator)
{
    radtest::StatefulCountingAllocator alloc;
    alloc.ResetCounts();

    auto ptr = rad::AllocateShared<int>(alloc, 2);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(*ptr, 2);
    EXPECT_EQ(alloc.AllocCount(), 1u);
    EXPECT_EQ(alloc.FreeCount(), 0u);

    ptr.Reset();
    EXPECT_EQ(alloc.AllocCount(), 1u);
    EXPECT_EQ(alloc.FreeCount(), 1u);
}

TEST(TestSharedPtr, UsesTypedAlloc)
{
    radtest::TypedAllocator alloc;

    auto ptr = rad::AllocateShared<int>(alloc, 2);
    EXPECT_TRUE(ptr);
    EXPECT_EQ(*ptr, 2);

    ptr.Reset();
}

TEST(TestWeakPtr, ConstructEmpy)
{
    rad::WeakPtr<int> weak;

    auto ptr = weak.Lock();

    EXPECT_EQ(ptr, nullptr);
}

TEST(TestWeakPtr, Construct)
{
    auto shared = rad::MakeShared<int>(1);
    rad::WeakPtr<int> w1(shared);

    EXPECT_EQ(shared.WeakCount(), 2u);

    rad::WeakPtr<int> w2(w1);

    EXPECT_EQ(shared.WeakCount(), 3u);
}

TEST(TestWeakPtr, ConstructMove)
{
    auto shared = rad::MakeShared<int>(1);
    auto o = shared;

    rad::WeakPtr<int> w1(Move(o));

    EXPECT_EQ(shared.WeakCount(), 2u);

    rad::WeakPtr<int> w2(Move(w1));

    EXPECT_EQ(shared.WeakCount(), 2u);
}

TEST(TestWeakPtr, Assign)
{
    auto shared = rad::MakeShared<int>(1);
    rad::WeakPtr<int> w1;
    rad::WeakPtr<int> w2;

    w1 = shared;
    w2 = w1;

    EXPECT_EQ(shared.WeakCount(), 3u);

    w2 = shared;

    EXPECT_EQ(shared.WeakCount(), 3u);

    w1 = w2;

    EXPECT_EQ(shared.WeakCount(), 3u);
}

TEST(TestWeakPtr, AssignMove)
{
    auto shared = rad::MakeShared<int>(1);
    auto o = shared;

    rad::WeakPtr<int> w1;
    rad::WeakPtr<int> w2;

    w1 = Move(o);
    w2 = Move(w1);

    EXPECT_EQ(shared.WeakCount(), 2u);

    w1 = shared;
    w2 = Move(w1);

    EXPECT_EQ(shared.WeakCount(), 2u);
}

TEST(TestWeakPtr, Reset)
{
    auto shared = rad::MakeShared<int>(1);

    rad::WeakPtr<int> w1(shared);

    shared = w1.Lock();

    EXPECT_NE(shared, nullptr);

    w1.Reset();

    shared = w1.Lock();

    EXPECT_EQ(shared, nullptr);
}

TEST(TestWeakPtr, Swap)
{
    auto s1 = rad::MakeShared<int>(2);
    auto s2 = rad::MakeShared<int>(3);

    rad::WeakPtr<int> w1(s1);
    rad::WeakPtr<int> w2(s2);

    w1.Swap(w1);

    s1 = w1.Lock();

    EXPECT_EQ(*s1, 2);

    w1.Swap(w2);

    auto sa = w1.Lock();
    auto sb = w2.Lock();

    EXPECT_EQ(*sa, 3);
    EXPECT_EQ(*sb, 2);
}

TEST(TestWeakPtr, UseCount)
{
    auto s1 = rad::MakeShared<int>(2);

    rad::WeakPtr<int> w1(s1);

    EXPECT_EQ(w1.UseCount(), 1u);

    auto s2 = s1;

    EXPECT_EQ(w1.UseCount(), 2u);
}

TEST(TestWeakPtr, Expired)
{
    rad::WeakPtr<int> weak;

    EXPECT_TRUE(weak.Expired());

    {
        auto shared = rad::MakeShared<int>(2);
        weak = shared;

        EXPECT_FALSE(weak.Expired());
    }

    EXPECT_TRUE(weak.Expired());
}

TEST(TestWeakPtr, Lock)
{
    rad::WeakPtr<int> weak;
    rad::SharedPtr<int> p;

    EXPECT_TRUE(weak.Expired());

    {
        auto shared = rad::MakeShared<int>(2);
        weak = shared;
        p = weak.Lock();

        EXPECT_FALSE(weak.Expired());
    }

    EXPECT_FALSE(weak.Expired());

    EXPECT_EQ(p.Get(), weak.Lock().Get());

    p.Reset();

    EXPECT_TRUE(weak.Expired());
}

TEST(TestWeakPtr, PolymorphicCtor)
{
    sptestobjs::Derived d;
    sptestobjs::Base* b = &d;
    sptestobjs::Extra* e = &d;
    EXPECT_EQ(static_cast<void*>(&d), static_cast<void*>(b));
    EXPECT_NE(static_cast<void*>(b), static_cast<void*>(e));

    radtest::StatefulCountingAllocator alloc;
    alloc.ResetCounts();

    {
        auto ptr = rad::AllocateShared<sptestobjs::Derived>(alloc);
        ptr->val = 1;
        ptr->extra = 2;

        rad::WeakPtr<sptestobjs::Base> bptr(ptr);
        rad::WeakPtr<sptestobjs::Extra> eptr(ptr);

        EXPECT_EQ(ptr.UseCount(), 1u);
        EXPECT_EQ(bptr.Lock()->val, 1u);
        EXPECT_EQ(eptr.Lock()->extra, 2);

        rad::WeakPtr<sptestobjs::Derived> wptr(ptr);

        rad::WeakPtr<sptestobjs::Base> bptr2(wptr);
        rad::WeakPtr<sptestobjs::Extra> eptr2(wptr);

        EXPECT_EQ(ptr.UseCount(), 1u);
        EXPECT_EQ(bptr2.Lock()->val, 1u);
        EXPECT_EQ(eptr2.Lock()->extra, 2);

        rad::WeakPtr<sptestobjs::Base> bptr3(Move(wptr));

        EXPECT_EQ(wptr.Lock(), nullptr);

        EXPECT_EQ(ptr.UseCount(), 1u);
        EXPECT_EQ(bptr3.Lock()->val, 1u);
    }

    EXPECT_EQ(alloc.AllocCount(), 1u);
    alloc.VerifyCounts();
}

TEST(TestWeakPtr, PolymorphicAssign)
{
    radtest::StatefulCountingAllocator alloc;
    alloc.ResetCounts();

    {
        rad::WeakPtr<sptestobjs::Extra> eptr;
        {
            auto ptr = rad::AllocateShared<sptestobjs::Derived>(alloc);
            ptr->extra = 2;

            eptr = ptr;

            EXPECT_EQ(ptr.UseCount(), 1u);
            EXPECT_EQ(eptr.Lock()->extra, 2);

            eptr = ptr;

            EXPECT_EQ(ptr.UseCount(), 1u);
            EXPECT_EQ(eptr.Lock()->extra, 2);

            rad::WeakPtr<sptestobjs::Derived> wptr(ptr);

            rad::WeakPtr<sptestobjs::Base> bptr2;
            rad::WeakPtr<sptestobjs::Extra> eptr2;

            bptr2 = wptr;
            eptr2 = wptr;

            EXPECT_EQ(ptr.UseCount(), 1u);
            EXPECT_EQ(eptr2.Lock()->extra, 2);

            bptr2 = wptr;
            eptr2 = wptr;

            EXPECT_EQ(ptr.UseCount(), 1u);
            EXPECT_EQ(eptr2.Lock()->extra, 2);

            bptr2 = Move(wptr);

            EXPECT_EQ(wptr.Lock(), nullptr);
            EXPECT_EQ(ptr.UseCount(), 1u);
            EXPECT_EQ(eptr2.Lock()->extra, 2);
        }
    }

    EXPECT_EQ(alloc.AllocCount(), 1u);
    alloc.VerifyCounts();
}

TEST(TestAtomicSharedPtr, Construct)
{
    rad::AtomicSharedPtr<int> aptr;
    rad::SharedPtr<int> ptr = aptr.Load();
    EXPECT_EQ(ptr, nullptr);

    rad::AtomicSharedPtr<int> aptrTwo(nullptr);
    ptr = aptrTwo.Load();
    EXPECT_EQ(ptr, nullptr);

    auto sptr = rad::MakeShared<int>(123);
    rad::AtomicSharedPtr<int> aptrThree(sptr);
    ptr = aptrThree.Load();
    EXPECT_EQ(ptr.Get(), sptr.Get());
}

TEST(TestAtomicSharedPtr, Store)
{
    rad::AtomicSharedPtr<int> aptr;
    auto sptr = rad::MakeShared<int>(123);
    aptr.Store(sptr);
    auto ptr = aptr.Load();
    EXPECT_EQ(ptr.Get(), sptr.Get());
}

TEST(TestAtomicSharedPtr, Exchange)
{
    rad::AtomicSharedPtr<int> aptr;
    auto sptr = rad::MakeShared<int>(123);
    auto ptr = aptr.Exchange(sptr);
    EXPECT_EQ(ptr, nullptr);
    ptr = aptr.Exchange(ptr);
    EXPECT_EQ(ptr.Get(), sptr.Get());
}

TEST(TestAtomicSharedPtr, AssignmentOperator)
{
    rad::AtomicSharedPtr<int> aptr;
    aptr = rad::MakeShared<int>(123);
    auto ptr = aptr.Load();
    EXPECT_EQ(*ptr, 123);
}

namespace
{
rad::SharedPtr<int> AtomicConversionTester(rad::SharedPtr<int> value)
{
    return value;
}
} // namespace

TEST(TestAtomicSharedPtr, ConversionOperator)
{
    auto ptr = rad::MakeShared<int>(123);
    rad::AtomicSharedPtr<int> aptr(ptr);
    auto ptrTwo = AtomicConversionTester(aptr);
    EXPECT_EQ(ptrTwo.Get(), ptr.Get());
}

TEST(TestAtomicWeakPtr, Construct)
{
    rad::AtomicWeakPtr<int> aptr;
    auto ptr = aptr.Load();
    EXPECT_EQ(ptr.Lock(), nullptr);

    auto sptr = rad::MakeShared<int>(123);
    rad::AtomicWeakPtr<int> aptr2(sptr);
    ptr = aptr2.Load();

    sptr = ptr.Lock();
    EXPECT_EQ(*sptr, 123);

    rad::AtomicWeakPtr<int> aptr3(ptr);
    ptr = aptr3.Load();

    sptr = ptr.Lock();
    EXPECT_EQ(*sptr, 123);
}

TEST(TestAtomicWeakPtr, Store)
{
    rad::AtomicWeakPtr<int> aptr;
    auto sptr = rad::MakeShared<int>(123);
    aptr.Store(sptr);
    auto ptr = aptr.Load();
    EXPECT_EQ(ptr.Lock().Get(), sptr.Get());
}

TEST(TestAtomicWeakPtr, Exchange)
{
    rad::AtomicWeakPtr<int> aptr;
    auto sptr = rad::MakeShared<int>(123);
    auto ptr = aptr.Exchange(sptr);
    EXPECT_EQ(ptr.Lock(), nullptr);
    ptr = aptr.Exchange(ptr);
    EXPECT_EQ(ptr.Lock().Get(), sptr.Get());
}

TEST(TestAtomicWeakPtr, AssignmentOperator)
{
    auto sptr = rad::MakeShared<int>(123);
    rad::AtomicWeakPtr<int> aptr;
    aptr = sptr;
    auto ptr = aptr.Load();
    EXPECT_EQ(*(ptr.Lock()), 123);

    rad::AtomicWeakPtr<int> aptr2;
    aptr2 = aptr.Load();
    ptr = aptr2.Load();
    EXPECT_EQ(*(ptr.Lock()), 123);
}

TEST(TestAtomicWeakPtr, ConversionOperator)
{
    auto ptr = rad::MakeShared<int>(123);
    rad::AtomicWeakPtr<int> aptr(ptr);
    auto ptrTwo = AtomicConversionTester(aptr);
    EXPECT_EQ(ptrTwo.Get(), ptr.Get());
}

namespace
{
rad::WeakPtr<int> AtomicConversionTesterWeak(rad::WeakPtr<int> value)
{
    return value;
}
} // namespace

TEST(TestAtomicWeakPtr, ConversionOperatorWeak)
{
    auto ptr = rad::MakeShared<int>(123);
    rad::AtomicWeakPtr<int> aptr(ptr);
    auto ptrTwo = AtomicConversionTesterWeak(aptr);
    EXPECT_EQ(ptrTwo.Lock().Get(), ptr.Get());
}
