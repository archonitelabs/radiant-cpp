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

#include "gtest/gtest.h"

#include "radiant/Atomic.h"
#include "radiant/Utility.h"

#if defined(RAD_GCC_VERSION) || defined(RAD_CLANG_VERSION)
RAD_S_ASSERT((static_cast<int>(rad::MemoryOrder::Relaxed) == __ATOMIC_RELAXED));
RAD_S_ASSERT((static_cast<int>(rad::MemoryOrder::Consume) == __ATOMIC_CONSUME));
RAD_S_ASSERT((static_cast<int>(rad::MemoryOrder::Acquire) == __ATOMIC_ACQUIRE));
RAD_S_ASSERT((static_cast<int>(rad::MemoryOrder::Release) == __ATOMIC_RELEASE));
RAD_S_ASSERT((static_cast<int>(rad::MemoryOrder::AcqRel) == __ATOMIC_ACQ_REL));
RAD_S_ASSERT((static_cast<int>(rad::MemoryOrder::SeqCst) == __ATOMIC_SEQ_CST));
#endif

namespace atom = rad::detail::atomic;
using Order = rad::MemoryOrder;
template <Order O>
using Tag = typename atom::OrderToTag<O>::Type;
template <typename T>
using SI = atom::SelectIntrinsic<T>;

using Relaxed = atom::RelaxedTag;
using Consume = atom::ConsumeTag;
using Acquire = atom::AcquireTag;
using Release = atom::ReleaseTag;
using AcqRel = atom::AcqRelTag;
using SeqCst = atom::SeqCstTag;

static constexpr rad::Atomic<int> constexprGlobalAtomic(0x11223344);

TEST(AtomicTests, AtomicDefaultCtor)
{
    rad::Atomic<int32_t> val;
    EXPECT_EQ(val.Load(rad::MemOrderRelaxed), 0);
}

TEST(AtomicTests, AtomicCtor)
{
    rad::Atomic<int32_t> val(0x11223344);
    EXPECT_EQ(val.Load(), 0x11223344);
    EXPECT_EQ(constexprGlobalAtomic, val);
}

TEST(AtomicTests, Atomic8StoreLoadSeqCst)
{
    rad::Atomic<int8_t> val;
    val.Store(2, rad::MemOrderSeqCst);
    EXPECT_EQ(val.Load(rad::MemOrderSeqCst), 2);

    val.Store(4);
    EXPECT_EQ(val.Load(), 4);
}

TEST(AtomicTests, Atomic16StoreLoadSeqCst)
{
    rad::Atomic<int16_t> val;
    val.Store(0x0fb0, rad::MemOrderSeqCst);
    EXPECT_EQ(val.Load(rad::MemOrderSeqCst), 0x0fb0);

    val.Store(0x1122);
    EXPECT_EQ(val.Load(), 0x1122);
}

TEST(AtomicTests, Atomic32StoreLoadSeqCst)
{
    rad::Atomic<int32_t> val;
    val.Store(0x0fb0a0c0, rad::MemOrderSeqCst);
    EXPECT_EQ(val.Load(rad::MemOrderSeqCst), 0x0fb0a0c0);

    val.Store(0x11223344);
    EXPECT_EQ(val.Load(), 0x11223344);
}

TEST(AtomicTests, Atomic64StoreLoadSeqCst)
{
    rad::Atomic<int64_t> val;
    val.Store(0x0fb0a0c00a0b0c0d, rad::MemOrderSeqCst);
    EXPECT_EQ(val.Load(rad::MemOrderSeqCst), 0x0fb0a0c00a0b0c0d);

    val.Store(0x1122334455667788);
    EXPECT_EQ(val.Load(), 0x1122334455667788);
}

TEST(AtomicTests, AtomicPtrStoreLoadSeqCst)
{
    rad::Atomic<char*> val;
    char* ptr = rad::Add2Ptr<char>(nullptr, 0x1234);
    val.Store(ptr, rad::MemOrderSeqCst);
    EXPECT_EQ(val.Load(rad::MemOrderSeqCst), ptr);

    val.Store(ptr);
    EXPECT_EQ(val.Load(), ptr);
}

TEST(AtomicTests, Atomic8Exchange)
{
    rad::Atomic<uint8_t> val(2);
    EXPECT_EQ(val.Exchange(8, rad::MemOrderSeqCst), 2);
    EXPECT_EQ(val, 8);

    EXPECT_EQ(val.Exchange(9, rad::MemOrderRelaxed), 8);
    EXPECT_EQ(val, 9);
}

TEST(AtomicTests, Atomic16Exchange)
{
    rad::Atomic<uint16_t> val(0x1122);
    EXPECT_EQ(val.Exchange(0x3344, rad::MemOrderSeqCst), 0x1122);
    EXPECT_EQ(val, 0x3344);
}

TEST(AtomicTests, Atomic32Exchange)
{
    rad::Atomic<uint32_t> val(0x11223344);
    EXPECT_EQ(val.Exchange(0x33445566, rad::MemOrderSeqCst), 0x11223344u);
    EXPECT_EQ(val, 0x33445566u);
}

TEST(AtomicTests, Atomic64Exchange)
{
    rad::Atomic<uint64_t> val(0x1122334455667788);
    EXPECT_EQ(val.Exchange(0xaabbccddeeff0011, rad::MemOrderSeqCst),
              0x1122334455667788ull);
    EXPECT_EQ(val, 0xaabbccddeeff0011ull);
}

TEST(AtomicTests, AtomicPtrExchange)
{
    char* ptr = rad::Add2Ptr<char>(nullptr, 0x11223344);
    char* ptr2 = rad::Add2Ptr<char>(nullptr, 0xaabbccdd);
    rad::Atomic<char*> val(ptr);
    EXPECT_EQ(val.Exchange(ptr2, rad::MemOrderSeqCst), ptr);
    EXPECT_EQ(val, ptr2);
}

TEST(AtomicTests, Atomic8CasWeak)
{
    rad::Atomic<int8_t> val(2);
    int8_t expected = 3;
    bool ret = val.CompareExchangeWeak(expected, 4, rad::MemOrderRelease);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 2);
    EXPECT_EQ(val, 2);

    ret = val.CompareExchangeWeak(expected,
                                  4,
                                  rad::MemOrderAcquire,
                                  rad::MemOrderRelaxed);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 2);
    EXPECT_EQ(val, 4);
}

TEST(AtomicTests, Atomic8CasStrong)
{
    rad::Atomic<int8_t> val(2);
    int8_t expected = 3;
    bool ret = val.CompareExchangeStrong(expected, 4, rad::MemOrderRelease);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 2);
    EXPECT_EQ(val, 2);

    ret = val.CompareExchangeStrong(expected,
                                    4,
                                    rad::MemOrderAcquire,
                                    rad::MemOrderRelaxed);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 2);
    EXPECT_EQ(val, 4);
}

TEST(AtomicTests, Atomic16CasWeak)
{
    rad::Atomic<int16_t> val(0x1122);
    int16_t expected = 3;
    bool ret = val.CompareExchangeWeak(expected, 0x3344, rad::MemOrderRelease);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x1122);
    EXPECT_EQ(val, 0x1122);

    ret = val.CompareExchangeWeak(expected,
                                  0x3344,
                                  rad::MemOrderAcquire,
                                  rad::MemOrderRelaxed);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x1122);
    EXPECT_EQ(val, 0x3344);
}

TEST(AtomicTests, Atomic16CasStrong)
{
    rad::Atomic<int16_t> val(0x1122);
    int16_t expected = 3;
    bool ret =
        val.CompareExchangeStrong(expected, 0x3344, rad::MemOrderRelease);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x1122);
    EXPECT_EQ(val, 0x1122);

    ret = val.CompareExchangeStrong(expected,
                                    0x3344,
                                    rad::MemOrderAcquire,
                                    rad::MemOrderRelaxed);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x1122);
    EXPECT_EQ(val, 0x3344);
}

TEST(AtomicTests, Atomic32CasWeak)
{
    rad::Atomic<int32_t> val(0x11223344);
    int32_t expected = 3;
    bool ret =
        val.CompareExchangeWeak(expected, 0x33445566, rad::MemOrderRelease);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x11223344);
    EXPECT_EQ(val, 0x11223344);

    ret = val.CompareExchangeWeak(expected,
                                  0x33445566,
                                  rad::MemOrderAcquire,
                                  rad::MemOrderRelaxed);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x11223344);
    EXPECT_EQ(val, 0x33445566);
}

TEST(AtomicTests, Atomic32CasStrong)
{
    rad::Atomic<int32_t> val(0x11223344);
    int32_t expected = 3;
    bool ret =
        val.CompareExchangeStrong(expected, 0x33445566, rad::MemOrderRelease);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x11223344);
    EXPECT_EQ(val, 0x11223344);

    ret = val.CompareExchangeStrong(expected,
                                    0x33445566,
                                    rad::MemOrderAcquire,
                                    rad::MemOrderRelaxed);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x11223344);
    EXPECT_EQ(val, 0x33445566);
}

TEST(AtomicTests, Atomic64CasWeak)
{
    rad::Atomic<int64_t> val(0x1122334455667788);
    int64_t expected = 3;
    bool ret = val.CompareExchangeWeak(expected,
                                       0x33445566778899aa,
                                       rad::MemOrderRelease);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x1122334455667788);
    EXPECT_EQ(val, 0x1122334455667788);

    ret = val.CompareExchangeWeak(expected,
                                  0x33445566778899aa,
                                  rad::MemOrderAcquire,
                                  rad::MemOrderRelaxed);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x1122334455667788);
    EXPECT_EQ(val, 0x33445566778899aa);
}

TEST(AtomicTests, Atomic64CasStrong)
{
    rad::Atomic<int64_t> val(0x1122334455667788);
    int64_t expected = 3;
    bool ret = val.CompareExchangeStrong(expected,
                                         0x33445566778899aa,
                                         rad::MemOrderRelease);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x1122334455667788);
    EXPECT_EQ(val, 0x1122334455667788);

    ret = val.CompareExchangeStrong(expected,
                                    0x33445566778899aa,
                                    rad::MemOrderAcquire,
                                    rad::MemOrderRelaxed);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x1122334455667788);
    EXPECT_EQ(val, 0x33445566778899aa);
}

TEST(AtomicTests, AtomicPtrCasWeak)
{
    char* ptr = rad::Add2Ptr<char>(nullptr, 0x11223344);
    char* ptr2 = rad::Add2Ptr<char>(nullptr, 0xaabbccdd);
    rad::Atomic<char*> val(ptr);
    char* expected = nullptr;
    bool ret = val.CompareExchangeWeak(expected, ptr2, rad::MemOrderRelease);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, ptr);
    EXPECT_EQ(val, ptr);

    ret = val.CompareExchangeWeak(expected,
                                  ptr2,
                                  rad::MemOrderAcquire,
                                  rad::MemOrderRelaxed);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, ptr);
    EXPECT_EQ(val, ptr2);
}

TEST(AtomicTests, AtomicPtrCasStrong)
{
    char* ptr = rad::Add2Ptr<char>(nullptr, 0x11223344);
    char* ptr2 = rad::Add2Ptr<char>(nullptr, 0xaabbccdd);
    rad::Atomic<char*> val(ptr);
    char* expected = nullptr;
    bool ret = val.CompareExchangeStrong(expected, ptr2, rad::MemOrderRelease);
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, ptr);
    EXPECT_EQ(val, ptr);

    ret = val.CompareExchangeStrong(expected,
                                    ptr2,
                                    rad::MemOrderAcquire,
                                    rad::MemOrderRelaxed);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, ptr);
    EXPECT_EQ(val, ptr2);
}

TEST(AtomicTests, Atomic8FetchAdd)
{
    rad::Atomic<int8_t> val(2);
    EXPECT_EQ(val.FetchAdd(4, rad::MemOrderSeqCst), 2);
    EXPECT_EQ(val, 6);

    EXPECT_EQ(val.FetchAdd(2), 6);
    EXPECT_EQ(val, 8);
}

TEST(AtomicTests, Atomic16FetchAdd)
{
    rad::Atomic<int16_t> val(0x101);
    EXPECT_EQ(val.FetchAdd(0x101, rad::MemOrderSeqCst), 0x101);
    EXPECT_EQ(val, 0x202);
}

TEST(AtomicTests, Atomic32FetchAdd)
{
    rad::Atomic<int32_t> val(0x1010101);
    EXPECT_EQ(val.FetchAdd(0x1010101), 0x1010101);
    EXPECT_EQ(val, 0x2020202);
}

TEST(AtomicTests, Atomic64FetchAdd)
{
    rad::Atomic<int64_t> val(0x101010101010101);
    EXPECT_EQ(val.FetchAdd(0x101010101010101), 0x101010101010101);
    EXPECT_EQ(val, 0x202020202020202);
}

