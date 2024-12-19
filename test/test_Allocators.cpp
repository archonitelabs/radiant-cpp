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

#include "gtest/gtest.h"

#include "radiant/Memory.h"

#include "TestAlloc.h"

struct TrackCtorDtor
{
    static int CtorCount;
    static int DtorCount;
    static int OtherCtorCount;

    TrackCtorDtor()
    {
        ++CtorCount;
    }

    explicit TrackCtorDtor(uint64_t val)
        : m_dummy_data(val)
    {
        ++OtherCtorCount;
    }

    ~TrackCtorDtor()
    {
        ++DtorCount;
    }

    static void Reset()
    {
        CtorCount = 0;
        DtorCount = 0;
        OtherCtorCount = 0;
    }

    uint64_t m_dummy_data;
};

int TrackCtorDtor::CtorCount = 0;
int TrackCtorDtor::DtorCount = 0;
int TrackCtorDtor::OtherCtorCount = 0;

TEST(AllocatorTests, MinimalAlloc)
{
    radtest::Mallocator mal;
    using allt = rad::AllocTraits<radtest::Mallocator>;
    RAD_S_ASSERT(allt::PropagateOnCopy == false);
    RAD_S_ASSERT(allt::PropagateOnMoveAssignment == false);
    RAD_S_ASSERT(allt::PropagateOnSwap == false);
    RAD_S_ASSERT(allt::IsAlwaysEqual == true);
    RAD_S_ASSERT(allt::HasConstructAndDestroy == false);
    RAD_S_ASSERT(allt::HasTypedAllocations == false);

    {
        void* vmem = allt::AllocBytes(mal, 1);
        EXPECT_NE(vmem, nullptr);
        allt::FreeBytes(mal, vmem, 1);

        allt::FreeBytes(mal, nullptr, 1);
    }

    {
        static constexpr size_t kBiggest = ~size_t(0);
        static constexpr size_t kEltsToAttempt = kBiggest / 2;

        uint64_t* too_much = allt::Alloc<uint64_t>(mal, kEltsToAttempt);
        EXPECT_EQ(too_much, nullptr);
        allt::Free(mal, too_much, kEltsToAttempt);

        uint64_t* unull = nullptr;
        allt::Free(mal, unull, kEltsToAttempt);
    }

    {
        TrackCtorDtor::Reset();
        TrackCtorDtor* tmem = allt::Alloc<TrackCtorDtor>(mal, 4);
        EXPECT_NE(tmem, nullptr);
        EXPECT_EQ(TrackCtorDtor::CtorCount, 0);
        EXPECT_EQ(TrackCtorDtor::DtorCount, 0);
        EXPECT_EQ(TrackCtorDtor::OtherCtorCount, 0);

        // check alignment
        uintptr_t mask = alignof(TrackCtorDtor) - 1;
        uintptr_t tmem_num = reinterpret_cast<uintptr_t>(tmem);
        EXPECT_EQ((tmem_num & mask), 0u);

        TrackCtorDtor* constructed = allt::Construct(mal, tmem);
        EXPECT_NE(constructed, nullptr);
        EXPECT_EQ(TrackCtorDtor::CtorCount, 1);
        EXPECT_EQ(TrackCtorDtor::DtorCount, 0);
        EXPECT_EQ(TrackCtorDtor::OtherCtorCount, 0);

        allt::Destroy(mal, constructed);
        EXPECT_EQ(TrackCtorDtor::CtorCount, 1);
        EXPECT_EQ(TrackCtorDtor::DtorCount, 1);
        EXPECT_EQ(TrackCtorDtor::OtherCtorCount, 0);

        TrackCtorDtor* other_constructed = allt::Construct(mal, tmem, 42u);
        EXPECT_NE(other_constructed, nullptr);
        EXPECT_EQ(TrackCtorDtor::CtorCount, 1);
        EXPECT_EQ(TrackCtorDtor::DtorCount, 1);
        EXPECT_EQ(TrackCtorDtor::OtherCtorCount, 1);

        allt::Destroy(mal, other_constructed);
        EXPECT_EQ(TrackCtorDtor::CtorCount, 1);
        EXPECT_EQ(TrackCtorDtor::DtorCount, 2);
        EXPECT_EQ(TrackCtorDtor::OtherCtorCount, 1);

        allt::Free(mal, tmem, 4);
        EXPECT_EQ(TrackCtorDtor::CtorCount, 1);
        EXPECT_EQ(TrackCtorDtor::DtorCount, 2);
        EXPECT_EQ(TrackCtorDtor::OtherCtorCount, 1);
    }

    {
        radtest::Mallocator mal2;
        EXPECT_TRUE(allt::Equal(mal, mal2));
        EXPECT_TRUE(allt::Equal(mal2, mal));
        EXPECT_TRUE(allt::Equal(mal, radtest::Mallocator{}));
    }
}

