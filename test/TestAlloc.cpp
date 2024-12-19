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

const uint32_t StatefulAllocator::k_BadState;
const uint32_t StatefulCountingAllocator::k_BadState;

uint32_t CountingAllocator::g_FreeCount = 0;
uint32_t CountingAllocator::g_AllocCount = 0;
size_t CountingAllocator::g_FreeBytesCount = 0;
size_t CountingAllocator::g_AllocBytesCount = 0;
uint32_t StatefulCountingAllocator::g_FreeCount = 0;
uint32_t StatefulCountingAllocator::g_AllocCount = 0;
size_t StatefulCountingAllocator::g_FreeBytesCount = 0;
size_t StatefulCountingAllocator::g_AllocBytesCount = 0;

} // namespace radtest