TEST(AtomicTests, AtomicPtrFetchAdd)
{
    char* ptr = rad::Add2Ptr<char>(nullptr, 0x10101010);
    char* ptr2 = rad::Add2Ptr<char>(nullptr, 0x20202020);
    rad::Atomic<char*> val(ptr);
    EXPECT_EQ(val.FetchAdd(0x10101010), ptr);
    EXPECT_EQ(val, ptr2);
}

TEST(AtomicTests, Atomic8FetchSub)
{
    rad::Atomic<int8_t> val(6);
    EXPECT_EQ(val.FetchSub(4, rad::MemOrderSeqCst), 6);
    EXPECT_EQ(val, 2);
}

TEST(AtomicTests, Atomic16FetchSub)
{
    rad::Atomic<int16_t> val(0x202);
    EXPECT_EQ(val.FetchSub(0x101), 0x202);
    EXPECT_EQ(val, 0x101);
}

TEST(AtomicTests, Atomic32FetchSub)
{
    rad::Atomic<int32_t> val(0x2020202);
    EXPECT_EQ(val.FetchSub(0x1010101), 0x2020202);
    EXPECT_EQ(val, 0x1010101);
}

TEST(AtomicTests, Atomic64FetchSub)
{
    rad::Atomic<int64_t> val(0x202020202020202);
    EXPECT_EQ(val.FetchSub(0x101010101010101), 0x202020202020202);
    EXPECT_EQ(val, 0x101010101010101);
}

TEST(AtomicTests, AtomicPtrFetchSub)
{
    char* ptr = rad::Add2Ptr<char>(nullptr, 0x20202020);
    char* ptr2 = rad::Add2Ptr<char>(nullptr, 0x10101010);
    rad::Atomic<char*> val(ptr);
    EXPECT_EQ(val.FetchSub(0x10101010), ptr);
    EXPECT_EQ(val, ptr2);
}

TEST(AtomicTests, Atomic8FetchAnd)
{
    rad::Atomic<int8_t> val(6);
    EXPECT_EQ(val.FetchAnd(3, rad::MemOrderSeqCst), 6);
    EXPECT_EQ(val, 2);
}

TEST(AtomicTests, Atomic16FetchAnd)
{
    rad::Atomic<int16_t> val(0x606);
    EXPECT_EQ(val.FetchAnd(0x303), 0x606);
    EXPECT_EQ(val, 0x202);
}

TEST(AtomicTests, Atomic32FetchAnd)
{
    rad::Atomic<int32_t> val(0x6060606);
    EXPECT_EQ(val.FetchAnd(0x3030303), 0x6060606);
    EXPECT_EQ(val, 0x2020202);
}

TEST(AtomicTests, Atomic64FetchAnd)
{
    rad::Atomic<int64_t> val(0x606060606060606);
    EXPECT_EQ(val.FetchAnd(0x303030303030303), 0x606060606060606);
    EXPECT_EQ(val, 0x202020202020202);
}

TEST(AtomicTests, Atomic8FetchOr)
{
    rad::Atomic<int8_t> val(6);
    EXPECT_EQ(val.FetchOr(3, rad::MemOrderSeqCst), 6);
    EXPECT_EQ(val, 7);
}

TEST(AtomicTests, Atomic16FetchOr)
{
    rad::Atomic<int16_t> val(0x606);
    EXPECT_EQ(val.FetchOr(0x303), 0x606);
    EXPECT_EQ(val, 0x707);
}

TEST(AtomicTests, Atomic32FetchOr)
{
    rad::Atomic<int32_t> val(0x6060606);
    EXPECT_EQ(val.FetchOr(0x3030303), 0x6060606);
    EXPECT_EQ(val, 0x7070707);
}

TEST(AtomicTests, Atomic64FetchOr)
{
    rad::Atomic<int64_t> val(0x606060606060606);
    EXPECT_EQ(val.FetchOr(0x303030303030303), 0x606060606060606);
    EXPECT_EQ(val, 0x707070707070707);
}

TEST(AtomicTests, Atomic8FetchXor)
{
    rad::Atomic<int8_t> val(6);
    EXPECT_EQ(val.FetchXor(3, rad::MemOrderSeqCst), 6);
    EXPECT_EQ(val, 5);
}

TEST(AtomicTests, Atomic16FetchXor)
{
    rad::Atomic<int16_t> val(0x606);
    EXPECT_EQ(val.FetchXor(0x303), 0x606);
    EXPECT_EQ(val, 0x505);
}

TEST(AtomicTests, Atomic32FetchXor)
{
    rad::Atomic<int32_t> val(0x6060606);
    EXPECT_EQ(val.FetchXor(0x3030303), 0x6060606);
    EXPECT_EQ(val, 0x5050505);
}

TEST(AtomicTests, Atomic64FetchXor)
{
    rad::Atomic<int64_t> val(0x606060606060606);
    EXPECT_EQ(val.FetchXor(0x303030303030303), 0x606060606060606);
    EXPECT_EQ(val, 0x505050505050505);
}

TEST(AtomicTests, Atomic8Assign)
{
    rad::Atomic<int8_t> val;
    EXPECT_EQ(val = 6, 6);
    EXPECT_EQ(val, 6);
}

TEST(AtomicTests, Atomic16Assign)
{
    rad::Atomic<int16_t> val;
    EXPECT_EQ(val = 0x1122, 0x1122);
    EXPECT_EQ(val, 0x1122);
}

TEST(AtomicTests, Atomic32Assign)
{
    rad::Atomic<int32_t> val;
    EXPECT_EQ(val = 0x11223344, 0x11223344);
    EXPECT_EQ(val, 0x11223344);
}

TEST(AtomicTests, Atomic64Assign)
{
    rad::Atomic<int64_t> val;
    EXPECT_EQ(val = 0x1122334455667788, 0x1122334455667788);
    EXPECT_EQ(val, 0x1122334455667788);
}

TEST(AtomicTests, AtomicPtrAssign)
{
    char* ptr = rad::Add2Ptr<char>(nullptr, 0x11223344);
    rad::Atomic<char*> val;
    EXPECT_EQ(val = ptr, ptr);
    EXPECT_EQ(val, ptr);
}

TEST(AtomicTests, Atomic8PreInc)
{
    rad::Atomic<int8_t> val(1);
    EXPECT_EQ(++val, 2);
    EXPECT_EQ(val, 2);
}

TEST(AtomicTests, Atomic16PreInc)
{
    rad::Atomic<int16_t> val(0x1fff);
    EXPECT_EQ(++val, 0x2000);
    EXPECT_EQ(val, 0x2000);
}

TEST(AtomicTests, Atomic32PreInc)
{
    rad::Atomic<int32_t> val(0x1fffffff);
    EXPECT_EQ(++val, 0x20000000);
    EXPECT_EQ(val, 0x20000000);
}

TEST(AtomicTests, Atomic64PreInc)
{
    rad::Atomic<int64_t> val(0x1fffffffffffffff);
    EXPECT_EQ(++val, 0x2000000000000000);
    EXPECT_EQ(val, 0x2000000000000000);
}

TEST(AtomicTests, AtomicPtrPreInc)
{
    char* ptr = rad::Add2Ptr<char>(nullptr, 0x1fffffff);
    char* ptr2 = rad::Add2Ptr<char>(nullptr, 0x20000000);
    rad::Atomic<char*> val(ptr);
    EXPECT_EQ(++val, ptr2);
    EXPECT_EQ(val, ptr2);
}

TEST(AtomicTests, Atomic8PostInc)
{
    rad::Atomic<int8_t> val(1);
    EXPECT_EQ(val++, 1);
    EXPECT_EQ(val, 2);
}

TEST(AtomicTests, Atomic16PostInc)
{
    rad::Atomic<int16_t> val(0x1fff);
    EXPECT_EQ(val++, 0x1fff);
    EXPECT_EQ(val, 0x2000);
}

TEST(AtomicTests, Atomic32PostInc)
{
    rad::Atomic<int32_t> val(0x1fffffff);
    EXPECT_EQ(val++, 0x1fffffff);
    EXPECT_EQ(val, 0x20000000);
}

TEST(AtomicTests, Atomic64PostInc)
{
    rad::Atomic<int64_t> val(0x1fffffffffffffff);
    EXPECT_EQ(val++, 0x1fffffffffffffff);
    EXPECT_EQ(val, 0x2000000000000000);
}

TEST(AtomicTests, AtomicPtrPostInc)
{
    char* ptr = rad::Add2Ptr<char>(nullptr, 0x1fffffff);
    char* ptr2 = rad::Add2Ptr<char>(nullptr, 0x20000000);
    rad::Atomic<char*> val(ptr);
    EXPECT_EQ(val++, ptr);
    EXPECT_EQ(val, ptr2);
}

TEST(AtomicTests, Atomic8PreDec)
{
    rad::Atomic<int8_t> val(2);
    EXPECT_EQ(--val, 1);
    EXPECT_EQ(val, 1);
}

TEST(AtomicTests, Atomic16PreDec)
{
    rad::Atomic<int16_t> val(0x2000);
    EXPECT_EQ(--val, 0x1fff);
    EXPECT_EQ(val, 0x1fff);
}

TEST(AtomicTests, Atomic32PreDec)
{
    rad::Atomic<int32_t> val(0x20000000);
    EXPECT_EQ(--val, 0x1fffffff);
    EXPECT_EQ(val, 0x1fffffff);
}

TEST(AtomicTests, Atomic64PreDec)
{
    rad::Atomic<int64_t> val(0x2000000000000000);
    EXPECT_EQ(--val, 0x1fffffffffffffff);
    EXPECT_EQ(val, 0x1fffffffffffffff);
}

TEST(AtomicTests, AtomicPtrPreDec)
{
    char* ptr = rad::Add2Ptr<char>(nullptr, 0x20000000);
    char* ptr2 = rad::Add2Ptr<char>(nullptr, 0x1fffffff);
    rad::Atomic<char*> val(ptr);
    EXPECT_EQ(--val, ptr2);
    EXPECT_EQ(val, ptr2);
}

TEST(AtomicTests, Atomic8PostDec)
{
    rad::Atomic<int8_t> val(2);
    EXPECT_EQ(val--, 2);
    EXPECT_EQ(val, 1);
}

TEST(AtomicTests, Atomic16PostDec)
{
    rad::Atomic<int16_t> val(0x2000);
    EXPECT_EQ(val--, 0x2000);
    EXPECT_EQ(val, 0x1fff);
}

TEST(AtomicTests, Atomic32PostDec)
{
    rad::Atomic<int32_t> val(0x20000000);
    EXPECT_EQ(val--, 0x20000000);
    EXPECT_EQ(val, 0x1fffffff);
}

TEST(AtomicTests, Atomic64PostDec)
{
    rad::Atomic<int64_t> val(0x2000000000000000);
    EXPECT_EQ(val--, 0x2000000000000000);
    EXPECT_EQ(val, 0x1fffffffffffffff);
}

TEST(AtomicTests, AtomicPtrPostDec)
{
    char* ptr = rad::Add2Ptr<char>(nullptr, 0x20000000);
    char* ptr2 = rad::Add2Ptr<char>(nullptr, 0x1fffffff);
    rad::Atomic<char*> val(ptr);
    EXPECT_EQ(val--, ptr);
    EXPECT_EQ(val, ptr2);
}

TEST(AtomicTests, Atomic8AddAssign)
{
    rad::Atomic<int8_t> val(2);
    EXPECT_EQ(val += 4, 6);
    EXPECT_EQ(val, 6);
}

TEST(AtomicTests, Atomic16AddAssign)
{
    rad::Atomic<int16_t> val(0x1fff);
    EXPECT_EQ(val += 4, 0x2003);
    EXPECT_EQ(val, 0x2003);
}

TEST(AtomicTests, Atomic32AddAssign)
{
    rad::Atomic<int32_t> val(0x1fffffff);
    EXPECT_EQ(val += 4, 0x20000003);
    EXPECT_EQ(val, 0x20000003);
}

TEST(AtomicTests, Atomic64AddAssign)
{
    rad::Atomic<int64_t> val(0x1fffffffffffffff);
    EXPECT_EQ(val += 4, 0x2000000000000003);
    EXPECT_EQ(val, 0x2000000000000003);
}

TEST(AtomicTests, AtomicPtrAddAssign)
{
    char* ptr = rad::Add2Ptr<char>(nullptr, 0x1fffffff);
    char* ptr2 = rad::Add2Ptr<char>(nullptr, 0x20000003);
    rad::Atomic<char*> val(ptr);
    EXPECT_EQ(val += 4, ptr2);
    EXPECT_EQ(val, ptr2);
}

TEST(AtomicTests, Atomic8SubAssign)
{
    rad::Atomic<int8_t> val(6);
    EXPECT_EQ(val -= 4, 2);
    EXPECT_EQ(val, 2);
}

TEST(AtomicTests, Atomic16SubAssign)
{
    rad::Atomic<int16_t> val(0x2003);
    EXPECT_EQ(val -= 4, 0x1fff);
    EXPECT_EQ(val, 0x1fff);
}

