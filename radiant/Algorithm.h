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

#pragma once

#include "radiant/TotallyRad.h"
// RAD_S_ASSERT_NOTHROW_MOVE_T uses TypeTraits.h
#include "radiant/TypeTraits.h" // NOLINT(misc-include-cleaner)
// rad::Move needs radiant/Utility.h
#include "radiant/Utility.h" // NOLINT(misc-include-cleaner)

namespace rad
{
template <typename T>
void Swap(T& a, T& b) noexcept
{
    RAD_S_ASSERT_NOTHROW_MOVE_T(T);
    T tmp = Move(a);
    a = Move(b);
    b = Move(tmp);
}
} // namespace rad
