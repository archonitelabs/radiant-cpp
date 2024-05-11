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

#include "test/TestAlloc.h"

#include <stdlib.h>

namespace radtest
{

uint32_t CountingAllocatorImpl::g_FreeCount = 0;
uint32_t CountingAllocatorImpl::g_AllocCount = 0;
uint32_t CountingAllocatorImpl::g_ReallocCount = 0;
uint32_t CountingAllocatorImpl::g_FreeBytesCount = 0;
uint32_t CountingAllocatorImpl::g_AllocBytesCount = 0;
uint32_t CountingAllocatorImpl::g_ReallocBytesCount = 0;

void CountingAllocatorImpl::Free(void* ptr) noexcept
{
    ++g_FreeCount;
    free(ptr);
}

void* CountingAllocatorImpl::Alloc(uint32_t size) noexcept
{
    ++g_AllocCount;
    return malloc(size);
}

void* CountingAllocatorImpl::Realloc(void* ptr, uint32_t size) noexcept
{
    ++g_ReallocCount;
    return realloc(ptr, size);
}

void CountingAllocatorImpl::FreeBytes(void* ptr) noexcept
{
    ++g_FreeBytesCount;
    free(ptr);
}

void* CountingAllocatorImpl::AllocBytes(uint32_t size) noexcept
{
    ++g_AllocBytesCount;
    return malloc(size);
}

void* CountingAllocatorImpl::ReallocBytes(void* ptr, uint32_t size) noexcept
{
    ++g_ReallocBytesCount;
    return realloc(ptr, size);
}

uint32_t CountingAllocatorImpl::FreeCount() noexcept
{
    return g_FreeCount;
}

uint32_t CountingAllocatorImpl::AllocCount() noexcept
{
    return g_AllocCount;
}

uint32_t CountingAllocatorImpl::ReallocCount() noexcept
{
    return g_ReallocCount;
}

uint32_t CountingAllocatorImpl::FreeBytesCount() noexcept
{
    return g_FreeBytesCount;
}

uint32_t CountingAllocatorImpl::AllocBytesCount() noexcept
{
    return g_AllocBytesCount;
}

uint32_t CountingAllocatorImpl::ReallocBytesCount() noexcept
{
    return g_ReallocBytesCount;
}

void CountingAllocatorImpl::ResetCounts() noexcept
{
    g_FreeCount = 0;
    g_AllocCount = 0;
    g_ReallocCount = 0;
    g_FreeBytesCount = 0;
    g_AllocBytesCount = 0;
    g_ReallocBytesCount = 0;
}

bool CountingAllocatorImpl::VerifyCounts() noexcept
{
    return (g_AllocCount == g_FreeCount);
}

bool CountingAllocatorImpl::VerifyCounts(uint32_t expectedAllocs,
                                         uint32_t expectedFrees) noexcept
{
    return (g_AllocCount == expectedAllocs) && (g_FreeCount == expectedFrees);
}

} // namespace radtest