TEST(AtomicTests, Atomic32SubAssign)
{
    rad::Atomic<int32_t> val(0x20000003);
    EXPECT_EQ(val -= 4, 0x1fffffff);
    EXPECT_EQ(val, 0x1fffffff);
}

TEST(AtomicTests, Atomic64SubAssign)
{
    rad::Atomic<int64_t> val(0x2000000000000003);
    EXPECT_EQ(val -= 4, 0x1fffffffffffffff);
    EXPECT_EQ(val, 0x1fffffffffffffff);
}

TEST(AtomicTests, AtomicPtrSubAssign)
{
    char* ptr = rad::Add2Ptr<char>(nullptr, 0x20000003);
    char* ptr2 = rad::Add2Ptr<char>(nullptr, 0x1fffffff);
    rad::Atomic<char*> val(ptr);
    EXPECT_EQ(val -= 4, ptr2);
    EXPECT_EQ(val, ptr2);
}

TEST(AtomicTests, Atomic8AndAssign)
{
    rad::Atomic<int8_t> val(6);
    EXPECT_EQ(val &= 3, 2);
    EXPECT_EQ(val, 2);
}

TEST(AtomicTests, Atomic16AndAssign)
{
    rad::Atomic<int16_t> val(0x606);
    EXPECT_EQ(val &= 0x303, 0x202);
    EXPECT_EQ(val, 0x202);
}

TEST(AtomicTests, Atomic32AndAssign)
{
    rad::Atomic<int32_t> val(0x6060606);
    EXPECT_EQ(val &= 0x3030303, 0x2020202);
    EXPECT_EQ(val, 0x2020202);
}

TEST(AtomicTests, Atomic64AndAssign)
{
    rad::Atomic<int64_t> val(0x606060606060606);
    EXPECT_EQ(val &= 0x303030303030303, 0x202020202020202);
    EXPECT_EQ(val, 0x202020202020202);
}

TEST(AtomicTests, Atomic8OrAssign)
{
    rad::Atomic<int8_t> val(6);
    EXPECT_EQ(val |= 3, 7);
    EXPECT_EQ(val, 7);
}

TEST(AtomicTests, Atomic16OrAssign)
{
    rad::Atomic<int16_t> val(0x606);
    EXPECT_EQ(val |= 0x303, 0x707);
    EXPECT_EQ(val, 0x707);
}

TEST(AtomicTests, Atomic32OrAssign)
{
    rad::Atomic<int32_t> val(0x6060606);
    EXPECT_EQ(val |= 0x3030303, 0x7070707);
    EXPECT_EQ(val, 0x7070707);
}

TEST(AtomicTests, Atomic64OrAssign)
{
    rad::Atomic<int64_t> val(0x606060606060606);
    EXPECT_EQ(val |= 0x303030303030303, 0x707070707070707);
    EXPECT_EQ(val, 0x707070707070707);
}

TEST(AtomicTests, Atomic8XorAssign)
{
    rad::Atomic<int8_t> val(6);
    EXPECT_EQ(val ^= 3, 5);
    EXPECT_EQ(val, 5);
}

TEST(AtomicTests, Atomic16XorAssign)
{
    rad::Atomic<int16_t> val(0x606);
    EXPECT_EQ(val ^= 0x303, 0x505);
    EXPECT_EQ(val, 0x505);
}

TEST(AtomicTests, Atomic32XorAssign)
{
    rad::Atomic<int32_t> val(0x6060606);
    EXPECT_EQ(val ^= 0x3030303, 0x5050505);
    EXPECT_EQ(val, 0x5050505);
}

TEST(AtomicTests, Atomic64XorAssign)
{
    rad::Atomic<int64_t> val(0x606060606060606);
    EXPECT_EQ(val ^= 0x303030303030303, 0x505050505050505);
    EXPECT_EQ(val, 0x505050505050505);
}

TEST(AtomicTests, IntrinSelect8LoadSeqCst)
{
    int8_t value = 0x0f;
    EXPECT_EQ(value, SI<int8_t>::Load(value, SeqCst()));
}

TEST(AtomicTests, IntrinSelect8LoadRelaxed)
{
    int8_t value = 0x0f;
    EXPECT_EQ(value, SI<int8_t>::Load(value, Relaxed()));
}

TEST(AtomicTests, IntrinSelect16LoadSeqCst)
{
    int16_t value = 0x0fb0;
    EXPECT_EQ(value, SI<int16_t>::Load(value, SeqCst()));
}

TEST(AtomicTests, IntrinSelect16LoadRelaxed)
{
    int16_t value = 0x0fb0;
    EXPECT_EQ(value, SI<int16_t>::Load(value, Relaxed()));
}

TEST(AtomicTests, IntrinSelect32LoadSeqCst)
{
    uint32_t value = 0xf0b0f0c0;
    EXPECT_EQ(value, SI<uint32_t>::Load(value, SeqCst()));
}

TEST(AtomicTests, IntrinSelect32LoadRelaxed)
{
    uint32_t value = 0xf0b0f0c0;
    EXPECT_EQ(value, SI<uint32_t>::Load(value, Relaxed()));
}

TEST(AtomicTests, IntrinSelect64LoadSeqCst)
{
    uint64_t value = 0xf0b0f0f0c0f0f0f0;
    EXPECT_EQ(value, SI<uint64_t>::Load(value, SeqCst()));
}

TEST(AtomicTests, IntrinSelect64LoadRelaxed)
{
    uint64_t value = 0xf0b0f0f0c0f0f0f0;
    EXPECT_EQ(value, SI<uint64_t>::Load(value, Relaxed()));
}

TEST(AtomicTests, IntrinSelect8StoreSeqCst)
{
    int8_t value = 0x0f;
    SI<int8_t>::Store(value, 8, SeqCst());
    EXPECT_EQ(value, 8);
}

TEST(AtomicTests, IntrinSelect8StoreRelaxed)
{
    int8_t value = 0x0f;
    SI<int8_t>::Store(value, 8, Relaxed());
    EXPECT_EQ(value, 8);
}

TEST(AtomicTests, IntrinSelect8StoreRelease)
{
    int8_t value = 0x0f;
    SI<int8_t>::Store(value, 8, Release());
    EXPECT_EQ(value, 8);
}

TEST(AtomicTests, IntrinSelect16StoreSeqCst)
{
    int16_t value = 0x0ff0;
    SI<int16_t>::Store(value, 0x0cb0, SeqCst());
    EXPECT_EQ(value, 0x0cb0);
}

TEST(AtomicTests, IntrinSelect16StoreRelaxed)
{
    int16_t value = 0x0ff0;
    SI<int16_t>::Store(value, 0x0cb0, Relaxed());
    EXPECT_EQ(value, 0x0cb0);
}

TEST(AtomicTests, IntrinSelect16StoreRelease)
{
    int16_t value = 0x0ff0;
    SI<int16_t>::Store(value, 0x0cb0, Release());
    EXPECT_EQ(value, 0x0cb0);
}

TEST(AtomicTests, IntrinSelect32StoreSeqCst)
{
    int32_t value = 0x0ff0f0b0;
    SI<int32_t>::Store(value, 0x0cb0e0b0, SeqCst());
    EXPECT_EQ(value, 0x0cb0e0b0);
}

TEST(AtomicTests, IntrinSelect32StoreRelaxed)
{
    int32_t value = 0x0ff0f0b0;
    SI<int32_t>::Store(value, 0x0cb0e0b0, Relaxed());
    EXPECT_EQ(value, 0x0cb0e0b0);
}

TEST(AtomicTests, IntrinSelect32StoreRelease)
{
    int32_t value = 0x0ff0f0b0;
    SI<int32_t>::Store(value, 0x0cb0e0b0, Release());
    EXPECT_EQ(value, 0x0cb0e0b0);
}

TEST(AtomicTests, IntrinSelect64StoreSeqCst)
{
    int64_t value = 0x0ff0f0b0c0b0c0b0;
    SI<int64_t>::Store(value, 0x0cb0f0f00a0b0c0d, SeqCst());
    EXPECT_EQ(value, 0x0cb0f0f00a0b0c0d);
}

TEST(AtomicTests, IntrinSelect64StoreRelaxed)
{
    int64_t value = 0x0ff0f0b0c0b0c0b0;
    SI<int64_t>::Store(value, 0x0cb0f0f00a0b0c0d, Relaxed());
    EXPECT_EQ(value, 0x0cb0f0f00a0b0c0d);
}

TEST(AtomicTests, IntrinSelect64StoreRelease)
{
    int64_t value = 0x0ff0f0b0c0b0c0b0;
    SI<int64_t>::Store(value, 0x0cb0f0f00a0b0c0d, Release());
    EXPECT_EQ(value, 0x0cb0f0f00a0b0c0d);
}

TEST(AtomicTests, IntrinSelect8ExchangeRelaxed)
{
    int8_t value = 8;
    int8_t old = SI<int8_t>::Exchange(value, 0x0f, Relaxed());
    EXPECT_EQ(old, 8);
    EXPECT_EQ(value, 0x0f);
}

TEST(AtomicTests, IntrinSelect8ExchangeConsume)
{
    int8_t value = 8;
    int8_t old = SI<int8_t>::Exchange(value, 0x0f, Consume());
    EXPECT_EQ(old, 8);
    EXPECT_EQ(value, 0x0f);
}

TEST(AtomicTests, IntrinSelect8ExchangeAcquire)
{
    int8_t value = 8;
    int8_t old = SI<int8_t>::Exchange(value, 0x0f, Acquire());
    EXPECT_EQ(old, 8);
    EXPECT_EQ(value, 0x0f);
}

TEST(AtomicTests, IntrinSelect8ExchangeRelease)
{
    int8_t value = 8;
    int8_t old = SI<int8_t>::Exchange(value, 0x0f, Release());
    EXPECT_EQ(old, 8);
    EXPECT_EQ(value, 0x0f);
}

TEST(AtomicTests, IntrinSelect8ExchangeAcqRel)
{
    int8_t value = 8;
    int8_t old = SI<int8_t>::Exchange(value, 0x0f, AcqRel());
    EXPECT_EQ(old, 8);
    EXPECT_EQ(value, 0x0f);
}

TEST(AtomicTests, IntrinSelect8ExchangeSeqCst)
{
    int8_t value = 8;
    int8_t old = SI<int8_t>::Exchange(value, 0x0f, SeqCst());
    EXPECT_EQ(old, 8);
    EXPECT_EQ(value, 0x0f);
}

TEST(AtomicTests, IntrinSelect16ExchangeRelaxed)
{
    int16_t value = 16;
    SI<int16_t>::Exchange(value, 0x0ff0, Relaxed());
    EXPECT_EQ(value, 0x0ff0);
}

TEST(AtomicTests, IntrinSelect16ExchangeConsume)
{
    int16_t value = 16;
    SI<int16_t>::Exchange(value, 0x0ff0, Consume());
    EXPECT_EQ(value, 0x0ff0);
}

TEST(AtomicTests, IntrinSelect16ExchangeAcquire)
{
    int16_t value = 16;
    SI<int16_t>::Exchange(value, 0x0ff0, Acquire());
    EXPECT_EQ(value, 0x0ff0);
}

TEST(AtomicTests, IntrinSelect16ExchangeRelease)
{
    int16_t value = 16;
    SI<int16_t>::Exchange(value, 0x0ff0, Release());
    EXPECT_EQ(value, 0x0ff0);
}

TEST(AtomicTests, IntrinSelect16ExchangeAcqRel)
{
    int16_t value = 16;
    SI<int16_t>::Exchange(value, 0x0ff0, AcqRel());
    EXPECT_EQ(value, 0x0ff0);
}

TEST(AtomicTests, IntrinSelect16ExchangeSeqCst)
{
    int16_t value = 16;
    SI<int16_t>::Exchange(value, 0x0ff0, SeqCst());
    EXPECT_EQ(value, 0x0ff0);
}

TEST(AtomicTests, IntrinSelect32ExchangeRelaxed)
{
    int32_t value = 32;
    SI<int32_t>::Exchange(value, 0x0ff0e0d0, Relaxed());
    EXPECT_EQ(value, 0x0ff0e0d0);
}

TEST(AtomicTests, IntrinSelect32ExchangeConsume)
{
    int32_t value = 32;
    SI<int32_t>::Exchange(value, 0x0ff0e0d0, Consume());
    EXPECT_EQ(value, 0x0ff0e0d0);
}

TEST(AtomicTests, IntrinSelect32ExchangeAcquire)
{
    int32_t value = 32;
    SI<int32_t>::Exchange(value, 0x0ff0e0d0, Acquire());
    EXPECT_EQ(value, 0x0ff0e0d0);
}

TEST(AtomicTests, IntrinSelect32ExchangeRelease)
{
    int32_t value = 32;
    SI<int32_t>::Exchange(value, 0x0ff0e0d0, Release());
    EXPECT_EQ(value, 0x0ff0e0d0);
}

