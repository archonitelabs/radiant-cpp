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
#include "radiant/Result.h"

#include <stdint.h>

namespace rad
{

enum class Error : uint16_t
{
    Unsuccessful = 1,
    NoMemory,
    LockNotGranted,
    IntegerOverflow,
    OutOfRange,
    InvalidAddress,
};

template <typename T>
using Res = Result<T, Error>;

struct EmptyOkType
{
};

using Err = Res<EmptyOkType>;

RAD_INLINE_VAR constexpr EmptyOkType NoError{};

RAD_INLINE_VAR constexpr ResultOkTagType ResOkTag{};
RAD_INLINE_VAR constexpr ResultErrTagType ResErrTag{};
RAD_INLINE_VAR constexpr ResultEmptyTagType ResVoidTag{};
RAD_INLINE_VAR constexpr ResultEmptyTagType ResEmptyTag{};

template <typename T>
using ResOk = ResultOk<T>;

template <typename T>
using ResErr = ResultErr<T>;

inline Err ErrIfNull(void* ptr, Error e = Error::NoMemory)
{
    if (ptr == nullptr)
    {
        return e;
    }
    return EmptyOkType{};
}

} // namespace rad
