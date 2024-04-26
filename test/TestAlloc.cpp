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

uint32_t StaticCountingAllocatorImpl::freeCount = 0;
uint32_t StaticCountingAllocatorImpl::allocCount = 0;
uint32_t StaticCountingAllocatorImpl::reallocCount = 0;
uint32_t StaticCountingAllocatorImpl::freeBytesCount = 0;
uint32_t StaticCountingAllocatorImpl::allocBytesCount = 0;
uint32_t StaticCountingAllocatorImpl::reallocBytesCount = 0;

void StaticCountingAllocatorImpl::Free(void* ptr) noexcept
{
    ++freeCount;
    free(ptr);
}

void* StaticCountingAllocatorImpl::Alloc(uint32_t size) noexcept
{
    ++allocCount;
    return malloc(size);
}

void* StaticCountingAllocatorImpl::Realloc(void* ptr, uint32_t size) noexcept
{
    ++reallocCount;
    return realloc(ptr, size);
}

void StaticCountingAllocatorImpl::FreeBytes(void* ptr) noexcept
{
    ++freeBytesCount;
    free(ptr);
}

void* StaticCountingAllocatorImpl::AllocBytes(uint32_t size) noexcept
{
    ++allocBytesCount;
    return malloc(size);
}

void* StaticCountingAllocatorImpl::ReallocBytes(void* ptr,
                                                uint32_t size) noexcept
{
    ++reallocBytesCount;
    return realloc(ptr, size);
}

} // namespace radtest