TEST(AtomicTests, IntrinSelect32ExchangeAcqRel)
{
    int32_t value = 32;
    SI<int32_t>::Exchange(value, 0x0ff0e0d0, AcqRel());
    EXPECT_EQ(value, 0x0ff0e0d0);
}

TEST(AtomicTests, IntrinSelect32ExchangeSeqCst)
{
    int32_t value = 32;
    SI<int32_t>::Exchange(value, 0x0ff0e0d0, SeqCst());
    EXPECT_EQ(value, 0x0ff0e0d0);
}

TEST(AtomicTests, IntrinSelect64ExchangeRelaxed)
{
    int64_t value = 64;
    SI<int64_t>::Exchange(value, 0x0ff0e0d011223344, Relaxed());
    EXPECT_EQ(value, 0x0ff0e0d011223344);
}

TEST(AtomicTests, IntrinSelect64ExchangeConsume)
{
    int64_t value = 64;
    SI<int64_t>::Exchange(value, 0x0ff0e0d011223344, Consume());
    EXPECT_EQ(value, 0x0ff0e0d011223344);
}

TEST(AtomicTests, IntrinSelect64ExchangeAcquire)
{
    int64_t value = 64;
    SI<int64_t>::Exchange(value, 0x0ff0e0d011223344, Acquire());
    EXPECT_EQ(value, 0x0ff0e0d011223344);
}

TEST(AtomicTests, IntrinSelect64ExchangeRelease)
{
    int64_t value = 64;
    SI<int64_t>::Exchange(value, 0x0ff0e0d011223344, Release());
    EXPECT_EQ(value, 0x0ff0e0d011223344);
}

TEST(AtomicTests, IntrinSelect64ExchangeAcqRel)
{
    int64_t value = 64;
    SI<int64_t>::Exchange(value, 0x0ff0e0d011223344, AcqRel());
    EXPECT_EQ(value, 0x0ff0e0d011223344);
}

TEST(AtomicTests, IntrinSelect64ExchangeSeqCst)
{
    int64_t value = 64;
    SI<int64_t>::Exchange(value, 0x0ff0e0d011223344, SeqCst());
    EXPECT_EQ(value, 0x0ff0e0d011223344);
}

TEST(AtomicTests, IntrinSelect8CasWeakRelaxed)
{
    uint8_t value = 8;
    uint8_t expected = 7;
    bool ret = SI<uint8_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                Relaxed(),
                                                Relaxed());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 8);

    ret = SI<uint8_t>::CompareExchangeWeak(value,
                                           9,
                                           expected,
                                           Relaxed(),
                                           Relaxed());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 9);
}

TEST(AtomicTests, IntrinSelect8CasWeakConsume)
{
    uint8_t value = 8;
    uint8_t expected = 7;
    bool ret = SI<uint8_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                Consume(),
                                                Consume());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 8);

    ret = SI<uint8_t>::CompareExchangeWeak(value,
                                           9,
                                           expected,
                                           Consume(),
                                           Consume());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 9);
}

TEST(AtomicTests, IntrinSelect8CasWeakAcquire)
{
    uint8_t value = 8;
    uint8_t expected = 7;
    bool ret = SI<uint8_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                Acquire(),
                                                Acquire());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 8);

    ret = SI<uint8_t>::CompareExchangeWeak(value,
                                           9,
                                           expected,
                                           Acquire(),
                                           Acquire());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 9);
}

TEST(AtomicTests, IntrinSelect8CasWeakRelease)
{
    uint8_t value = 8;
    uint8_t expected = 7;
    bool ret = SI<uint8_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                Release(),
                                                Release());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 8);

    ret = SI<uint8_t>::CompareExchangeWeak(value,
                                           9,
                                           expected,
                                           Release(),
                                           Release());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 9);
}

TEST(AtomicTests, IntrinSelect8CasWeakAcqRel)
{
    uint8_t value = 8;
    uint8_t expected = 7;
    bool ret = SI<uint8_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                AcqRel(),
                                                AcqRel());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 8);

    ret = SI<uint8_t>::CompareExchangeWeak(value,
                                           9,
                                           expected,
                                           AcqRel(),
                                           AcqRel());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 9);
}

TEST(AtomicTests, IntrinSelect8CasWeakSeqCst)
{
    uint8_t value = 8;
    uint8_t expected = 7;
    bool ret = SI<uint8_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                SeqCst(),
                                                SeqCst());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 8);

    ret = SI<uint8_t>::CompareExchangeWeak(value,
                                           9,
                                           expected,
                                           SeqCst(),
                                           SeqCst());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 9);
}

TEST(AtomicTests, IntrinSelect16CasWeakRelaxed)
{
    int16_t value = 0x0ff0;
    int16_t expected = 7;
    bool ret = SI<int16_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                Relaxed(),
                                                Relaxed());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x0ff0);

    ret = SI<int16_t>::CompareExchangeWeak(value,
                                           0x1ff2,
                                           expected,
                                           Relaxed(),
                                           Relaxed());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x1ff2);
}

TEST(AtomicTests, IntrinSelect16CasWeakConsume)
{
    int16_t value = 0x0ff0;
    int16_t expected = 7;
    bool ret = SI<int16_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                Consume(),
                                                Consume());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x0ff0);

    ret = SI<int16_t>::CompareExchangeWeak(value,
                                           0x1ff2,
                                           expected,
                                           Consume(),
                                           Consume());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x1ff2);
}

TEST(AtomicTests, IntrinSelect16CasWeakAcquire)
{
    int16_t value = 0x0ff0;
    int16_t expected = 7;
    bool ret = SI<int16_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                Acquire(),
                                                Acquire());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x0ff0);

    ret = SI<int16_t>::CompareExchangeWeak(value,
                                           0x1ff2,
                                           expected,
                                           Acquire(),
                                           Acquire());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x1ff2);
}

TEST(AtomicTests, IntrinSelect16CasWeakRelease)
{
    int16_t value = 0x0ff0;
    int16_t expected = 7;
    bool ret = SI<int16_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                Release(),
                                                Release());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x0ff0);

    ret = SI<int16_t>::CompareExchangeWeak(value,
                                           0x1ff2,
                                           expected,
                                           Release(),
                                           Release());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x1ff2);
}

TEST(AtomicTests, IntrinSelect16CasWeakAcqRel)
{
    int16_t value = 0x0ff0;
    int16_t expected = 7;
    bool ret = SI<int16_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                AcqRel(),
                                                AcqRel());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x0ff0);

    ret = SI<int16_t>::CompareExchangeWeak(value,
                                           0x1ff2,
                                           expected,
                                           AcqRel(),
                                           AcqRel());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x1ff2);
}

TEST(AtomicTests, IntrinSelect16CasWeakSeqCst)
{
    int16_t value = 0x0ff0;
    int16_t expected = 7;
    bool ret = SI<int16_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                SeqCst(),
                                                SeqCst());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x0ff0);

    ret = SI<int16_t>::CompareExchangeWeak(value,
                                           0x1ff2,
                                           expected,
                                           SeqCst(),
                                           SeqCst());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x1ff2);
}

TEST(AtomicTests, IntrinSelect32CasWeakRelaxed)
{
    int32_t value = 0x0ff0e0d0;
    int32_t expected = 7;
    bool ret = SI<int32_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                Relaxed(),
                                                Relaxed());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x0ff0e0d0);

    ret = SI<int32_t>::CompareExchangeWeak(value,
                                           0x1ff2f3f4,
                                           expected,
                                           Relaxed(),
                                           Relaxed());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x1ff2f3f4);
}

TEST(AtomicTests, IntrinSelect32CasWeakConsume)
{
    int32_t value = 0x0ff0e0d0;
    int32_t expected = 7;
    bool ret = SI<int32_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                Consume(),
                                                Consume());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x0ff0e0d0);

    ret = SI<int32_t>::CompareExchangeWeak(value,
                                           0x1ff2f3f4,
                                           expected,
                                           Consume(),
                                           Consume());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x1ff2f3f4);
}

TEST(AtomicTests, IntrinSelect32CasWeakAcquire)
{
    int32_t value = 0x0ff0e0d0;
    int32_t expected = 7;
    bool ret = SI<int32_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                Acquire(),
                                                Acquire());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x0ff0e0d0);

    ret = SI<int32_t>::CompareExchangeWeak(value,
                                           0x1ff2f3f4,
                                           expected,
                                           Acquire(),
                                           Acquire());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x1ff2f3f4);
}

TEST(AtomicTests, IntrinSelect32CasWeakRelease)
{
    int32_t value = 0x0ff0e0d0;
    int32_t expected = 7;
    bool ret = SI<int32_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                Release(),
                                                Release());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x0ff0e0d0);

    ret = SI<int32_t>::CompareExchangeWeak(value,
                                           0x1ff2f3f4,
                                           expected,
                                           Release(),
                                           Release());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x1ff2f3f4);
}

TEST(AtomicTests, IntrinSelect32CasWeakAcqRel)
{
    int32_t value = 0x0ff0e0d0;
    int32_t expected = 7;
    bool ret = SI<int32_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                AcqRel(),
                                                AcqRel());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x0ff0e0d0);

    ret = SI<int32_t>::CompareExchangeWeak(value,
                                           0x1ff2f3f4,
                                           expected,
                                           AcqRel(),
                                           AcqRel());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x1ff2f3f4);
}

TEST(AtomicTests, IntrinSelect32CasWeakSeqCst)
{
    int32_t value = 0x0ff0e0d0;
    int32_t expected = 7;
    bool ret = SI<int32_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                SeqCst(),
                                                SeqCst());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x0ff0e0d0);

    ret = SI<int32_t>::CompareExchangeWeak(value,
                                           0x1ff2f3f4,
                                           expected,
                                           SeqCst(),
                                           SeqCst());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x1ff2f3f4);
}

TEST(AtomicTests, IntrinSelect64CasWeakRelaxed)
{
    int64_t value = 0x0ff0e0d011223344;
    int64_t expected = 7;
    bool ret = SI<int64_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                Relaxed(),
                                                Relaxed());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x0ff0e0d011223344);

    ret = SI<int64_t>::CompareExchangeWeak(value,
                                           0x1ff2f3f411223344,
                                           expected,
                                           Relaxed(),
                                           Relaxed());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x1ff2f3f411223344);
}

TEST(AtomicTests, IntrinSelect64CasWeakConsume)
{
    int64_t value = 0x0ff0e0d011223344;
    int64_t expected = 7;
    bool ret = SI<int64_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                Consume(),
                                                Consume());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x0ff0e0d011223344);

    ret = SI<int64_t>::CompareExchangeWeak(value,
                                           0x1ff2f3f411223344,
                                           expected,
                                           Consume(),
                                           Consume());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x1ff2f3f411223344);
}

TEST(AtomicTests, IntrinSelect64CasWeakAcquire)
{
    int64_t value = 0x0ff0e0d011223344;
    int64_t expected = 7;
    bool ret = SI<int64_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                Acquire(),
                                                Acquire());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x0ff0e0d011223344);

    ret = SI<int64_t>::CompareExchangeWeak(value,
                                           0x1ff2f3f411223344,
                                           expected,
                                           Acquire(),
                                           Acquire());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x1ff2f3f411223344);
}

TEST(AtomicTests, IntrinSelect64CasWeakRelease)
{
    int64_t value = 0x0ff0e0d011223344;
    int64_t expected = 7;
    bool ret = SI<int64_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                Release(),
                                                Release());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x0ff0e0d011223344);

    ret = SI<int64_t>::CompareExchangeWeak(value,
                                           0x1ff2f3f411223344,
                                           expected,
                                           Release(),
                                           Release());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x1ff2f3f411223344);
}

TEST(AtomicTests, IntrinSelect64CasWeakAcqRel)
{
    int64_t value = 0x0ff0e0d011223344;
    int64_t expected = 7;
    bool ret = SI<int64_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                AcqRel(),
                                                AcqRel());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x0ff0e0d011223344);

    ret = SI<int64_t>::CompareExchangeWeak(value,
                                           0x1ff2f3f411223344,
                                           expected,
                                           AcqRel(),
                                           AcqRel());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x1ff2f3f411223344);
}

TEST(AtomicTests, IntrinSelect64CasWeakSeqCst)
{
    int64_t value = 0x0ff0e0d011223344;
    int64_t expected = 7;
    bool ret = SI<int64_t>::CompareExchangeWeak(value,
                                                9,
                                                expected,
                                                SeqCst(),
                                                SeqCst());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x0ff0e0d011223344);

    ret = SI<int64_t>::CompareExchangeWeak(value,
                                           0x1ff2f3f411223344,
                                           expected,
                                           SeqCst(),
                                           SeqCst());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x1ff2f3f411223344);
}

TEST(AtomicTests, IntrinSelect8CasStrongRelaxed)
{
    uint8_t value = 8;
    uint8_t expected = 7;
    bool ret = SI<uint8_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  Relaxed(),
                                                  Relaxed());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 8);

    ret = SI<uint8_t>::CompareExchangeStrong(value,
                                             9,
                                             expected,
                                             Relaxed(),
                                             Relaxed());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 9);
}