TEST(AllocatorTests, PropagatingAllocator)
{
    constexpr uint32_t kMainTag = 42;
    radtest::MovingTaggedAllocator tal(kMainTag);
    using allt = rad::AllocTraits<radtest::MovingTaggedAllocator>;
    RAD_S_ASSERT(allt::PropagateOnCopy == true);
    RAD_S_ASSERT(allt::PropagateOnMoveAssignment == true);
    RAD_S_ASSERT(allt::PropagateOnSwap == true);
    RAD_S_ASSERT(allt::IsAlwaysEqual == false);
    RAD_S_ASSERT(allt::HasConstructAndDestroy == false);
    RAD_S_ASSERT(allt::HasTypedAllocations == false);

    {
        void* vmem = allt::AllocBytes(tal, 1);
        EXPECT_NE(vmem, nullptr);
        allt::FreeBytes(tal, vmem, 1);

        allt::FreeBytes(tal, nullptr, 1);
    }

    {
        static constexpr size_t kBiggest = ~size_t(0);
        static constexpr size_t kEltsToAttempt = kBiggest / 2;

        uint64_t* too_much = allt::Alloc<uint64_t>(tal, kEltsToAttempt);
        EXPECT_EQ(too_much, nullptr);
        allt::Free(tal, too_much, kEltsToAttempt);

        uint64_t* unull = nullptr;
        allt::Free(tal, unull, kEltsToAttempt);
    }

    {
        TrackCtorDtor::Reset();
        TrackCtorDtor* tmem = allt::Alloc<TrackCtorDtor>(tal, 4);
        EXPECT_NE(tmem, nullptr);
        EXPECT_EQ(TrackCtorDtor::CtorCount, 0);
        EXPECT_EQ(TrackCtorDtor::DtorCount, 0);
        EXPECT_EQ(TrackCtorDtor::OtherCtorCount, 0);

        // check alignment
        uintptr_t mask = alignof(TrackCtorDtor) - 1;
        uintptr_t tmem_num = reinterpret_cast<uintptr_t>(tmem);
        EXPECT_EQ((tmem_num & mask), 0u);

        TrackCtorDtor* constructed = allt::Construct(tal, tmem);
        EXPECT_NE(constructed, nullptr);
        EXPECT_EQ(TrackCtorDtor::CtorCount, 1);
        EXPECT_EQ(TrackCtorDtor::DtorCount, 0);
        EXPECT_EQ(TrackCtorDtor::OtherCtorCount, 0);

        allt::Destroy(tal, constructed);
        EXPECT_EQ(TrackCtorDtor::CtorCount, 1);
        EXPECT_EQ(TrackCtorDtor::DtorCount, 1);
        EXPECT_EQ(TrackCtorDtor::OtherCtorCount, 0);

        TrackCtorDtor* other_constructed = allt::Construct(tal, tmem, 42u);
        EXPECT_NE(other_constructed, nullptr);
        EXPECT_EQ(TrackCtorDtor::CtorCount, 1);
        EXPECT_EQ(TrackCtorDtor::DtorCount, 1);
        EXPECT_EQ(TrackCtorDtor::OtherCtorCount, 1);

        allt::Destroy(tal, other_constructed);
        EXPECT_EQ(TrackCtorDtor::CtorCount, 1);
        EXPECT_EQ(TrackCtorDtor::DtorCount, 2);
        EXPECT_EQ(TrackCtorDtor::OtherCtorCount, 1);

        allt::Free(tal, tmem, 4);
        EXPECT_EQ(TrackCtorDtor::CtorCount, 1);
        EXPECT_EQ(TrackCtorDtor::DtorCount, 2);
        EXPECT_EQ(TrackCtorDtor::OtherCtorCount, 1);
    }

    {
        radtest::MovingTaggedAllocator tal2 = tal;
        EXPECT_TRUE(allt::Equal(tal, tal2));
        EXPECT_TRUE(allt::Equal(tal2, tal));

        radtest::MovingTaggedAllocator tal3(kMainTag);
        EXPECT_TRUE(allt::Equal(tal, tal3));
        EXPECT_TRUE(allt::Equal(tal3, tal));

        radtest::MovingTaggedAllocator tal4(kMainTag + 1);
        EXPECT_FALSE(allt::Equal(tal, tal4));
        EXPECT_FALSE(allt::Equal(tal4, tal));
    }
}

