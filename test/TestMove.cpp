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

#include "test/TestMove.h"

namespace radtest
{

static uint32_t g_MoveCount = 0;

MoveTester::MoveTester(const MoveTester& other)
    : value(other.value)
{
}

MoveTester::MoveTester(MoveTester&& other)
    : value(other.value)
{
    other.value = 0xDEADBEEF;
    g_MoveCount++;
}

MoveTester& MoveTester::operator=(const MoveTester& other)
{
    value = other.value;
    return *this;
}

MoveTester& MoveTester::operator=(MoveTester&& other)
{
    value = other.value;
    other.value = 0xDEADBEEF;
    g_MoveCount++;
    return *this;
}

uint32_t MoveTester::MoveCount()
{
    return g_MoveCount;
}

void MoveTester::ResetCounts()
{
    g_MoveCount = 0;
}

}; // namespace radtest