TEST(AtomicTests, IntrinSelect8CasStrongConsume)
{
    uint8_t value = 8;
    uint8_t expected = 7;
    bool ret = SI<uint8_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  Consume(),
                                                  Consume());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 8);

    ret = SI<uint8_t>::CompareExchangeStrong(value,
                                             9,
                                             expected,
                                             Consume(),
                                             Consume());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 9);
}

TEST(AtomicTests, IntrinSelect8CasStrongAcquire)
{
    uint8_t value = 8;
    uint8_t expected = 7;
    bool ret = SI<uint8_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  Acquire(),
                                                  Acquire());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 8);

    ret = SI<uint8_t>::CompareExchangeStrong(value,
                                             9,
                                             expected,
                                             Acquire(),
                                             Acquire());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 9);
}

TEST(AtomicTests, IntrinSelect8CasStrongRelease)
{
    uint8_t value = 8;
    uint8_t expected = 7;
    bool ret = SI<uint8_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  Release(),
                                                  Release());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 8);

    ret = SI<uint8_t>::CompareExchangeStrong(value,
                                             9,
                                             expected,
                                             Release(),
                                             Release());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 9);
}

TEST(AtomicTests, IntrinSelect8CasStrongAcqRel)
{
    uint8_t value = 8;
    uint8_t expected = 7;
    bool ret = SI<uint8_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  AcqRel(),
                                                  AcqRel());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 8);

    ret = SI<uint8_t>::CompareExchangeStrong(value,
                                             9,
                                             expected,
                                             AcqRel(),
                                             AcqRel());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 9);
}

TEST(AtomicTests, IntrinSelect8CasStrongSeqCst)
{
    uint8_t value = 8;
    uint8_t expected = 7;
    bool ret = SI<uint8_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  SeqCst(),
                                                  SeqCst());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 8);

    ret = SI<uint8_t>::CompareExchangeStrong(value,
                                             9,
                                             expected,
                                             SeqCst(),
                                             SeqCst());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 8);
    EXPECT_EQ(value, 9);
}

TEST(AtomicTests, IntrinSelect16CasStrongRelaxed)
{
    int16_t value = 0x0ff0;
    int16_t expected = 7;
    bool ret = SI<int16_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  Relaxed(),
                                                  Relaxed());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x0ff0);

    ret = SI<int16_t>::CompareExchangeStrong(value,
                                             0x1ff2,
                                             expected,
                                             Relaxed(),
                                             Relaxed());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x1ff2);
}

TEST(AtomicTests, IntrinSelect16CasStrongConsume)
{
    int16_t value = 0x0ff0;
    int16_t expected = 7;
    bool ret = SI<int16_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  Consume(),
                                                  Consume());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x0ff0);

    ret = SI<int16_t>::CompareExchangeStrong(value,
                                             0x1ff2,
                                             expected,
                                             Consume(),
                                             Consume());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x1ff2);
}

TEST(AtomicTests, IntrinSelect16CasStrongAcquire)
{
    int16_t value = 0x0ff0;
    int16_t expected = 7;
    bool ret = SI<int16_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  Acquire(),
                                                  Acquire());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x0ff0);

    ret = SI<int16_t>::CompareExchangeStrong(value,
                                             0x1ff2,
                                             expected,
                                             Acquire(),
                                             Acquire());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x1ff2);
}

TEST(AtomicTests, IntrinSelect16CasStrongRelease)
{
    int16_t value = 0x0ff0;
    int16_t expected = 7;
    bool ret = SI<int16_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  Release(),
                                                  Release());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x0ff0);

    ret = SI<int16_t>::CompareExchangeStrong(value,
                                             0x1ff2,
                                             expected,
                                             Release(),
                                             Release());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x1ff2);
}

TEST(AtomicTests, IntrinSelect16CasStrongAcqRel)
{
    int16_t value = 0x0ff0;
    int16_t expected = 7;
    bool ret = SI<int16_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  AcqRel(),
                                                  AcqRel());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x0ff0);

    ret = SI<int16_t>::CompareExchangeStrong(value,
                                             0x1ff2,
                                             expected,
                                             AcqRel(),
                                             AcqRel());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x1ff2);
}

TEST(AtomicTests, IntrinSelect16CasStrongSeqCst)
{
    int16_t value = 0x0ff0;
    int16_t expected = 7;
    bool ret = SI<int16_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  SeqCst(),
                                                  SeqCst());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x0ff0);

    ret = SI<int16_t>::CompareExchangeStrong(value,
                                             0x1ff2,
                                             expected,
                                             SeqCst(),
                                             SeqCst());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0);
    EXPECT_EQ(value, 0x1ff2);
}

TEST(AtomicTests, IntrinSelect32CasStrongRelaxed)
{
    int32_t value = 0x0ff0e0d0;
    int32_t expected = 7;
    bool ret = SI<int32_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  Relaxed(),
                                                  Relaxed());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x0ff0e0d0);

    ret = SI<int32_t>::CompareExchangeStrong(value,
                                             0x1ff2f3f4,
                                             expected,
                                             Relaxed(),
                                             Relaxed());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x1ff2f3f4);
}

TEST(AtomicTests, IntrinSelect32CasStrongConsume)
{
    int32_t value = 0x0ff0e0d0;
    int32_t expected = 7;
    bool ret = SI<int32_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  Consume(),
                                                  Consume());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x0ff0e0d0);

    ret = SI<int32_t>::CompareExchangeStrong(value,
                                             0x1ff2f3f4,
                                             expected,
                                             Consume(),
                                             Consume());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x1ff2f3f4);
}

TEST(AtomicTests, IntrinSelect32CasStrongAcquire)
{
    int32_t value = 0x0ff0e0d0;
    int32_t expected = 7;
    bool ret = SI<int32_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  Acquire(),
                                                  Acquire());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x0ff0e0d0);

    ret = SI<int32_t>::CompareExchangeStrong(value,
                                             0x1ff2f3f4,
                                             expected,
                                             Acquire(),
                                             Acquire());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x1ff2f3f4);
}

TEST(AtomicTests, IntrinSelect32CasStrongRelease)
{
    int32_t value = 0x0ff0e0d0;
    int32_t expected = 7;
    bool ret = SI<int32_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  Release(),
                                                  Release());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x0ff0e0d0);

    ret = SI<int32_t>::CompareExchangeStrong(value,
                                             0x1ff2f3f4,
                                             expected,
                                             Release(),
                                             Release());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x1ff2f3f4);
}

TEST(AtomicTests, IntrinSelect32CasStrongAcqRel)
{
    int32_t value = 0x0ff0e0d0;
    int32_t expected = 7;
    bool ret = SI<int32_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  AcqRel(),
                                                  AcqRel());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x0ff0e0d0);

    ret = SI<int32_t>::CompareExchangeStrong(value,
                                             0x1ff2f3f4,
                                             expected,
                                             AcqRel(),
                                             AcqRel());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x1ff2f3f4);
}

TEST(AtomicTests, IntrinSelect32CasStrongSeqCst)
{
    int32_t value = 0x0ff0e0d0;
    int32_t expected = 7;
    bool ret = SI<int32_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  SeqCst(),
                                                  SeqCst());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x0ff0e0d0);

    ret = SI<int32_t>::CompareExchangeStrong(value,
                                             0x1ff2f3f4,
                                             expected,
                                             SeqCst(),
                                             SeqCst());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d0);
    EXPECT_EQ(value, 0x1ff2f3f4);
}

TEST(AtomicTests, IntrinSelect64CasStrongRelaxed)
{
    int64_t value = 0x0ff0e0d011223344;
    int64_t expected = 7;
    bool ret = SI<int64_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  Relaxed(),
                                                  Relaxed());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x0ff0e0d011223344);

    ret = SI<int64_t>::CompareExchangeStrong(value,
                                             0x1ff2f3f411223344,
                                             expected,
                                             Relaxed(),
                                             Relaxed());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x1ff2f3f411223344);
}

TEST(AtomicTests, IntrinSelect64CasStrongConsume)
{
    int64_t value = 0x0ff0e0d011223344;
    int64_t expected = 7;
    bool ret = SI<int64_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  Consume(),
                                                  Consume());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x0ff0e0d011223344);

    ret = SI<int64_t>::CompareExchangeStrong(value,
                                             0x1ff2f3f411223344,
                                             expected,
                                             Consume(),
                                             Consume());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x1ff2f3f411223344);
}

TEST(AtomicTests, IntrinSelect64CasStrongAcquire)
{
    int64_t value = 0x0ff0e0d011223344;
    int64_t expected = 7;
    bool ret = SI<int64_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  Acquire(),
                                                  Acquire());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x0ff0e0d011223344);

    ret = SI<int64_t>::CompareExchangeStrong(value,
                                             0x1ff2f3f411223344,
                                             expected,
                                             Acquire(),
                                             Acquire());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x1ff2f3f411223344);
}

TEST(AtomicTests, IntrinSelect64CasStrongRelease)
{
    int64_t value = 0x0ff0e0d011223344;
    int64_t expected = 7;
    bool ret = SI<int64_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  Release(),
                                                  Release());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x0ff0e0d011223344);

    ret = SI<int64_t>::CompareExchangeStrong(value,
                                             0x1ff2f3f411223344,
                                             expected,
                                             Release(),
                                             Release());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x1ff2f3f411223344);
}

TEST(AtomicTests, IntrinSelect64CasStrongAcqRel)
{
    int64_t value = 0x0ff0e0d011223344;
    int64_t expected = 7;
    bool ret = SI<int64_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  AcqRel(),
                                                  AcqRel());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x0ff0e0d011223344);

    ret = SI<int64_t>::CompareExchangeStrong(value,
                                             0x1ff2f3f411223344,
                                             expected,
                                             AcqRel(),
                                             AcqRel());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x1ff2f3f411223344);
}

TEST(AtomicTests, IntrinSelect64CasStrongSeqCst)
{
    int64_t value = 0x0ff0e0d011223344;
    int64_t expected = 7;
    bool ret = SI<int64_t>::CompareExchangeStrong(value,
                                                  9,
                                                  expected,
                                                  SeqCst(),
                                                  SeqCst());
    EXPECT_EQ(ret, false);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x0ff0e0d011223344);

    ret = SI<int64_t>::CompareExchangeStrong(value,
                                             0x1ff2f3f411223344,
                                             expected,
                                             SeqCst(),
                                             SeqCst());
    EXPECT_EQ(ret, true);
    EXPECT_EQ(expected, 0x0ff0e0d011223344);
    EXPECT_EQ(value, 0x1ff2f3f411223344);
}

TEST(AtomicTests, IntrinSelect8FetchAddRelaxed)
{
    uint8_t value = 8;
    uint8_t result = SI<uint8_t>::FetchAdd(value, 4, Relaxed());
    EXPECT_EQ(value, 12);
    EXPECT_EQ(result, 8);
}

TEST(AtomicTests, IntrinSelect8FetchAddConsume)
{
    uint8_t value = 8;
    uint8_t result = SI<uint8_t>::FetchAdd(value, 4, Consume());
    EXPECT_EQ(value, 12);
    EXPECT_EQ(result, 8);
}

TEST(AtomicTests, IntrinSelect8FetchAddAcquire)
{
    uint8_t value = 8;
    uint8_t result = SI<uint8_t>::FetchAdd(value, 4, Acquire());
    EXPECT_EQ(value, 12);
    EXPECT_EQ(result, 8);
}

TEST(AtomicTests, IntrinSelect8FetchAddRelease)
{
    uint8_t value = 8;
    uint8_t result = SI<uint8_t>::FetchAdd(value, 4, Release());
    EXPECT_EQ(value, 12);
    EXPECT_EQ(result, 8);
}

TEST(AtomicTests, IntrinSelect8FetchAddAcqRel)
{
    uint8_t value = 8;
    uint8_t result = SI<uint8_t>::FetchAdd(value, 4, AcqRel());
    EXPECT_EQ(value, 12);
    EXPECT_EQ(result, 8);
}

TEST(AtomicTests, IntrinSelect8FetchAddSeqCst)
{
    uint8_t value = 8;
    uint8_t result = SI<uint8_t>::FetchAdd(value, 4, SeqCst());
    EXPECT_EQ(value, 12);
    EXPECT_EQ(result, 8);
}

TEST(AtomicTests, IntrinSelect16FetchAddRelaxed)
{
    uint16_t value = 0x1ff;
    uint16_t result = SI<uint16_t>::FetchAdd(value, 4, Relaxed());
    EXPECT_EQ(value, 0x203);
    EXPECT_EQ(result, 0x1ff);
}

TEST(AtomicTests, IntrinSelect16FetchAddConsume)
{
    uint16_t value = 0x1ff;
    uint16_t result = SI<uint16_t>::FetchAdd(value, 4, Consume());
    EXPECT_EQ(value, 0x203);
    EXPECT_EQ(result, 0x1ff);
}