#if RAD_ENABLE_STD
TEST(AllocatorTests, StdAllocator)
{
    rad::StdAllocator sal;
    using allt = rad::AllocTraits<rad::StdAllocator>;
    RAD_S_ASSERT(allt::PropagateOnCopy == false);
    RAD_S_ASSERT(allt::PropagateOnMoveAssignment == false);
    RAD_S_ASSERT(allt::PropagateOnSwap == false);
    RAD_S_ASSERT(allt::IsAlwaysEqual == true);
    RAD_S_ASSERT(allt::HasConstructAndDestroy == true);
    RAD_S_ASSERT(allt::HasTypedAllocations == true);

    {
        void* vmem = allt::AllocBytes(sal, 1);
        EXPECT_NE(vmem, nullptr);
        allt::FreeBytes(sal, vmem, 1);
    }

    {
        TrackCtorDtor::Reset();
        TrackCtorDtor* tmem = allt::Alloc<TrackCtorDtor>(sal, 4);
        EXPECT_NE(tmem, nullptr);
        EXPECT_EQ(TrackCtorDtor::CtorCount, 0);
        EXPECT_EQ(TrackCtorDtor::DtorCount, 0);
        EXPECT_EQ(TrackCtorDtor::OtherCtorCount, 0);

        // check alignment
        uintptr_t mask = alignof(TrackCtorDtor) - 1;
        uintptr_t tmem_num = reinterpret_cast<uintptr_t>(tmem);
        EXPECT_EQ((tmem_num & mask), 0u);

        TrackCtorDtor* constructed = allt::Construct(sal, tmem);
        EXPECT_NE(constructed, nullptr);
        EXPECT_EQ(TrackCtorDtor::CtorCount, 1);
        EXPECT_EQ(TrackCtorDtor::DtorCount, 0);
        EXPECT_EQ(TrackCtorDtor::OtherCtorCount, 0);

        allt::Destroy(sal, constructed);
        EXPECT_EQ(TrackCtorDtor::CtorCount, 1);
        EXPECT_EQ(TrackCtorDtor::DtorCount, 1);
        EXPECT_EQ(TrackCtorDtor::OtherCtorCount, 0);

        TrackCtorDtor* other_constructed = allt::Construct(sal, tmem, 42);
        EXPECT_NE(other_constructed, nullptr);
        EXPECT_EQ(TrackCtorDtor::CtorCount, 1);
        EXPECT_EQ(TrackCtorDtor::DtorCount, 1);
        EXPECT_EQ(TrackCtorDtor::OtherCtorCount, 1);

        allt::Destroy(sal, other_constructed);
        EXPECT_EQ(TrackCtorDtor::CtorCount, 1);
        EXPECT_EQ(TrackCtorDtor::DtorCount, 2);
        EXPECT_EQ(TrackCtorDtor::OtherCtorCount, 1);

        allt::Free(sal, tmem, 4);
        EXPECT_EQ(TrackCtorDtor::CtorCount, 1);
        EXPECT_EQ(TrackCtorDtor::DtorCount, 2);
        EXPECT_EQ(TrackCtorDtor::OtherCtorCount, 1);
    }

    {
        rad::StdAllocator sal2;
        EXPECT_TRUE(allt::Equal(sal, sal2));
        EXPECT_TRUE(allt::Equal(sal2, sal));
        EXPECT_TRUE(allt::Equal(sal, rad::StdAllocator{}));

        EXPECT_TRUE(sal == sal2);
        EXPECT_FALSE(sal != sal2);
    }
}

#if RAD_CPP20
constexpr bool test_constexpr_StdAllocator()
{
    rad::StdAllocator sal;
    using allt = rad::AllocTraits<rad::StdAllocator>;

    {
        uint64_t* tmem = allt::Alloc<uint64_t>(sal, 4);
        assert(tmem != nullptr);

        uint64_t* constructed = allt::Construct(sal, tmem);
        assert(constructed != nullptr);

        *constructed = 19;

        allt::Destroy(sal, constructed);

        uint64_t* other_constructed = allt::Construct(sal, tmem, 42);
        assert(other_constructed != nullptr);
        assert(*other_constructed == 42);

        allt::Destroy(sal, other_constructed);
        allt::Free(sal, tmem, 4);
    }

    {
        rad::StdAllocator sal2;
        RAD_UNUSED(sal2);
        assert(allt::Equal(sal, sal2));
        assert(allt::Equal(sal2, sal));
        assert(allt::Equal(sal, rad::StdAllocator{}));
    }
    return true;
}

static_assert(test_constexpr_StdAllocator());

#endif // RAD_CPP20 ^^^

#endif // RAD_ENABLE_STD ^^^

struct RadTestExceptionType
{
};

struct ThrowingMallocator : radtest::Mallocator
{
    static void HandleSizeOverflow()
    {
        throw RadTestExceptionType();
    }
};

struct BigType
{
    char buf[1 << 30]; // 1 GB
};

TEST(AllocatorTests, ThrowingAllocator)
{
    ThrowingMallocator tm;
#if RAD_AMD64 || RAD_ARM64
    size_t elt_count = 1ull << 40;
#elif RAD_I386 || RAD_ARM
    size_t elt_count = 4;
#else
#error "Unknown platform"
#endif

    BigType* b = nullptr;

    using AllocatorTraits = rad::AllocTraits<ThrowingMallocator>;

    EXPECT_THROW(b = AllocatorTraits::template Alloc<BigType>(tm, elt_count);
                 , RadTestExceptionType);
    EXPECT_EQ(b, nullptr);
    AllocatorTraits::template Free<BigType>(tm, b, elt_count);
}