TEST(AtomicTests, IntrinSelect16FetchAddAcquire)
{
    uint16_t value = 0x1ff;
    uint16_t result = SI<uint16_t>::FetchAdd(value, 4, Acquire());
    EXPECT_EQ(value, 0x203);
    EXPECT_EQ(result, 0x1ff);
}

TEST(AtomicTests, IntrinSelect16FetchAddRelease)
{
    uint16_t value = 0x1ff;
    uint16_t result = SI<uint16_t>::FetchAdd(value, 4, Release());
    EXPECT_EQ(value, 0x203);
    EXPECT_EQ(result, 0x1ff);
}

TEST(AtomicTests, IntrinSelect16FetchAddAcqRel)
{
    uint16_t value = 0x1ff;
    uint16_t result = SI<uint16_t>::FetchAdd(value, 4, AcqRel());
    EXPECT_EQ(value, 0x203);
    EXPECT_EQ(result, 0x1ff);
}

TEST(AtomicTests, IntrinSelect16FetchAddSeqCst)
{
    uint16_t value = 0x1ff;
    uint16_t result = SI<uint16_t>::FetchAdd(value, 4, SeqCst());
    EXPECT_EQ(value, 0x203);
    EXPECT_EQ(result, 0x1ff);
}

TEST(AtomicTests, IntrinSelect32FetchAddRelaxed)
{
    uint32_t value = 0x1ffffff;
    uint32_t result = SI<uint32_t>::FetchAdd(value, 4, Relaxed());
    EXPECT_EQ(value, 0x2000003u);
    EXPECT_EQ(result, 0x1ffffffu);
}

TEST(AtomicTests, IntrinSelect32FetchAddConsume)
{
    uint32_t value = 0x1ffffff;
    uint32_t result = SI<uint32_t>::FetchAdd(value, 4, Consume());
    EXPECT_EQ(value, 0x2000003u);
    EXPECT_EQ(result, 0x1ffffffu);
}

TEST(AtomicTests, IntrinSelect32FetchAddAcquire)
{
    uint32_t value = 0x1ffffff;
    uint32_t result = SI<uint32_t>::FetchAdd(value, 4, Acquire());
    EXPECT_EQ(value, 0x2000003u);
    EXPECT_EQ(result, 0x1ffffffu);
}

TEST(AtomicTests, IntrinSelect32FetchAddRelease)
{
    uint32_t value = 0x1ffffff;
    uint32_t result = SI<uint32_t>::FetchAdd(value, 4, Release());
    EXPECT_EQ(value, 0x2000003u);
    EXPECT_EQ(result, 0x1ffffffu);
}

TEST(AtomicTests, IntrinSelect32FetchAddAcqRel)
{
    uint32_t value = 0x1ffffff;
    uint32_t result = SI<uint32_t>::FetchAdd(value, 4, AcqRel());
    EXPECT_EQ(value, 0x2000003u);
    EXPECT_EQ(result, 0x1ffffffu);
}

TEST(AtomicTests, IntrinSelect32FetchAddSeqCst)
{
    uint32_t value = 0x1ffffff;
    uint32_t result = SI<uint32_t>::FetchAdd(value, 4, SeqCst());
    EXPECT_EQ(value, 0x2000003u);
    EXPECT_EQ(result, 0x1ffffffu);
}

TEST(AtomicTests, IntrinSelect64FetchAddRelaxed)
{
    uint64_t value = 0x1ffffffffffffff;
    uint64_t result = SI<uint64_t>::FetchAdd(value, 4, Relaxed());
    EXPECT_EQ(value, 0x200000000000003ull);
    EXPECT_EQ(result, 0x1ffffffffffffffull);
}

TEST(AtomicTests, IntrinSelect64FetchAddConsume)
{
    uint64_t value = 0x1ffffffffffffff;
    uint64_t result = SI<uint64_t>::FetchAdd(value, 4, Consume());
    EXPECT_EQ(value, 0x200000000000003ull);
    EXPECT_EQ(result, 0x1ffffffffffffffull);
}

TEST(AtomicTests, IntrinSelect64FetchAddAcquire)
{
    uint64_t value = 0x1ffffffffffffff;
    uint64_t result = SI<uint64_t>::FetchAdd(value, 4, Acquire());
    EXPECT_EQ(value, 0x200000000000003ull);
    EXPECT_EQ(result, 0x1ffffffffffffffull);
}

TEST(AtomicTests, IntrinSelect64FetchAddRelease)
{
    uint64_t value = 0x1ffffffffffffff;
    uint64_t result = SI<uint64_t>::FetchAdd(value, 4, Release());
    EXPECT_EQ(value, 0x200000000000003ull);
    EXPECT_EQ(result, 0x1ffffffffffffffull);
}

TEST(AtomicTests, IntrinSelect64FetchAddAcqRel)
{
    uint64_t value = 0x1ffffffffffffff;
    uint64_t result = SI<uint64_t>::FetchAdd(value, 4, AcqRel());
    EXPECT_EQ(value, 0x200000000000003ull);
    EXPECT_EQ(result, 0x1ffffffffffffffull);
}

TEST(AtomicTests, IntrinSelect64FetchAddSeqCst)
{
    uint64_t value = 0x1ffffffffffffff;
    uint64_t result = SI<uint64_t>::FetchAdd(value, 4, SeqCst());
    EXPECT_EQ(value, 0x200000000000003ull);
    EXPECT_EQ(result, 0x1ffffffffffffffull);
}

TEST(AtomicTests, IntrinSelectPtrFetchAddRelaxed)
{
    char* ptr = rad::Add2Ptr<char>(nullptr, 0x1ffffff);
    char* orgptr = ptr;
    char* ptr2 = rad::Add2Ptr<char>(nullptr, 0x2000003);
    char* result = SI<char*>::FetchAdd(ptr, 4, Relaxed());
    EXPECT_EQ(ptr, ptr2);
    EXPECT_EQ(result, orgptr);
}

TEST(AtomicTests, IntrinSelectPtrFetchAddConsume)
{
    char* ptr = rad::Add2Ptr<char>(nullptr, 0x1ffffff);
    char* orgptr = ptr;
    char* ptr2 = rad::Add2Ptr<char>(nullptr, 0x2000003);
    char* result = SI<char*>::FetchAdd(ptr, 4, Consume());
    EXPECT_EQ(ptr, ptr2);
    EXPECT_EQ(result, orgptr);
}

TEST(AtomicTests, IntrinSelectPtrFetchAddAcquire)
{
    char* ptr = rad::Add2Ptr<char>(nullptr, 0x1ffffff);
    char* orgptr = ptr;
    char* ptr2 = rad::Add2Ptr<char>(nullptr, 0x2000003);
    char* result = SI<char*>::FetchAdd(ptr, 4, Acquire());
    EXPECT_EQ(ptr, ptr2);
    EXPECT_EQ(result, orgptr);
}

TEST(AtomicTests, IntrinSelectPtrFetchAddRelease)
{
    char* ptr = rad::Add2Ptr<char>(nullptr, 0x1ffffff);
    char* orgptr = ptr;
    char* ptr2 = rad::Add2Ptr<char>(nullptr, 0x2000003);
    char* result = SI<char*>::FetchAdd(ptr, 4, Release());
    EXPECT_EQ(ptr, ptr2);
    EXPECT_EQ(result, orgptr);
}

TEST(AtomicTests, IntrinSelectPtrFetchAddAcqRel)
{
    char* ptr = rad::Add2Ptr<char>(nullptr, 0x1ffffff);
    char* orgptr = ptr;
    char* ptr2 = rad::Add2Ptr<char>(nullptr, 0x2000003);
    char* result = SI<char*>::FetchAdd(ptr, 4, AcqRel());
    EXPECT_EQ(ptr, ptr2);
    EXPECT_EQ(result, orgptr);
}

TEST(AtomicTests, IntrinSelectPtrFetchAddSeqCst)
{
    char* ptr = rad::Add2Ptr<char>(nullptr, 0x1ffffff);
    char* orgptr = ptr;
    char* ptr2 = rad::Add2Ptr<char>(nullptr, 0x2000003);
    char* result = SI<char*>::FetchAdd(ptr, 4, SeqCst());
    EXPECT_EQ(ptr, ptr2);
    EXPECT_EQ(result, orgptr);
}

TEST(AtomicTests, IntrinSelect8FetchSubRelaxed)
{
    uint8_t value = 8;
    uint8_t result = SI<uint8_t>::FetchSub(value, 4, Relaxed());
    EXPECT_EQ(value, 4);
    EXPECT_EQ(result, 8);
}

TEST(AtomicTests, IntrinSelect8FetchSubConsume)
{
    uint8_t value = 8;
    uint8_t result = SI<uint8_t>::FetchSub(value, 4, Consume());
    EXPECT_EQ(value, 4);
    EXPECT_EQ(result, 8);
}

TEST(AtomicTests, IntrinSelect8FetchSubAcquire)
{
    uint8_t value = 8;
    uint8_t result = SI<uint8_t>::FetchSub(value, 4, Acquire());
    EXPECT_EQ(value, 4);
    EXPECT_EQ(result, 8);
}

TEST(AtomicTests, IntrinSelect8FetchSubRelease)
{
    uint8_t value = 8;
    uint8_t result = SI<uint8_t>::FetchSub(value, 4, Release());
    EXPECT_EQ(value, 4);
    EXPECT_EQ(result, 8);
}

TEST(AtomicTests, IntrinSelect8FetchSubAcqRel)
{
    uint8_t value = 8;
    uint8_t result = SI<uint8_t>::FetchSub(value, 4, AcqRel());
    EXPECT_EQ(value, 4);
    EXPECT_EQ(result, 8);
}

TEST(AtomicTests, IntrinSelect8FetchSubSeqCst)
{
    uint8_t value = 8;
    uint8_t result = SI<uint8_t>::FetchSub(value, 4, SeqCst());
    EXPECT_EQ(value, 4);
    EXPECT_EQ(result, 8);
}

TEST(AtomicTests, IntrinSelect16FetchSubRelaxed)
{
    uint16_t value = 0x203;
    uint16_t result = SI<uint16_t>::FetchSub(value, 4, Relaxed());
    EXPECT_EQ(value, 0x1ff);
    EXPECT_EQ(result, 0x203);
}

TEST(AtomicTests, IntrinSelect16FetchSubConsume)
{
    uint16_t value = 0x203;
    uint16_t result = SI<uint16_t>::FetchSub(value, 4, Consume());
    EXPECT_EQ(value, 0x1ff);
    EXPECT_EQ(result, 0x203);
}

TEST(AtomicTests, IntrinSelect16FetchSubAcquire)
{
    uint16_t value = 0x203;
    uint16_t result = SI<uint16_t>::FetchSub(value, 4, Acquire());
    EXPECT_EQ(value, 0x1ff);
    EXPECT_EQ(result, 0x203);
}

TEST(AtomicTests, IntrinSelect16FetchSubRelease)
{
    uint16_t value = 0x203;
    uint16_t result = SI<uint16_t>::FetchSub(value, 4, Release());
    EXPECT_EQ(value, 0x1ff);
    EXPECT_EQ(result, 0x203);
}

TEST(AtomicTests, IntrinSelect16FetchSubAcqRel)
{
    uint16_t value = 0x203;
    uint16_t result = SI<uint16_t>::FetchSub(value, 4, AcqRel());
    EXPECT_EQ(value, 0x1ff);
    EXPECT_EQ(result, 0x203);
}

TEST(AtomicTests, IntrinSelect16FetchSubSeqCst)
{
    uint16_t value = 0x203;
    uint16_t result = SI<uint16_t>::FetchSub(value, 4, SeqCst());
    EXPECT_EQ(value, 0x1ff);
    EXPECT_EQ(result, 0x203);
}

TEST(AtomicTests, IntrinSelect32FetchSubRelaxed)
{
    uint32_t value = 0x2000003;
    uint32_t result = SI<uint32_t>::FetchSub(value, 4, Relaxed());
    EXPECT_EQ(value, 0x1ffffffu);
    EXPECT_EQ(result, 0x2000003u);
}

TEST(AtomicTests, IntrinSelect32FetchSubConsume)
{
    uint32_t value = 0x2000003;
    uint32_t result = SI<uint32_t>::FetchSub(value, 4, Consume());
    EXPECT_EQ(value, 0x1ffffffu);
    EXPECT_EQ(result, 0x2000003u);
}

TEST(AtomicTests, IntrinSelect32FetchSubAcquire)
{
    uint32_t value = 0x2000003;
    uint32_t result = SI<uint32_t>::FetchSub(value, 4, Acquire());
    EXPECT_EQ(value, 0x1ffffffu);
    EXPECT_EQ(result, 0x2000003u);
}

TEST(AtomicTests, IntrinSelect32FetchSubRelease)
{
    uint32_t value = 0x2000003;
    uint32_t result = SI<uint32_t>::FetchSub(value, 4, Release());
    EXPECT_EQ(value, 0x1ffffffu);
    EXPECT_EQ(result, 0x2000003u);
}

TEST(AtomicTests, IntrinSelect32FetchSubAcqRel)
{
    uint32_t value = 0x2000003;
    uint32_t result = SI<uint32_t>::FetchSub(value, 4, AcqRel());
    EXPECT_EQ(value, 0x1ffffffu);
    EXPECT_EQ(result, 0x2000003u);
}

TEST(AtomicTests, IntrinSelect32FetchSubSeqCst)
{
    uint32_t value = 0x2000003;
    uint32_t result = SI<uint32_t>::FetchSub(value, 4, SeqCst());
    EXPECT_EQ(value, 0x1ffffffu);
    EXPECT_EQ(result, 0x2000003u);
}

TEST(AtomicTests, IntrinSelect64FetchSubRelaxed)
{
    uint64_t value = 0x200000000000003;
    uint64_t result = SI<uint64_t>::FetchSub(value, 4, Relaxed());
    EXPECT_EQ(value, 0x1ffffffffffffffull);
    EXPECT_EQ(result, 0x200000000000003ull);
}

TEST(AtomicTests, IntrinSelect64FetchSubConsume)
{
    uint64_t value = 0x200000000000003;
    uint64_t result = SI<uint64_t>::FetchSub(value, 4, Consume());
    EXPECT_EQ(value, 0x1ffffffffffffffull);
    EXPECT_EQ(result, 0x200000000000003ull);
}

TEST(AtomicTests, IntrinSelect64FetchSubAcquire)
{
    uint64_t value = 0x200000000000003;
    uint64_t result = SI<uint64_t>::FetchSub(value, 4, Acquire());
    EXPECT_EQ(value, 0x1ffffffffffffffull);
    EXPECT_EQ(result, 0x200000000000003ull);
}

TEST(AtomicTests, IntrinSelect64FetchSubRelease)
{
    uint64_t value = 0x200000000000003;
    uint64_t result = SI<uint64_t>::FetchSub(value, 4, Release());
    EXPECT_EQ(value, 0x1ffffffffffffffull);
    EXPECT_EQ(result, 0x200000000000003ull);
}

TEST(AtomicTests, IntrinSelect64FetchSubAcqRel)
{
    uint64_t value = 0x200000000000003;
    uint64_t result = SI<uint64_t>::FetchSub(value, 4, AcqRel());
    EXPECT_EQ(value, 0x1ffffffffffffffull);
    EXPECT_EQ(result, 0x200000000000003ull);
}

TEST(AtomicTests, IntrinSelect64FetchSubSeqCst)
{
    uint64_t value = 0x200000000000003;
    uint64_t result = SI<uint64_t>::FetchSub(value, 4, SeqCst());
    EXPECT_EQ(value, 0x1ffffffffffffffull);
    EXPECT_EQ(result, 0x200000000000003ull);
}

TEST(AtomicTests, IntrinSelect8FetchAndRelaxed)
{
    uint8_t value = 6;
    uint8_t result = SI<uint8_t>::FetchAnd(value, 3, Relaxed());
    EXPECT_EQ(value, 2);
    EXPECT_EQ(result, 6);
}

TEST(AtomicTests, IntrinSelect8FetchAndConsume)
{
    uint8_t value = 6;
    uint8_t result = SI<uint8_t>::FetchAnd(value, 3, Consume());
    EXPECT_EQ(value, 2);
    EXPECT_EQ(result, 6);
}

TEST(AtomicTests, IntrinSelect8FetchAndAcquire)
{
    uint8_t value = 6;
    uint8_t result = SI<uint8_t>::FetchAnd(value, 3, Acquire());
    EXPECT_EQ(value, 2);
    EXPECT_EQ(result, 6);
}

TEST(AtomicTests, IntrinSelect8FetchAndRelease)
{
    uint8_t value = 6;
    uint8_t result = SI<uint8_t>::FetchAnd(value, 3, Release());
    EXPECT_EQ(value, 2);
    EXPECT_EQ(result, 6);
}

TEST(AtomicTests, IntrinSelect8FetchAndAcqRel)
{
    uint8_t value = 6;
    uint8_t result = SI<uint8_t>::FetchAnd(value, 3, AcqRel());
    EXPECT_EQ(value, 2);
    EXPECT_EQ(result, 6);
}

TEST(AtomicTests, IntrinSelect8FetchAndSeqCst)
{
    uint8_t value = 6;
    uint8_t result = SI<uint8_t>::FetchAnd(value, 3, SeqCst());
    EXPECT_EQ(value, 2);
    EXPECT_EQ(result, 6);
}

TEST(AtomicTests, IntrinSelect16FetchAndRelaxed)
{
    uint16_t value = 0x606;
    uint16_t result = SI<uint16_t>::FetchAnd(value, 0x303, Relaxed());
    EXPECT_EQ(value, 0x202);
    EXPECT_EQ(result, 0x606);
}

TEST(AtomicTests, IntrinSelect16FetchAndConsume)
{
    uint16_t value = 0x606;
    uint16_t result = SI<uint16_t>::FetchAnd(value, 0x303, Consume());
    EXPECT_EQ(value, 0x202);
    EXPECT_EQ(result, 0x606);
}

TEST(AtomicTests, IntrinSelect16FetchAndAcquire)
{
    uint16_t value = 0x606;
    uint16_t result = SI<uint16_t>::FetchAnd(value, 0x303, Acquire());
    EXPECT_EQ(value, 0x202);
    EXPECT_EQ(result, 0x606);
}

TEST(AtomicTests, IntrinSelect16FetchAndRelease)
{
    uint16_t value = 0x606;
    uint16_t result = SI<uint16_t>::FetchAnd(value, 0x303, Release());
    EXPECT_EQ(value, 0x202);
    EXPECT_EQ(result, 0x606);
}

TEST(AtomicTests, IntrinSelect16FetchAndAcqRel)
{
    uint16_t value = 0x606;
    uint16_t result = SI<uint16_t>::FetchAnd(value, 0x303, AcqRel());
    EXPECT_EQ(value, 0x202);
    EXPECT_EQ(result, 0x606);
}

TEST(AtomicTests, IntrinSelect16FetchAndSeqCst)
{
    uint16_t value = 0x606;
    uint16_t result = SI<uint16_t>::FetchAnd(value, 0x303, SeqCst());
    EXPECT_EQ(value, 0x202);
    EXPECT_EQ(result, 0x606);
}

TEST(AtomicTests, IntrinSelect32FetchAndRelaxed)
{
    uint32_t value = 0x6060606;
    uint32_t result = SI<uint32_t>::FetchAnd(value, 0x3030303, Relaxed());
    EXPECT_EQ(value, 0x2020202u);
    EXPECT_EQ(result, 0x6060606u);
}

TEST(AtomicTests, IntrinSelect32FetchAndConsume)
{
    uint32_t value = 0x6060606;
    uint32_t result = SI<uint32_t>::FetchAnd(value, 0x3030303, Consume());
    EXPECT_EQ(value, 0x2020202u);
    EXPECT_EQ(result, 0x6060606u);
}

TEST(AtomicTests, IntrinSelect32FetchAndAcquire)
{
    uint32_t value = 0x6060606;
    uint32_t result = SI<uint32_t>::FetchAnd(value, 0x3030303, Acquire());
    EXPECT_EQ(value, 0x2020202u);
    EXPECT_EQ(result, 0x6060606u);
}

TEST(AtomicTests, IntrinSelect32FetchAndRelease)
{
    uint32_t value = 0x6060606;
    uint32_t result = SI<uint32_t>::FetchAnd(value, 0x3030303, Release());
    EXPECT_EQ(value, 0x2020202u);
    EXPECT_EQ(result, 0x6060606u);
}

TEST(AtomicTests, IntrinSelect32FetchAndAcqRel)
{
    uint32_t value = 0x6060606;
    uint32_t result = SI<uint32_t>::FetchAnd(value, 0x3030303, AcqRel());
    EXPECT_EQ(value, 0x2020202u);
    EXPECT_EQ(result, 0x6060606u);
}

TEST(AtomicTests, IntrinSelect32FetchAndSeqCst)
{
    uint32_t value = 0x6060606;
    uint32_t result = SI<uint32_t>::FetchAnd(value, 0x3030303, SeqCst());
    EXPECT_EQ(value, 0x2020202u);
    EXPECT_EQ(result, 0x6060606u);
}

TEST(AtomicTests, IntrinSelect64FetchAndRelaxed)
{
    uint64_t value = 0x606060606060606;
    uint64_t result =
        SI<uint64_t>::FetchAnd(value, 0x303030303030303, Relaxed());
    EXPECT_EQ(value, 0x202020202020202ull);
    EXPECT_EQ(result, 0x606060606060606ull);
}

TEST(AtomicTests, IntrinSelect64FetchAndConsume)
{
    uint64_t value = 0x606060606060606;
    uint64_t result =
        SI<uint64_t>::FetchAnd(value, 0x303030303030303, Consume());
    EXPECT_EQ(value, 0x202020202020202ull);
    EXPECT_EQ(result, 0x606060606060606ull);
}

TEST(AtomicTests, IntrinSelect64FetchAndAcquire)
{
    uint64_t value = 0x606060606060606;
    uint64_t result =
        SI<uint64_t>::FetchAnd(value, 0x303030303030303, Acquire());
    EXPECT_EQ(value, 0x202020202020202ull);
    EXPECT_EQ(result, 0x606060606060606ull);
}

TEST(AtomicTests, IntrinSelect64FetchAndRelease)
{
    uint64_t value = 0x606060606060606;
    uint64_t result =
        SI<uint64_t>::FetchAnd(value, 0x303030303030303, Release());
    EXPECT_EQ(value, 0x202020202020202ull);
    EXPECT_EQ(result, 0x606060606060606ull);
}

TEST(AtomicTests, IntrinSelect64FetchAndAcqRel)
{
    uint64_t value = 0x606060606060606;
    uint64_t result =
        SI<uint64_t>::FetchAnd(value, 0x303030303030303, AcqRel());
    EXPECT_EQ(value, 0x202020202020202ull);
    EXPECT_EQ(result, 0x606060606060606ull);
}

TEST(AtomicTests, IntrinSelect64FetchAndSeqCst)
{
    uint64_t value = 0x606060606060606;
    uint64_t result =
        SI<uint64_t>::FetchAnd(value, 0x303030303030303, SeqCst());
    EXPECT_EQ(value, 0x202020202020202ull);
    EXPECT_EQ(result, 0x606060606060606ull);
}

TEST(AtomicTests, IntrinSelect8FetchOrRelaxed)
{
    uint8_t value = 6;
    uint8_t result = SI<uint8_t>::FetchOr(value, 3, Relaxed());
    EXPECT_EQ(value, 7);
    EXPECT_EQ(result, 6);
}

TEST(AtomicTests, IntrinSelect8FetchOrConsume)
{
    uint8_t value = 6;
    uint8_t result = SI<uint8_t>::FetchOr(value, 3, Consume());
    EXPECT_EQ(value, 7);
    EXPECT_EQ(result, 6);
}

TEST(AtomicTests, IntrinSelect8FetchOrAcquire)
{
    uint8_t value = 6;
    uint8_t result = SI<uint8_t>::FetchOr(value, 3, Acquire());
    EXPECT_EQ(value, 7);
    EXPECT_EQ(result, 6);
}

TEST(AtomicTests, IntrinSelect8FetchOrRelease)
{
    uint8_t value = 6;
    uint8_t result = SI<uint8_t>::FetchOr(value, 3, Release());
    EXPECT_EQ(value, 7);
    EXPECT_EQ(result, 6);
}

TEST(AtomicTests, IntrinSelect8FetchOrAcqRel)
{
    uint8_t value = 6;
    uint8_t result = SI<uint8_t>::FetchOr(value, 3, AcqRel());
    EXPECT_EQ(value, 7);
    EXPECT_EQ(result, 6);
}

TEST(AtomicTests, IntrinSelect8FetchOrSeqCst)
{
    uint8_t value = 6;
    uint8_t result = SI<uint8_t>::FetchOr(value, 3, SeqCst());
    EXPECT_EQ(value, 7);
    EXPECT_EQ(result, 6);
}

TEST(AtomicTests, IntrinSelect16FetchOrRelaxed)
{
    uint16_t value = 0x606;
    uint16_t result = SI<uint16_t>::FetchOr(value, 0x303, Relaxed());
    EXPECT_EQ(value, 0x707);
    EXPECT_EQ(result, 0x606);
}

TEST(AtomicTests, IntrinSelect16FetchOrConsume)
{
    uint16_t value = 0x606;
    uint16_t result = SI<uint16_t>::FetchOr(value, 0x303, Consume());
    EXPECT_EQ(value, 0x707);
    EXPECT_EQ(result, 0x606);
}

TEST(AtomicTests, IntrinSelect16FetchOrAcquire)
{
    uint16_t value = 0x606;
    uint16_t result = SI<uint16_t>::FetchOr(value, 0x303, Acquire());
    EXPECT_EQ(value, 0x707);
    EXPECT_EQ(result, 0x606);
}

TEST(AtomicTests, IntrinSelect16FetchOrRelease)
{
    uint16_t value = 0x606;
    uint16_t result = SI<uint16_t>::FetchOr(value, 0x303, Release());
    EXPECT_EQ(value, 0x707);
    EXPECT_EQ(result, 0x606);
}

TEST(AtomicTests, IntrinSelect16FetchOrAcqRel)
{
    uint16_t value = 0x606;
    uint16_t result = SI<uint16_t>::FetchOr(value, 0x303, AcqRel());
    EXPECT_EQ(value, 0x707);
    EXPECT_EQ(result, 0x606);
}

TEST(AtomicTests, IntrinSelect16FetchOrSeqCst)
{
    uint16_t value = 0x606;
    uint16_t result = SI<uint16_t>::FetchOr(value, 0x303, SeqCst());
    EXPECT_EQ(value, 0x707);
    EXPECT_EQ(result, 0x606);
}

TEST(AtomicTests, IntrinSelect32FetchOrRelaxed)
{
    uint32_t value = 0x6060606;
    uint32_t result = SI<uint32_t>::FetchOr(value, 0x3030303, Relaxed());
    EXPECT_EQ(value, 0x7070707u);
    EXPECT_EQ(result, 0x6060606u);
}

TEST(AtomicTests, IntrinSelect32FetchOrConsume)
{
    uint32_t value = 0x6060606;
    uint32_t result = SI<uint32_t>::FetchOr(value, 0x3030303, Consume());
    EXPECT_EQ(value, 0x7070707u);
    EXPECT_EQ(result, 0x6060606u);
}

TEST(AtomicTests, IntrinSelect32FetchOrAcquire)
{
    uint32_t value = 0x6060606;
    uint32_t result = SI<uint32_t>::FetchOr(value, 0x3030303, Acquire());
    EXPECT_EQ(value, 0x7070707u);
    EXPECT_EQ(result, 0x6060606u);
}

TEST(AtomicTests, IntrinSelect32FetchOrRelease)
{
    uint32_t value = 0x6060606;
    uint32_t result = SI<uint32_t>::FetchOr(value, 0x3030303, Release());
    EXPECT_EQ(value, 0x7070707u);
    EXPECT_EQ(result, 0x6060606u);
}

TEST(AtomicTests, IntrinSelect32FetchOrAcqRel)
{
    uint32_t value = 0x6060606;
    uint32_t result = SI<uint32_t>::FetchOr(value, 0x3030303, AcqRel());
    EXPECT_EQ(value, 0x7070707u);
    EXPECT_EQ(result, 0x6060606u);
}

TEST(AtomicTests, IntrinSelect32FetchOrSeqCst)
{
    uint32_t value = 0x6060606;
    uint32_t result = SI<uint32_t>::FetchOr(value, 0x3030303, SeqCst());
    EXPECT_EQ(value, 0x7070707u);
    EXPECT_EQ(result, 0x6060606u);
}

TEST(AtomicTests, IntrinSelect64FetchOrRelaxed)
{
    uint64_t value = 0x606060606060606;
    uint64_t result =
        SI<uint64_t>::FetchOr(value, 0x303030303030303, Relaxed());
    EXPECT_EQ(value, 0x707070707070707ull);
    EXPECT_EQ(result, 0x606060606060606ull);
}

TEST(AtomicTests, IntrinSelect64FetchOrConsume)
{
    uint64_t value = 0x606060606060606;
    uint64_t result =
        SI<uint64_t>::FetchOr(value, 0x303030303030303, Consume());
    EXPECT_EQ(value, 0x707070707070707ull);
    EXPECT_EQ(result, 0x606060606060606ull);
}

TEST(AtomicTests, IntrinSelect64FetchOrAcquire)
{
    uint64_t value = 0x606060606060606;
    uint64_t result =
        SI<uint64_t>::FetchOr(value, 0x303030303030303, Acquire());
    EXPECT_EQ(value, 0x707070707070707ull);
    EXPECT_EQ(result, 0x606060606060606ull);
}

TEST(AtomicTests, IntrinSelect64FetchOrRelease)
{
    uint64_t value = 0x606060606060606;
    uint64_t result =
        SI<uint64_t>::FetchOr(value, 0x303030303030303, Release());
    EXPECT_EQ(value, 0x707070707070707ull);
    EXPECT_EQ(result, 0x606060606060606ull);
}

TEST(AtomicTests, IntrinSelect64FetchOrAcqRel)
{
    uint64_t value = 0x606060606060606;
    uint64_t result = SI<uint64_t>::FetchOr(value, 0x303030303030303, AcqRel());
    EXPECT_EQ(value, 0x707070707070707ull);
    EXPECT_EQ(result, 0x606060606060606ull);
}

TEST(AtomicTests, IntrinSelect64FetchOrSeqCst)
{
    uint64_t value = 0x606060606060606;
    uint64_t result = SI<uint64_t>::FetchOr(value, 0x303030303030303, SeqCst());
    EXPECT_EQ(value, 0x707070707070707ull);
    EXPECT_EQ(result, 0x606060606060606ull);
}

TEST(AtomicTests, IntrinSelect8FetchXorRelaxed)
{
    uint8_t value = 6;
    uint8_t result = SI<uint8_t>::FetchXor(value, 3, Relaxed());
    EXPECT_EQ(value, 5);
    EXPECT_EQ(result, 6);
}

TEST(AtomicTests, IntrinSelect8FetchXorConsume)
{
    uint8_t value = 6;
    uint8_t result = SI<uint8_t>::FetchXor(value, 3, Consume());
    EXPECT_EQ(value, 5);
    EXPECT_EQ(result, 6);
}

TEST(AtomicTests, IntrinSelect8FetchXorAcquire)
{
    uint8_t value = 6;
    uint8_t result = SI<uint8_t>::FetchXor(value, 3, Acquire());
    EXPECT_EQ(value, 5);
    EXPECT_EQ(result, 6);
}

TEST(AtomicTests, IntrinSelect8FetchXorRelease)
{
    uint8_t value = 6;
    uint8_t result = SI<uint8_t>::FetchXor(value, 3, Release());
    EXPECT_EQ(value, 5);
    EXPECT_EQ(result, 6);
}

TEST(AtomicTests, IntrinSelect8FetchXorAcqRel)
{
    uint8_t value = 6;
    uint8_t result = SI<uint8_t>::FetchXor(value, 3, AcqRel());
    EXPECT_EQ(value, 5);
    EXPECT_EQ(result, 6);
}

TEST(AtomicTests, IntrinSelect8FetchXorSeqCst)
{
    uint8_t value = 6;
    uint8_t result = SI<uint8_t>::FetchXor(value, 3, SeqCst());
    EXPECT_EQ(value, 5);
    EXPECT_EQ(result, 6);
}

TEST(AtomicTests, IntrinSelect16FetchXorRelaxed)
{
    uint16_t value = 0x606;
    uint16_t result = SI<uint16_t>::FetchXor(value, 0x303, Relaxed());
    EXPECT_EQ(value, 0x505);
    EXPECT_EQ(result, 0x606);
}

TEST(AtomicTests, IntrinSelect16FetchXorConsume)
{
    uint16_t value = 0x606;
    uint16_t result = SI<uint16_t>::FetchXor(value, 0x303, Consume());
    EXPECT_EQ(value, 0x505);
    EXPECT_EQ(result, 0x606);
}

TEST(AtomicTests, IntrinSelect16FetchXorAcquire)
{
    uint16_t value = 0x606;
    uint16_t result = SI<uint16_t>::FetchXor(value, 0x303, Acquire());
    EXPECT_EQ(value, 0x505);
    EXPECT_EQ(result, 0x606);
}

TEST(AtomicTests, IntrinSelect16FetchXorRelease)
{
    uint16_t value = 0x606;
    uint16_t result = SI<uint16_t>::FetchXor(value, 0x303, Release());
    EXPECT_EQ(value, 0x505);
    EXPECT_EQ(result, 0x606);
}

TEST(AtomicTests, IntrinSelect16FetchXorAcqRel)
{
    uint16_t value = 0x606;
    uint16_t result = SI<uint16_t>::FetchXor(value, 0x303, AcqRel());
    EXPECT_EQ(value, 0x505);
    EXPECT_EQ(result, 0x606);
}

TEST(AtomicTests, IntrinSelect16FetchXorSeqCst)
{
    uint16_t value = 0x606;
    uint16_t result = SI<uint16_t>::FetchXor(value, 0x303, SeqCst());
    EXPECT_EQ(value, 0x505);
    EXPECT_EQ(result, 0x606);
}

TEST(AtomicTests, IntrinSelect32FetchXorRelaxed)
{
    uint32_t value = 0x6060606;
    uint32_t result = SI<uint32_t>::FetchXor(value, 0x3030303, Relaxed());
    EXPECT_EQ(value, 0x5050505u);
    EXPECT_EQ(result, 0x6060606u);
}

TEST(AtomicTests, IntrinSelect32FetchXorConsume)
{
    uint32_t value = 0x6060606;
    uint32_t result = SI<uint32_t>::FetchXor(value, 0x3030303, Consume());
    EXPECT_EQ(value, 0x5050505u);
    EXPECT_EQ(result, 0x6060606u);
}

TEST(AtomicTests, IntrinSelect32FetchXorAcquire)
{
    uint32_t value = 0x6060606;
    uint32_t result = SI<uint32_t>::FetchXor(value, 0x3030303, Acquire());
    EXPECT_EQ(value, 0x5050505u);
    EXPECT_EQ(result, 0x6060606u);
}

TEST(AtomicTests, IntrinSelect32FetchXorRelease)
{
    uint32_t value = 0x6060606;
    uint32_t result = SI<uint32_t>::FetchXor(value, 0x3030303, Release());
    EXPECT_EQ(value, 0x5050505u);
    EXPECT_EQ(result, 0x6060606u);
}

TEST(AtomicTests, IntrinSelect32FetchXorAcqRel)
{
    uint32_t value = 0x6060606;
    uint32_t result = SI<uint32_t>::FetchXor(value, 0x3030303, AcqRel());
    EXPECT_EQ(value, 0x5050505u);
    EXPECT_EQ(result, 0x6060606u);
}

TEST(AtomicTests, IntrinSelect32FetchXorSeqCst)
{
    uint32_t value = 0x6060606;
    uint32_t result = SI<uint32_t>::FetchXor(value, 0x3030303, SeqCst());
    EXPECT_EQ(value, 0x5050505u);
    EXPECT_EQ(result, 0x6060606u);
}

TEST(AtomicTests, IntrinSelect64FetchXorRelaxed)
{
    uint64_t value = 0x606060606060606;
    uint64_t result =
        SI<uint64_t>::FetchXor(value, 0x303030303030303, Relaxed());
    EXPECT_EQ(value, 0x505050505050505ull);
    EXPECT_EQ(result, 0x606060606060606ull);
}

TEST(AtomicTests, IntrinSelect64FetchXorConsume)
{
    uint64_t value = 0x606060606060606;
    uint64_t result =
        SI<uint64_t>::FetchXor(value, 0x303030303030303, Consume());
    EXPECT_EQ(value, 0x505050505050505ull);
    EXPECT_EQ(result, 0x606060606060606ull);
}

TEST(AtomicTests, IntrinSelect64FetchXorAcquire)
{
    uint64_t value = 0x606060606060606;
    uint64_t result =
        SI<uint64_t>::FetchXor(value, 0x303030303030303, Acquire());
    EXPECT_EQ(value, 0x505050505050505ull);
    EXPECT_EQ(result, 0x606060606060606ull);
}

TEST(AtomicTests, IntrinSelect64FetchXorRelease)
{
    uint64_t value = 0x606060606060606;
    uint64_t result =
        SI<uint64_t>::FetchXor(value, 0x303030303030303, Release());
    EXPECT_EQ(value, 0x505050505050505ull);
    EXPECT_EQ(result, 0x606060606060606ull);
}

TEST(AtomicTests, IntrinSelect64FetchXorAcqRel)
{
    uint64_t value = 0x606060606060606;
    uint64_t result =
        SI<uint64_t>::FetchXor(value, 0x303030303030303, AcqRel());
    EXPECT_EQ(value, 0x505050505050505ull);
    EXPECT_EQ(result, 0x606060606060606ull);
}

TEST(AtomicTests, IntrinSelect64FetchXorSeqCst)
{
    uint64_t value = 0x606060606060606;
    uint64_t result =
        SI<uint64_t>::FetchXor(value, 0x303030303030303, SeqCst());
    EXPECT_EQ(value, 0x505050505050505ull);
    EXPECT_EQ(result, 0x606060606060606ull);
}
