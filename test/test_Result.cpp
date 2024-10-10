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

//
// Disable nothrow assertions for unit testing
//
#define RAD_ENABLE_NOTHROW_ASSERTIONS      0
#define RAD_ENABLE_NOTHROW_DTOR_ASSERTIONS 0
#define RAD_ENABLE_NOTHROW_MOVE_ASSERTIONS 0

#include "radiant/Result.h"
#include "radiant/Utility.h"

#include "gtest/gtest.h"
#include "test/TestMove.h"
#include "test/TestThrow.h"
#include <string>
#include <utility>

namespace testnoexcept
{

//
// ResultContainer<[throwing]>
//
using ThrowingResultContainer = rad::TypeWrapper<radtest::ThrowingObject>;

// clang-format off

RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowingResultContainer>().~TypeWrapper()));
RAD_S_ASSERT(!noexcept(ThrowingResultContainer()));
RAD_S_ASSERT(!noexcept(ThrowingResultContainer(::rad::DeclVal<const radtest::ThrowingObject&>())));
RAD_S_ASSERT(!noexcept(ThrowingResultContainer(::rad::DeclVal<radtest::ThrowingObject&&>())));
RAD_S_ASSERT(!noexcept(ThrowingResultContainer(::rad::DeclVal<const ThrowingResultContainer&>())));
RAD_S_ASSERT(!noexcept(ThrowingResultContainer(::rad::DeclVal<ThrowingResultContainer&&>())));
RAD_S_ASSERT(!noexcept(ThrowingResultContainer(1)));
#if RAD_ENABLE_STD
RAD_S_ASSERT(!noexcept(ThrowingResultContainer({ 1 })));
#endif
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowingResultContainer>().operator=(::rad::DeclVal<const radtest::ThrowingObject&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowingResultContainer>().operator=(::rad::DeclVal<radtest::ThrowingObject&&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowingResultContainer>().operator=(::rad::DeclVal<const ThrowingResultContainer&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowingResultContainer>().operator=(::rad::DeclVal<ThrowingResultContainer&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowingResultContainer>().Get()));

// clang-format on

//
// ResultContainer<[non-throwing]>
//

using NonThrowingResultContainer = rad::TypeWrapper<radtest::NonThrowingObject>;

// clang-format off

RAD_S_ASSERT(noexcept(::rad::DeclVal<NonThrowingResultContainer>().~TypeWrapper()));
RAD_S_ASSERT(noexcept(NonThrowingResultContainer()));
RAD_S_ASSERT(noexcept(NonThrowingResultContainer(::rad::DeclVal<const radtest::NonThrowingObject&>())));
RAD_S_ASSERT(noexcept(NonThrowingResultContainer(::rad::DeclVal<radtest::NonThrowingObject&&>())));
RAD_S_ASSERT(noexcept(NonThrowingResultContainer(::rad::DeclVal<const NonThrowingResultContainer&>())));
RAD_S_ASSERT(noexcept(NonThrowingResultContainer(::rad::DeclVal<NonThrowingResultContainer&&>())));
RAD_S_ASSERT(noexcept(NonThrowingResultContainer(1)));
#if RAD_ENABLE_STD
RAD_S_ASSERT(noexcept(NonThrowingResultContainer({ 1 })));
#endif
RAD_S_ASSERT(noexcept(::rad::DeclVal<NonThrowingResultContainer>().operator=(::rad::DeclVal<const radtest::NonThrowingObject&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NonThrowingResultContainer>().operator=(::rad::DeclVal<radtest::NonThrowingObject&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NonThrowingResultContainer>().operator=(::rad::DeclVal<const NonThrowingResultContainer&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NonThrowingResultContainer>().operator=(::rad::DeclVal<NonThrowingResultContainer&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NonThrowingResultContainer>().Get()));

// clang-format on

//
// ResultStorage<[throwing], [throwing], true>
//

using ThrowThrowResultStorageTriv =
    rad::detail::ResultStorage<radtest::ThrowingObjectTrivDtor,
                               radtest::ThrowingObjectTrivDtorTwo>;

// clang-format off

RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowThrowResultStorageTriv>().~ResultStorage()));
RAD_S_ASSERT(noexcept(ThrowThrowResultStorageTriv()));
RAD_S_ASSERT(noexcept(ThrowThrowResultStorageTriv(rad::ResultEmptyTag)));
RAD_S_ASSERT(!noexcept(ThrowThrowResultStorageTriv(rad::ResultOkTag, ::rad::DeclVal<const radtest::ThrowingObjectTrivDtor&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResultStorageTriv(rad::ResultOkTag, ::rad::DeclVal<radtest::ThrowingObjectTrivDtor&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResultStorageTriv(rad::ResultOkTag, ::rad::DeclVal<radtest::ThrowingObjectTrivDtor&&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResultStorageTriv(rad::ResultErrTag, ::rad::DeclVal<const radtest::ThrowingObjectTrivDtorTwo&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResultStorageTriv(rad::ResultErrTag, ::rad::DeclVal<radtest::ThrowingObjectTrivDtorTwo&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResultStorageTriv(rad::ResultErrTag, ::rad::DeclVal<radtest::ThrowingObjectTrivDtorTwo&&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResultStorageTriv(rad::ResultOkTag, ::rad::DeclVal<const rad::ResultOk<radtest::ThrowingObjectTrivDtor>&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResultStorageTriv(rad::ResultOkTag, ::rad::DeclVal<rad::ResultOk<radtest::ThrowingObjectTrivDtor>&&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResultStorageTriv(rad::ResultErrTag, ::rad::DeclVal<const rad::ResultErr<radtest::ThrowingObjectTrivDtorTwo>&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResultStorageTriv(rad::ResultErrTag, ::rad::DeclVal<rad::ResultErr<radtest::ThrowingObjectTrivDtorTwo>&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowThrowResultStorageTriv>().Destruct()));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResultStorageTriv>().Construct(rad::ResultOkTag, 1)));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResultStorageTriv>().Construct(rad::ResultOkTag, std::initializer_list<int>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResultStorageTriv>().Construct(rad::ResultErrTag, 1)));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResultStorageTriv>().Construct(rad::ResultErrTag, std::initializer_list<int>())));

// clang-format on

//
// ResultStorage<[non-throwing], [non-throwing], true>
//

using NoThrowNoThrowResultStorageTriv =
    rad::detail::ResultStorage<radtest::NonThrowingObjectTrivDtor,
                               radtest::NonThrowingObjectTrivDtorTwo>;

// clang-format off

RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResultStorageTriv>().~ResultStorage()));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageTriv()));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageTriv(rad::ResultEmptyTag)));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageTriv(rad::ResultOkTag, ::rad::DeclVal<const radtest::NonThrowingObjectTrivDtor&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageTriv(rad::ResultOkTag, ::rad::DeclVal<radtest::NonThrowingObjectTrivDtor&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageTriv(rad::ResultOkTag, ::rad::DeclVal<radtest::NonThrowingObjectTrivDtor&&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageTriv(rad::ResultErrTag, ::rad::DeclVal<const radtest::NonThrowingObjectTrivDtorTwo&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageTriv(rad::ResultErrTag, ::rad::DeclVal<radtest::NonThrowingObjectTrivDtorTwo&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageTriv(rad::ResultErrTag, ::rad::DeclVal<radtest::NonThrowingObjectTrivDtorTwo&&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageTriv(rad::ResultOkTag, ::rad::DeclVal<const rad::ResultOk<radtest::NonThrowingObjectTrivDtor>&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageTriv(rad::ResultOkTag, ::rad::DeclVal<rad::ResultOk<radtest::NonThrowingObjectTrivDtor>&&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageTriv(rad::ResultErrTag, ::rad::DeclVal<const rad::ResultErr<radtest::NonThrowingObjectTrivDtorTwo>&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageTriv(rad::ResultErrTag, ::rad::DeclVal<rad::ResultErr<radtest::NonThrowingObjectTrivDtorTwo>&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResultStorageTriv>().Destruct()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResultStorageTriv>().Construct(rad::ResultOkTag, 1)));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResultStorageTriv>().Construct(rad::ResultOkTag, std::initializer_list<int>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResultStorageTriv>().Construct(rad::ResultErrTag, 1)));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResultStorageTriv>().Construct(rad::ResultErrTag, std::initializer_list<int>())));

// clang-format on

//
// ResultStorage<[throwing], [non-throwing], true>
//

using ThrowNoThrowResultStorageTriv =
    rad::detail::ResultStorage<radtest::ThrowingObjectTrivDtor,
                               radtest::NonThrowingObjectTrivDtor>;

// clang-format off

RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResultStorageTriv>().~ResultStorage()));
RAD_S_ASSERT(noexcept(ThrowNoThrowResultStorageTriv()));
RAD_S_ASSERT(noexcept(ThrowNoThrowResultStorageTriv(rad::ResultEmptyTag)));
RAD_S_ASSERT(!noexcept(ThrowNoThrowResultStorageTriv(rad::ResultOkTag, ::rad::DeclVal<const radtest::ThrowingObjectTrivDtor&>())));
RAD_S_ASSERT(!noexcept(ThrowNoThrowResultStorageTriv(rad::ResultOkTag, ::rad::DeclVal<radtest::ThrowingObjectTrivDtor&>())));
RAD_S_ASSERT(!noexcept(ThrowNoThrowResultStorageTriv(rad::ResultOkTag, ::rad::DeclVal<radtest::ThrowingObjectTrivDtor&&>())));
RAD_S_ASSERT(noexcept(ThrowNoThrowResultStorageTriv(rad::ResultErrTag, ::rad::DeclVal<const radtest::NonThrowingObjectTrivDtor&>())));
RAD_S_ASSERT(noexcept(ThrowNoThrowResultStorageTriv(rad::ResultErrTag, ::rad::DeclVal<radtest::NonThrowingObjectTrivDtor&>())));
RAD_S_ASSERT(noexcept(ThrowNoThrowResultStorageTriv(rad::ResultErrTag, ::rad::DeclVal<radtest::NonThrowingObjectTrivDtor&&>())));
RAD_S_ASSERT(!noexcept(ThrowNoThrowResultStorageTriv(rad::ResultOkTag, ::rad::DeclVal<const rad::ResultOk<radtest::ThrowingObjectTrivDtor>&>())));
RAD_S_ASSERT(!noexcept(ThrowNoThrowResultStorageTriv(rad::ResultOkTag, ::rad::DeclVal<rad::ResultOk<radtest::ThrowingObjectTrivDtor>&&>())));
RAD_S_ASSERT(noexcept(ThrowNoThrowResultStorageTriv(rad::ResultErrTag, ::rad::DeclVal<const rad::ResultErr<radtest::NonThrowingObjectTrivDtor>&>())));
RAD_S_ASSERT(noexcept(ThrowNoThrowResultStorageTriv(rad::ResultErrTag, ::rad::DeclVal<rad::ResultErr<radtest::NonThrowingObjectTrivDtor>&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResultStorageTriv>().Destruct()));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowNoThrowResultStorageTriv>().Construct(rad::ResultOkTag, 1)));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowNoThrowResultStorageTriv>().Construct(rad::ResultOkTag, std::initializer_list<int>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResultStorageTriv>().Construct(rad::ResultErrTag, 1)));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResultStorageTriv>().Construct(rad::ResultErrTag, std::initializer_list<int>())));

// clang-format on

//
// ResultStorage<[non-throwing], [throwing], true>
//

using NoThrowThrowResultStorageTriv =
    rad::detail::ResultStorage<radtest::NonThrowingObjectTrivDtor,
                               radtest::ThrowingObjectTrivDtor>;

// clang-format off

RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResultStorageTriv>().~ResultStorage()));
RAD_S_ASSERT(noexcept(NoThrowThrowResultStorageTriv()));
RAD_S_ASSERT(noexcept(NoThrowThrowResultStorageTriv(rad::ResultEmptyTag)));
RAD_S_ASSERT(noexcept(NoThrowThrowResultStorageTriv(rad::ResultOkTag, ::rad::DeclVal<const radtest::NonThrowingObjectTrivDtor&>())));
RAD_S_ASSERT(noexcept(NoThrowThrowResultStorageTriv(rad::ResultOkTag, ::rad::DeclVal<radtest::NonThrowingObjectTrivDtor&>())));
RAD_S_ASSERT(noexcept(NoThrowThrowResultStorageTriv(rad::ResultOkTag, ::rad::DeclVal<radtest::NonThrowingObjectTrivDtor&&>())));
RAD_S_ASSERT(!noexcept(NoThrowThrowResultStorageTriv(rad::ResultErrTag, ::rad::DeclVal<const radtest::ThrowingObjectTrivDtor&>())));
RAD_S_ASSERT(!noexcept(NoThrowThrowResultStorageTriv(rad::ResultErrTag, ::rad::DeclVal<radtest::ThrowingObjectTrivDtor&>())));
RAD_S_ASSERT(!noexcept(NoThrowThrowResultStorageTriv(rad::ResultErrTag, ::rad::DeclVal<radtest::ThrowingObjectTrivDtor&&>())));
RAD_S_ASSERT(noexcept(NoThrowThrowResultStorageTriv(rad::ResultOkTag, ::rad::DeclVal<const rad::ResultOk<radtest::NonThrowingObjectTrivDtor>&>())));
RAD_S_ASSERT(noexcept(NoThrowThrowResultStorageTriv(rad::ResultOkTag, ::rad::DeclVal<rad::ResultOk<radtest::NonThrowingObjectTrivDtor>&&>())));
RAD_S_ASSERT(!noexcept(NoThrowThrowResultStorageTriv(rad::ResultErrTag, ::rad::DeclVal<const rad::ResultErr<radtest::ThrowingObjectTrivDtor>&>())));
RAD_S_ASSERT(!noexcept(NoThrowThrowResultStorageTriv(rad::ResultErrTag, ::rad::DeclVal<rad::ResultErr<radtest::ThrowingObjectTrivDtor>&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResultStorageTriv>().Destruct()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResultStorageTriv>().Construct(rad::ResultOkTag, 1)));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResultStorageTriv>().Construct(rad::ResultOkTag, std::initializer_list<int>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<NoThrowThrowResultStorageTriv>().Construct(rad::ResultErrTag, 1)));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<NoThrowThrowResultStorageTriv>().Construct(rad::ResultErrTag, std::initializer_list<int>())));

// clang-format on

//
// ResultStorage<[throwing], [throwing], false>
//

using ThrowThrowResultStorageNonTriv =
    rad::detail::ResultStorage<radtest::ThrowingObject,
                               radtest::ThrowingObjectTwo>;

// clang-format off

RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowThrowResultStorageNonTriv>().~ResultStorage()));
RAD_S_ASSERT(noexcept(ThrowThrowResultStorageNonTriv()));
RAD_S_ASSERT(noexcept(ThrowThrowResultStorageNonTriv(rad::ResultEmptyTag)));
RAD_S_ASSERT(!noexcept(ThrowThrowResultStorageNonTriv(rad::ResultOkTag, ::rad::DeclVal<const radtest::ThrowingObject&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResultStorageNonTriv(rad::ResultOkTag, ::rad::DeclVal<radtest::ThrowingObject&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResultStorageNonTriv(rad::ResultOkTag, ::rad::DeclVal<radtest::ThrowingObject&&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResultStorageNonTriv(rad::ResultErrTag, ::rad::DeclVal<const radtest::ThrowingObjectTwo&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResultStorageNonTriv(rad::ResultErrTag, ::rad::DeclVal<radtest::ThrowingObjectTwo&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResultStorageNonTriv(rad::ResultErrTag, ::rad::DeclVal<radtest::ThrowingObjectTwo&&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResultStorageNonTriv(rad::ResultOkTag, ::rad::DeclVal<const rad::ResultOk<radtest::ThrowingObject>&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResultStorageNonTriv(rad::ResultOkTag, ::rad::DeclVal<rad::ResultOk<radtest::ThrowingObject>&&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResultStorageNonTriv(rad::ResultErrTag, ::rad::DeclVal<const rad::ResultErr<radtest::ThrowingObjectTwo>&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResultStorageNonTriv(rad::ResultErrTag, ::rad::DeclVal<rad::ResultErr<radtest::ThrowingObjectTwo>&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowThrowResultStorageNonTriv>().Destruct()));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResultStorageNonTriv>().Construct(rad::ResultOkTag, 1)));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResultStorageNonTriv>().Construct(rad::ResultOkTag, std::initializer_list<int>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResultStorageNonTriv>().Construct(rad::ResultErrTag, 1)));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResultStorageNonTriv>().Construct(rad::ResultErrTag, std::initializer_list<int>())));

// clang-format on

//
// ResultStorage<[non-throwing], [non-throwing], false>
//

using NoThrowNoThrowResultStorageNonTriv =
    rad::detail::ResultStorage<radtest::NonThrowingObject,
                               radtest::NonThrowingObjectTwo>;

// clang-format off

RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResultStorageNonTriv>().~ResultStorage()));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageNonTriv()));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageNonTriv(rad::ResultEmptyTag)));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageNonTriv(rad::ResultOkTag, ::rad::DeclVal<const radtest::NonThrowingObject&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageNonTriv(rad::ResultOkTag, ::rad::DeclVal<radtest::NonThrowingObject&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageNonTriv(rad::ResultOkTag, ::rad::DeclVal<radtest::NonThrowingObject&&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageNonTriv(rad::ResultErrTag, ::rad::DeclVal<const radtest::NonThrowingObjectTwo&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageNonTriv(rad::ResultErrTag, ::rad::DeclVal<radtest::NonThrowingObjectTwo&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageNonTriv(rad::ResultErrTag, ::rad::DeclVal<radtest::NonThrowingObjectTwo&&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageNonTriv(rad::ResultOkTag, ::rad::DeclVal<const rad::ResultOk<radtest::NonThrowingObject>&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageNonTriv(rad::ResultOkTag, ::rad::DeclVal<rad::ResultOk<radtest::NonThrowingObject>&&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageNonTriv(rad::ResultErrTag, ::rad::DeclVal<const rad::ResultErr<radtest::NonThrowingObjectTwo>&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResultStorageNonTriv(rad::ResultErrTag, ::rad::DeclVal<rad::ResultErr<radtest::NonThrowingObjectTwo>&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResultStorageNonTriv>().Destruct()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResultStorageNonTriv>().Construct(rad::ResultOkTag, 1)));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResultStorageNonTriv>().Construct(rad::ResultOkTag, std::initializer_list<int>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResultStorageNonTriv>().Construct(rad::ResultErrTag, 1)));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResultStorageNonTriv>().Construct(rad::ResultErrTag, std::initializer_list<int>())));

// clang-format on

//
// ResultStorageNon<[throwing], [non-throwing], false>
//

using ThrowNoThrowResultStorageNonTriv =
    rad::detail::ResultStorage<radtest::ThrowingObject,
                               radtest::NonThrowingObject>;

// clang-format off

RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResultStorageNonTriv>().~ResultStorage()));
RAD_S_ASSERT(noexcept(ThrowNoThrowResultStorageNonTriv()));
RAD_S_ASSERT(noexcept(ThrowNoThrowResultStorageNonTriv(rad::ResultEmptyTag)));
RAD_S_ASSERT(!noexcept(ThrowNoThrowResultStorageNonTriv(rad::ResultOkTag, ::rad::DeclVal<const radtest::ThrowingObject&>())));
RAD_S_ASSERT(!noexcept(ThrowNoThrowResultStorageNonTriv(rad::ResultOkTag, ::rad::DeclVal<radtest::ThrowingObject&>())));
RAD_S_ASSERT(!noexcept(ThrowNoThrowResultStorageNonTriv(rad::ResultOkTag, ::rad::DeclVal<radtest::ThrowingObject&&>())));
RAD_S_ASSERT(noexcept(ThrowNoThrowResultStorageNonTriv(rad::ResultErrTag, ::rad::DeclVal<const radtest::NonThrowingObject&>())));
RAD_S_ASSERT(noexcept(ThrowNoThrowResultStorageNonTriv(rad::ResultErrTag, ::rad::DeclVal<radtest::NonThrowingObject&>())));
RAD_S_ASSERT(noexcept(ThrowNoThrowResultStorageNonTriv(rad::ResultErrTag, ::rad::DeclVal<radtest::NonThrowingObject&&>())));
RAD_S_ASSERT(!noexcept(ThrowNoThrowResultStorageNonTriv(rad::ResultOkTag, ::rad::DeclVal<const rad::ResultOk<radtest::ThrowingObject>&>())));
RAD_S_ASSERT(!noexcept(ThrowNoThrowResultStorageNonTriv(rad::ResultOkTag, ::rad::DeclVal<rad::ResultOk<radtest::ThrowingObject>&&>())));
RAD_S_ASSERT(noexcept(ThrowNoThrowResultStorageNonTriv(rad::ResultErrTag, ::rad::DeclVal<const rad::ResultErr<radtest::NonThrowingObject>&>())));
RAD_S_ASSERT(noexcept(ThrowNoThrowResultStorageNonTriv(rad::ResultErrTag, ::rad::DeclVal<rad::ResultErr<radtest::NonThrowingObject>&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResultStorageNonTriv>().Destruct()));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowNoThrowResultStorageNonTriv>().Construct(rad::ResultOkTag, 1)));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowNoThrowResultStorageNonTriv>().Construct(rad::ResultOkTag, std::initializer_list<int>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResultStorageNonTriv>().Construct(rad::ResultErrTag, 1)));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResultStorageNonTriv>().Construct(rad::ResultErrTag, std::initializer_list<int>())));

// clang-format on

//
// ResultStorage<[non-throwing], [throwing], false>
//

using NoThrowThrowResultStorageNonTriv =
    rad::detail::ResultStorage<radtest::NonThrowingObject,
                               radtest::ThrowingObject>;

// clang-format off

RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResultStorageNonTriv>().~ResultStorage()));
RAD_S_ASSERT(noexcept(NoThrowThrowResultStorageNonTriv()));
RAD_S_ASSERT(noexcept(NoThrowThrowResultStorageNonTriv(rad::ResultEmptyTag)));
RAD_S_ASSERT(noexcept(NoThrowThrowResultStorageNonTriv(rad::ResultOkTag, ::rad::DeclVal<const radtest::NonThrowingObject&>())));
RAD_S_ASSERT(noexcept(NoThrowThrowResultStorageNonTriv(rad::ResultOkTag, ::rad::DeclVal<radtest::NonThrowingObject&>())));
RAD_S_ASSERT(noexcept(NoThrowThrowResultStorageNonTriv(rad::ResultOkTag, ::rad::DeclVal<radtest::NonThrowingObject&&>())));
RAD_S_ASSERT(!noexcept(NoThrowThrowResultStorageNonTriv(rad::ResultErrTag, ::rad::DeclVal<const radtest::ThrowingObject&>())));
RAD_S_ASSERT(!noexcept(NoThrowThrowResultStorageNonTriv(rad::ResultErrTag, ::rad::DeclVal<radtest::ThrowingObject&>())));
RAD_S_ASSERT(!noexcept(NoThrowThrowResultStorageNonTriv(rad::ResultErrTag, ::rad::DeclVal<radtest::ThrowingObject&&>())));
RAD_S_ASSERT(noexcept(NoThrowThrowResultStorageNonTriv(rad::ResultOkTag, ::rad::DeclVal<const rad::ResultOk<radtest::NonThrowingObject>&>())));
RAD_S_ASSERT(noexcept(NoThrowThrowResultStorageNonTriv(rad::ResultOkTag, ::rad::DeclVal<rad::ResultOk<radtest::NonThrowingObject>&&>())));
RAD_S_ASSERT(!noexcept(NoThrowThrowResultStorageNonTriv(rad::ResultErrTag, ::rad::DeclVal<const rad::ResultErr<radtest::ThrowingObject>&>())));
RAD_S_ASSERT(!noexcept(NoThrowThrowResultStorageNonTriv(rad::ResultErrTag, ::rad::DeclVal<rad::ResultErr<radtest::ThrowingObject>&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResultStorageNonTriv>().Destruct()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResultStorageNonTriv>().Construct(rad::ResultOkTag, 1)));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResultStorageNonTriv>().Construct(rad::ResultOkTag, std::initializer_list<int>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<NoThrowThrowResultStorageNonTriv>().Construct(rad::ResultErrTag, 1)));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<NoThrowThrowResultStorageNonTriv>().Construct(rad::ResultErrTag, std::initializer_list<int>())));

// clang-format on

//
// Result<[throwing], [throwing]>
//

using ThrowThrowResult =
    rad::Result<radtest::ThrowingObject, radtest::ThrowingObjectTwo>;

// clang-format off

RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowThrowResult>().~Result()));
RAD_S_ASSERT(noexcept(ThrowThrowResult()));
RAD_S_ASSERT(noexcept(ThrowThrowResult(rad::ResultEmptyTag)));
RAD_S_ASSERT(!noexcept(ThrowThrowResult(::rad::DeclVal<const radtest::ThrowingObject&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResult(::rad::DeclVal<radtest::ThrowingObject&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResult(::rad::DeclVal<radtest::ThrowingObject&&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResult(::rad::DeclVal<const radtest::ThrowingObjectTwo&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResult(::rad::DeclVal<radtest::ThrowingObjectTwo&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResult(::rad::DeclVal<radtest::ThrowingObjectTwo&&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResult(::rad::DeclVal<const rad::ResultOk<radtest::ThrowingObject>&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResult(::rad::DeclVal<rad::ResultOk<radtest::ThrowingObject>&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResult(::rad::DeclVal<rad::ResultOk<radtest::ThrowingObject>&&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResult(::rad::DeclVal<const rad::ResultErr<radtest::ThrowingObjectTwo>&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResult(::rad::DeclVal<rad::ResultErr<radtest::ThrowingObjectTwo>&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResult(::rad::DeclVal<rad::ResultErr<radtest::ThrowingObjectTwo>&&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResult(rad::ResultOkTag, 1)));
RAD_S_ASSERT(!noexcept(ThrowThrowResult(rad::ResultErrTag, 1)));
#if RAD_ENABLE_STD
RAD_S_ASSERT(!noexcept(ThrowThrowResult(rad::ResultOkTag, { 1 })));
RAD_S_ASSERT(!noexcept(ThrowThrowResult(rad::ResultErrTag, { 1 })));
#endif
RAD_S_ASSERT(!noexcept(ThrowThrowResult(::rad::DeclVal<const ThrowThrowResult&>())));
RAD_S_ASSERT(!noexcept(ThrowThrowResult(::rad::DeclVal<ThrowThrowResult&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowThrowResult>().operator bool()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowThrowResult>().IsOk()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowThrowResult>().IsErr()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowThrowResult>().State()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowThrowResult>().Ok()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowThrowResult>().Err()));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().Or(1)));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().OnErr<int>(1)));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().OnOk<int>(1)));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().OnErr(::rad::DeclVal<std::string&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().OnErr(::rad::DeclVal<const std::string&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().OnOk(::rad::DeclVal<const std::string&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowThrowResult>().operator->()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowThrowResult>().operator*()));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().Assign(::rad::DeclVal<const ThrowThrowResult&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().operator=(::rad::DeclVal<const ThrowThrowResult&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().Assign(::rad::DeclVal<ThrowThrowResult&&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().operator=(::rad::DeclVal<ThrowThrowResult&&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().Assign(::rad::DeclVal<const radtest::ThrowingObject&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().operator=(::rad::DeclVal<const radtest::ThrowingObject&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().Assign(::rad::DeclVal<radtest::ThrowingObject&&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().operator=(::rad::DeclVal<radtest::ThrowingObject&&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().Assign(::rad::DeclVal<const radtest::ThrowingObjectTwo&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().operator=(::rad::DeclVal<const radtest::ThrowingObjectTwo&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().Assign(::rad::DeclVal<radtest::ThrowingObjectTwo&&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().operator=(::rad::DeclVal<radtest::ThrowingObjectTwo&&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().Assign(::rad::DeclVal<const rad::ResultOk<radtest::ThrowingObject>&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().operator=(::rad::DeclVal<const rad::ResultOk<radtest::ThrowingObject>&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().Assign(::rad::DeclVal<rad::ResultOk<radtest::ThrowingObject>&&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().operator=(::rad::DeclVal<rad::ResultOk<radtest::ThrowingObject>&&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().Assign(::rad::DeclVal<const rad::ResultErr<radtest::ThrowingObjectTwo>&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().operator=(::rad::DeclVal<const rad::ResultErr<radtest::ThrowingObjectTwo>&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().Assign(::rad::DeclVal<rad::ResultErr<radtest::ThrowingObjectTwo>&&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowThrowResult>().operator=(::rad::DeclVal<rad::ResultErr<radtest::ThrowingObjectTwo>&&>())));

//
// Result<[non-throwing], [throwing]>
//

using NoThrowThrowResult =
    rad::Result<radtest::NonThrowingObject, radtest::ThrowingObject>;

// clang-format off

RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResult>().~Result()));
RAD_S_ASSERT(noexcept(NoThrowThrowResult()));
RAD_S_ASSERT(noexcept(NoThrowThrowResult(rad::ResultEmptyTag)));
RAD_S_ASSERT(noexcept(NoThrowThrowResult(::rad::DeclVal<const radtest::NonThrowingObject&>())));
RAD_S_ASSERT(noexcept(NoThrowThrowResult(::rad::DeclVal<radtest::NonThrowingObject&>())));
RAD_S_ASSERT(noexcept(NoThrowThrowResult(::rad::DeclVal<radtest::NonThrowingObject&&>())));
RAD_S_ASSERT(!noexcept(NoThrowThrowResult(::rad::DeclVal<const radtest::ThrowingObject&>())));
RAD_S_ASSERT(!noexcept(NoThrowThrowResult(::rad::DeclVal<radtest::ThrowingObject&>())));
RAD_S_ASSERT(!noexcept(NoThrowThrowResult(::rad::DeclVal<radtest::ThrowingObject&&>())));
RAD_S_ASSERT(noexcept(NoThrowThrowResult(::rad::DeclVal<const rad::ResultOk<radtest::NonThrowingObject>&>())));
RAD_S_ASSERT(noexcept(NoThrowThrowResult(::rad::DeclVal<rad::ResultOk<radtest::NonThrowingObject>&>())));
RAD_S_ASSERT(noexcept(NoThrowThrowResult(::rad::DeclVal<rad::ResultOk<radtest::NonThrowingObject>&&>())));
RAD_S_ASSERT(!noexcept(NoThrowThrowResult(::rad::DeclVal<const rad::ResultErr<radtest::ThrowingObject>&>())));
RAD_S_ASSERT(!noexcept(NoThrowThrowResult(::rad::DeclVal<rad::ResultErr<radtest::ThrowingObject>&>())));
RAD_S_ASSERT(!noexcept(NoThrowThrowResult(::rad::DeclVal<rad::ResultErr<radtest::ThrowingObject>&&>())));
RAD_S_ASSERT(noexcept(NoThrowThrowResult(rad::ResultOkTag, 1)));
RAD_S_ASSERT(!noexcept(NoThrowThrowResult(rad::ResultErrTag, 1)));
#if RAD_ENABLE_STD
RAD_S_ASSERT(noexcept(NoThrowThrowResult(rad::ResultOkTag, { 1 })));
RAD_S_ASSERT(!noexcept(NoThrowThrowResult(rad::ResultErrTag, { 1 })));
#endif
RAD_S_ASSERT(!noexcept(NoThrowThrowResult(::rad::DeclVal<const NoThrowThrowResult&>())));
RAD_S_ASSERT(!noexcept(NoThrowThrowResult(::rad::DeclVal<NoThrowThrowResult&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResult>().operator bool()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResult>().IsOk()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResult>().IsErr()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResult>().State()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResult>().Ok()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResult>().Err()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResult>().Or(1)));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResult>().OnErr<int>(1)));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<NoThrowThrowResult>().OnOk<int>(1)));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResult>().OnErr(::rad::DeclVal<std::string&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResult>().OnErr(::rad::DeclVal<const std::string&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<NoThrowThrowResult>().OnOk(::rad::DeclVal<std::string&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<NoThrowThrowResult>().OnOk(::rad::DeclVal<const std::string&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResult>().operator->()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResult>().operator*()));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<NoThrowThrowResult>().Assign(::rad::DeclVal<const NoThrowThrowResult&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<NoThrowThrowResult>().operator=(::rad::DeclVal<const NoThrowThrowResult&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<NoThrowThrowResult>().Assign(::rad::DeclVal<NoThrowThrowResult&&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<NoThrowThrowResult>().operator=(::rad::DeclVal<NoThrowThrowResult&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResult>().Assign(::rad::DeclVal<const radtest::NonThrowingObject&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResult>().operator=(::rad::DeclVal<const radtest::NonThrowingObject&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResult>().Assign(::rad::DeclVal<radtest::NonThrowingObject&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResult>().operator=(::rad::DeclVal<radtest::NonThrowingObject&&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<NoThrowThrowResult>().Assign(::rad::DeclVal<const radtest::ThrowingObject&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<NoThrowThrowResult>().operator=(::rad::DeclVal<const radtest::ThrowingObject&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<NoThrowThrowResult>().Assign(::rad::DeclVal<radtest::ThrowingObject&&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<NoThrowThrowResult>().operator=(::rad::DeclVal<radtest::ThrowingObject&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResult>().Assign(::rad::DeclVal<const rad::ResultOk<radtest::NonThrowingObject>&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResult>().operator=(::rad::DeclVal<const rad::ResultOk<radtest::NonThrowingObject>&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResult>().Assign(::rad::DeclVal<rad::ResultOk<radtest::NonThrowingObject>&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowThrowResult>().operator=(::rad::DeclVal<rad::ResultOk<radtest::NonThrowingObject>&&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<NoThrowThrowResult>().Assign(::rad::DeclVal<const rad::ResultErr<radtest::ThrowingObject>&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<NoThrowThrowResult>().operator=(::rad::DeclVal<const rad::ResultErr<radtest::ThrowingObject>&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<NoThrowThrowResult>().Assign(::rad::DeclVal<rad::ResultErr<radtest::ThrowingObject>&&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<NoThrowThrowResult>().operator=(::rad::DeclVal<rad::ResultErr<radtest::ThrowingObject>&&>())));

// clang-format on

//
// Result<[throwing], [non-throwing]>
//

using ThrowNoThrowResult =
    rad::Result<radtest::ThrowingObject, radtest::NonThrowingObject>;

// clang-format off

RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResult>().~Result()));
RAD_S_ASSERT(noexcept(ThrowNoThrowResult()));
RAD_S_ASSERT(noexcept(ThrowNoThrowResult(rad::ResultEmptyTag)));
RAD_S_ASSERT(!noexcept(ThrowNoThrowResult(::rad::DeclVal<const radtest::ThrowingObject&>())));
RAD_S_ASSERT(!noexcept(ThrowNoThrowResult(::rad::DeclVal<radtest::ThrowingObject&>())));
RAD_S_ASSERT(!noexcept(ThrowNoThrowResult(::rad::DeclVal<radtest::ThrowingObject&&>())));
RAD_S_ASSERT(noexcept(ThrowNoThrowResult(::rad::DeclVal<const radtest::NonThrowingObject&>())));
RAD_S_ASSERT(noexcept(ThrowNoThrowResult(::rad::DeclVal<radtest::NonThrowingObject&>())));
RAD_S_ASSERT(noexcept(ThrowNoThrowResult(::rad::DeclVal<radtest::NonThrowingObject&&>())));
RAD_S_ASSERT(!noexcept(ThrowNoThrowResult(::rad::DeclVal<const rad::ResultOk<radtest::ThrowingObject>&>())));
RAD_S_ASSERT(!noexcept(ThrowNoThrowResult(::rad::DeclVal<rad::ResultOk<radtest::ThrowingObject>&>())));
RAD_S_ASSERT(!noexcept(ThrowNoThrowResult(::rad::DeclVal<rad::ResultOk<radtest::ThrowingObject>&&>())));
RAD_S_ASSERT(noexcept(ThrowNoThrowResult(::rad::DeclVal<const rad::ResultErr<radtest::NonThrowingObject>&>())));
RAD_S_ASSERT(noexcept(ThrowNoThrowResult(::rad::DeclVal<rad::ResultErr<radtest::NonThrowingObject>&>())));
RAD_S_ASSERT(noexcept(ThrowNoThrowResult(::rad::DeclVal<rad::ResultErr<radtest::NonThrowingObject>&&>())));
RAD_S_ASSERT(!noexcept(ThrowNoThrowResult(rad::ResultOkTag, 1)));
RAD_S_ASSERT(noexcept(ThrowNoThrowResult(rad::ResultErrTag, 1)));
#if RAD_ENABLE_STD
RAD_S_ASSERT(!noexcept(ThrowNoThrowResult(rad::ResultOkTag, { 1 })));
RAD_S_ASSERT(noexcept(ThrowNoThrowResult(rad::ResultErrTag, { 1 })));
#endif
RAD_S_ASSERT(!noexcept(ThrowNoThrowResult(::rad::DeclVal<const ThrowNoThrowResult&>())));
RAD_S_ASSERT(!noexcept(ThrowNoThrowResult(::rad::DeclVal<ThrowNoThrowResult&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResult>().operator bool()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResult>().IsOk()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResult>().IsErr()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResult>().State()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResult>().Ok()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResult>().Err()));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowNoThrowResult>().Or(1)));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowNoThrowResult>().OnErr<int>(1)));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResult>().OnOk<int>(1)));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowNoThrowResult>().OnErr(::rad::DeclVal<std::string&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowNoThrowResult>().OnErr(::rad::DeclVal<const std::string&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResult>().OnOk(::rad::DeclVal<std::string&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResult>().OnOk(::rad::DeclVal<const std::string&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResult>().operator->()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResult>().operator*()));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowNoThrowResult>().Assign(::rad::DeclVal<const ThrowNoThrowResult&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowNoThrowResult>().operator=(::rad::DeclVal<const ThrowNoThrowResult&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowNoThrowResult>().Assign(::rad::DeclVal<ThrowNoThrowResult&&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowNoThrowResult>().operator=(::rad::DeclVal<ThrowNoThrowResult&&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowNoThrowResult>().Assign(::rad::DeclVal<const radtest::ThrowingObject&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowNoThrowResult>().operator=(::rad::DeclVal<const radtest::ThrowingObject&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowNoThrowResult>().Assign(::rad::DeclVal<radtest::ThrowingObject&&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowNoThrowResult>().operator=(::rad::DeclVal<radtest::ThrowingObject&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResult>().Assign(::rad::DeclVal<const radtest::NonThrowingObject&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResult>().operator=(::rad::DeclVal<const radtest::NonThrowingObject&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResult>().Assign(::rad::DeclVal<radtest::NonThrowingObject&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResult>().operator=(::rad::DeclVal<radtest::NonThrowingObject&&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowNoThrowResult>().Assign(::rad::DeclVal<const rad::ResultOk<radtest::ThrowingObject>&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowNoThrowResult>().operator=(::rad::DeclVal<const rad::ResultOk<radtest::ThrowingObject>&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowNoThrowResult>().Assign(::rad::DeclVal<rad::ResultOk<radtest::ThrowingObject>&&>())));
RAD_S_ASSERT(!noexcept(::rad::DeclVal<ThrowNoThrowResult>().operator=(::rad::DeclVal<rad::ResultOk<radtest::ThrowingObject>&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResult>().Assign(::rad::DeclVal<const rad::ResultErr<radtest::NonThrowingObject>&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResult>().operator=(::rad::DeclVal<const rad::ResultErr<radtest::NonThrowingObject>&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResult>().Assign(::rad::DeclVal<rad::ResultErr<radtest::NonThrowingObject>&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<ThrowNoThrowResult>().operator=(::rad::DeclVal<rad::ResultErr<radtest::NonThrowingObject>&&>())));

//
// Result<[non-throwing], [non-throwing]>
//

using NoThrowNoThrowResult =
    rad::Result<radtest::NonThrowingObject, radtest::NonThrowingObjectTwo>;

// clang-format off

RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().~Result()));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResult()));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResult(rad::ResultEmptyTag)));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResult(::rad::DeclVal<const radtest::NonThrowingObject&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResult(::rad::DeclVal<radtest::NonThrowingObject&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResult(::rad::DeclVal<radtest::NonThrowingObject&&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResult(::rad::DeclVal<const radtest::NonThrowingObjectTwo&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResult(::rad::DeclVal<radtest::NonThrowingObjectTwo&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResult(::rad::DeclVal<radtest::NonThrowingObjectTwo&&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResult(::rad::DeclVal<const rad::ResultOk<radtest::NonThrowingObject>&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResult(::rad::DeclVal<rad::ResultOk<radtest::NonThrowingObject>&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResult(::rad::DeclVal<rad::ResultOk<radtest::NonThrowingObject>&&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResult(::rad::DeclVal<const rad::ResultErr<radtest::NonThrowingObjectTwo>&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResult(::rad::DeclVal<rad::ResultErr<radtest::NonThrowingObjectTwo>&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResult(::rad::DeclVal<rad::ResultErr<radtest::NonThrowingObjectTwo>&&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResult(rad::ResultOkTag, 1)));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResult(rad::ResultErrTag, 1)));
#if RAD_ENABLE_STD
RAD_S_ASSERT(noexcept(NoThrowNoThrowResult(rad::ResultOkTag, { 1 })));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResult(rad::ResultErrTag, { 1 })));
#endif
RAD_S_ASSERT(noexcept(NoThrowNoThrowResult(::rad::DeclVal<const NoThrowNoThrowResult&>())));
RAD_S_ASSERT(noexcept(NoThrowNoThrowResult(::rad::DeclVal<NoThrowNoThrowResult&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().operator bool()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().IsOk()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().IsErr()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().State()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().Ok()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().Err()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().Or(1)));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().OnErr<int>(1)));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().OnOk<int>(1)));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().OnErr(::rad::DeclVal<std::string&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().OnErr(::rad::DeclVal<const std::string&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().OnOk(::rad::DeclVal<std::string&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().OnOk(::rad::DeclVal<const std::string&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().operator->()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().operator*()));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().Assign(::rad::DeclVal<const NoThrowNoThrowResult&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().operator=(::rad::DeclVal<const NoThrowNoThrowResult&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().Assign(::rad::DeclVal<NoThrowNoThrowResult&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().operator=(::rad::DeclVal<NoThrowNoThrowResult&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().Assign(::rad::DeclVal<const radtest::NonThrowingObject&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().operator=(::rad::DeclVal<const radtest::NonThrowingObject&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().Assign(::rad::DeclVal<radtest::NonThrowingObject&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().operator=(::rad::DeclVal<radtest::NonThrowingObject&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().Assign(::rad::DeclVal<const radtest::NonThrowingObjectTwo&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().operator=(::rad::DeclVal<const radtest::NonThrowingObjectTwo&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().Assign(::rad::DeclVal<radtest::NonThrowingObjectTwo&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().operator=(::rad::DeclVal<radtest::NonThrowingObjectTwo&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().Assign(::rad::DeclVal<const rad::ResultOk<radtest::NonThrowingObject>&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().operator=(::rad::DeclVal<const rad::ResultOk<radtest::NonThrowingObject>&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().Assign(::rad::DeclVal<rad::ResultOk<radtest::NonThrowingObject>&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().operator=(::rad::DeclVal<rad::ResultOk<radtest::NonThrowingObject>&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().Assign(::rad::DeclVal<const rad::ResultErr<radtest::NonThrowingObjectTwo>&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().operator=(::rad::DeclVal<const rad::ResultErr<radtest::NonThrowingObjectTwo>&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().Assign(::rad::DeclVal<rad::ResultErr<radtest::NonThrowingObjectTwo>&&>())));
RAD_S_ASSERT(noexcept(::rad::DeclVal<NoThrowNoThrowResult>().operator=(::rad::DeclVal<rad::ResultErr<radtest::NonThrowingObjectTwo>&&>())));

} // namespace testnoexcept

typedef long MYSTATUS;
#define MYSTATUS_UNSUCCESSFUL    ((MYSTATUS)0xC0000001L)
#define MYSTATUS_INVALID_ADDRESS ((MYSTATUS)0xC0000141L)
template <typename T>
using ResultStatus = rad::Result<T, MYSTATUS>;

struct EmptyOk
{
};

class TestObject
{
public:

    ResultStatus<TestObject&> DoIt(bool ActuallyDoIt)
    {
        if (ActuallyDoIt)
        {
            m_didIt = true;
            return *this;
        }

        m_didIt = false;
        return MYSTATUS_UNSUCCESSFUL;
    }

    ResultStatus<EmptyOk> DoEmpty(bool Error)
    {
        if (Error)
        {
            return MYSTATUS_UNSUCCESSFUL;
        }

        return EmptyOk();
    }

    ResultStatus<TestObject&> NestedEmpty(bool Error, bool ActuallyDoIt)
    {
        auto res = DoEmpty(Error);
        if (!res.IsOk())
        {
            return res.Err();
        }

        return DoIt(ActuallyDoIt);
    }

    ResultStatus<TestObject&> OnErr(bool ActuallyDoIt)
    {
        return DoIt(ActuallyDoIt).OnErr<MYSTATUS>(MYSTATUS_INVALID_ADDRESS);
    }

    ResultStatus<TestObject&> OnOk(bool Error)
    {
        return DoEmpty(Error).OnOk(*this);
    }

    bool m_didIt = false;
};

RAD_S_ASSERT(std::is_trivially_destructible<ResultStatus<EmptyOk>>::value);
RAD_S_ASSERT(!std::is_trivially_destructible<ResultStatus<std::string>>::value);
RAD_S_ASSERT(std::is_trivially_destructible<ResultStatus<std::string&>>::value);
RAD_S_ASSERT((!std::is_trivially_destructible<
              rad::Result<std::string, std::string>>::value));
RAD_S_ASSERT((
    !std::is_trivially_destructible<rad::Result<EmptyOk, std::string>>::value));

#if RAD_CPP17
RAD_S_ASSERT(
    std::is_trivially_destructible<ResultStatus<std::string_view>>::value);
RAD_S_ASSERT((std::is_trivially_destructible<
              rad::Result<std::string_view, std::string_view>>::value));
RAD_S_ASSERT((!std::is_trivially_destructible<
              rad::Result<std::string, std::string_view>>::value));
RAD_S_ASSERT((!std::is_trivially_destructible<
              rad::Result<std::string_view, std::string>>::value));
RAD_S_ASSERT((std::is_trivially_destructible<
              rad::Result<EmptyOk, std::string_view>>::value));
#endif

static constexpr ResultStatus<int> k_ResultNum = 0xc001;
static constexpr ResultStatus<int> k_ResultNumErr = MYSTATUS_UNSUCCESSFUL;
static constexpr ResultStatus<EmptyOk> k_Error = MYSTATUS_UNSUCCESSFUL;

#if RAD_CPP17
static constexpr ResultStatus<std::string_view> k_ResultStr{ "Yeet!" };
static constexpr ResultStatus<std::string_view> k_ResultStrErr =
    MYSTATUS_UNSUCCESSFUL;
static constexpr rad::Result<std::string_view, std::string_view>
    k_ResultStrStr = rad::ResultOk<std::string_view>("Ok!");
static constexpr rad::Result<std::string_view, std::string_view>
    k_ResultStrStrErr = rad::ResultErr<std::string_view>("Err!");
static constexpr rad::Result<EmptyOk, std::string_view> k_ErrStr{ "Err!" };
#endif

TEST(ResultTests, TestObjectDidIt)
{
    TestObject object;

    auto res = object.DoIt(true);
    EXPECT_TRUE(res.IsOk());
    EXPECT_EQ(&res.Ok(), &object);
    EXPECT_TRUE(object.m_didIt);

    res = res->DoIt(false);
    EXPECT_TRUE(res.IsErr());
    EXPECT_EQ(res.Err(), MYSTATUS_UNSUCCESSFUL);
    EXPECT_FALSE(object.m_didIt);

    object.m_didIt = false;

    EXPECT_TRUE(object.DoEmpty(true).IsErr());
    EXPECT_TRUE(object.DoEmpty(false).IsOk());
    EXPECT_EQ(object.DoEmpty(true).Err(), MYSTATUS_UNSUCCESSFUL);

    res = object.NestedEmpty(true, false);
    EXPECT_FALSE(object.m_didIt);
    EXPECT_TRUE(res.IsErr());

    res = object.NestedEmpty(true, true);
    EXPECT_FALSE(object.m_didIt);
    EXPECT_TRUE(res.IsErr());

    res = object.NestedEmpty(false, false);
    EXPECT_FALSE(object.m_didIt);
    EXPECT_TRUE(res.IsErr());

    res = object.NestedEmpty(false, true);
    EXPECT_TRUE(object.m_didIt);
    EXPECT_TRUE(res.IsOk());

    object.m_didIt = false;

    res = object.OnErr(false);
    EXPECT_FALSE(object.m_didIt);
    EXPECT_TRUE(res.IsErr());
    EXPECT_EQ(res.Err(), MYSTATUS_INVALID_ADDRESS);

    res = object.OnErr(true);
    EXPECT_TRUE(object.m_didIt);
    EXPECT_TRUE(res.IsOk());
    EXPECT_EQ(&res.Ok(), &object);

    res = object.OnOk(true);
    EXPECT_TRUE(res.IsErr());
    EXPECT_EQ(res.Err(), MYSTATUS_UNSUCCESSFUL);

    res = object.OnOk(false);
    EXPECT_TRUE(res.IsOk());
    EXPECT_EQ(&res.Ok(), &object);
}

TEST(ResultTests, ConstexprErrStatus)
{
    EXPECT_EQ(k_Error, MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, ConstexprOkNumeric)
{
    EXPECT_FALSE(k_ResultNum.IsEmpty());
    EXPECT_FALSE(k_ResultNum.IsErr());
    EXPECT_TRUE(k_ResultNum.IsOk());
    EXPECT_EQ(k_ResultNum, 0xc001);
    auto resNum = k_ResultNum;
    EXPECT_EQ(resNum, k_ResultNum);
}

TEST(ResultTests, ConstexprErrNumeric)
{
    EXPECT_FALSE(k_ResultNumErr.IsEmpty());
    EXPECT_TRUE(k_ResultNumErr.IsErr());
    EXPECT_FALSE(k_ResultNumErr.IsOk());
    EXPECT_EQ(k_ResultNumErr, MYSTATUS_UNSUCCESSFUL);
    auto resNum = k_ResultNumErr;
    EXPECT_EQ(resNum, k_ResultNumErr);
    EXPECT_NE(k_ResultNum, k_ResultNumErr);
}

#if RAD_CPP17

TEST(ResultTests, ConstexprErrString)
{
    EXPECT_EQ(k_ErrStr, std::string_view("Err!"));
}

TEST(ResultTests, ConstexprOkString)
{
    EXPECT_FALSE(k_ResultStr.IsEmpty());
    EXPECT_FALSE(k_ResultStr.IsErr());
    EXPECT_TRUE(k_ResultStr.IsOk());
    EXPECT_EQ(k_ResultStr, std::string_view("Yeet!"));
    auto resStr = k_ResultStr;
    EXPECT_EQ(resStr, k_ResultStr);
    ResultStatus<std::string> strAllocStr = resStr;
    EXPECT_EQ(strAllocStr, resStr);
}

TEST(ResultTests, ConstexprOkStringErrStatus)
{
    EXPECT_FALSE(k_ResultStrErr.IsEmpty());
    EXPECT_TRUE(k_ResultStrErr.IsErr());
    EXPECT_FALSE(k_ResultStrErr.IsOk());
    EXPECT_EQ(k_ResultStrErr, MYSTATUS_UNSUCCESSFUL);
    auto resStr = k_ResultStrErr;
    EXPECT_EQ(resStr, k_ResultStrErr);
    EXPECT_NE(k_ResultStr, k_ResultStrErr);
}

TEST(ResultTests, ConstexprOkStringErr)
{
    EXPECT_FALSE(k_ResultStrStr.IsEmpty());
    EXPECT_FALSE(k_ResultStrStr.IsErr());
    EXPECT_TRUE(k_ResultStrStr.IsOk());
    EXPECT_EQ(k_ResultStrStr, rad::ResultOk<std::string_view>("Ok!"));
    auto resStrStr = k_ResultStrStr;
    EXPECT_EQ(resStrStr, k_ResultStrStr);
    ResultStatus<std::string> strAllocStrStr =
        rad::ResultOk<std::string>(resStrStr.Ok());
    EXPECT_EQ(strAllocStrStr.Ok(), resStrStr.Ok());
}

TEST(ResultTests, ConstexprOkStringErrString)
{
    EXPECT_FALSE(k_ResultStrStrErr.IsEmpty());
    EXPECT_TRUE(k_ResultStrStrErr.IsErr());
    EXPECT_FALSE(k_ResultStrStrErr.IsOk());
    EXPECT_EQ(k_ResultStrStrErr, rad::ResultErr<std::string_view>("Err!"));
    auto resStrStr = k_ResultStrStrErr;
    EXPECT_EQ(resStrStr, k_ResultStrStrErr);
    EXPECT_NE(k_ResultStrStr, k_ResultStrStrErr);
    ResultStatus<std::string> strAllocStrStrErr =
        rad::ResultOk<std::string>(resStrStr.Err());
    EXPECT_EQ(strAllocStrStrErr.Ok(), resStrStr.Err());
}

#endif

static const ResultStatus<int> g_ResultEmpty{};
static const ResultStatus<int> g_ResultNum{ 0xc001 };
static const ResultStatus<int> g_ResultNumErr{ MYSTATUS_UNSUCCESSFUL };
static const ResultStatus<std::string> g_ResultStr{ "Yeet!" };
static const ResultStatus<std::string> g_ResultStrErr{ MYSTATUS_UNSUCCESSFUL };
static const rad::Result<std::string, std::string> g_ResultStrStr{
    rad::ResultOk<std::string>("Ok!")
};
static const rad::Result<std::string, std::string> g_ResultStrStrErr{
    rad::ResultErr<std::string>("Err!")
};

TEST(ResultTests, GlobalEmpty)
{
    EXPECT_TRUE(g_ResultEmpty.IsEmpty());
    EXPECT_FALSE(g_ResultEmpty.IsErr());
    EXPECT_FALSE(g_ResultEmpty.IsOk());
    auto resEmpty = g_ResultEmpty;
    EXPECT_EQ(resEmpty, g_ResultEmpty);
}

TEST(ResultTests, GlobalOkNumeric)
{
    EXPECT_FALSE(g_ResultNum.IsEmpty());
    EXPECT_FALSE(g_ResultNum.IsErr());
    EXPECT_TRUE(g_ResultNum.IsOk());
    EXPECT_EQ(g_ResultNum, 0xc001);
    auto resNum = g_ResultNum;
    EXPECT_EQ(resNum, g_ResultNum);
    EXPECT_NE(resNum, g_ResultEmpty);
    resNum = g_ResultEmpty;
    EXPECT_TRUE(resNum.IsEmpty());
}

TEST(ResultTests, GlobalErrNumeric)
{
    EXPECT_FALSE(g_ResultNumErr.IsEmpty());
    EXPECT_TRUE(g_ResultNumErr.IsErr());
    EXPECT_FALSE(g_ResultNumErr.IsOk());
    EXPECT_EQ(g_ResultNumErr, MYSTATUS_UNSUCCESSFUL);
    auto resNum = g_ResultNumErr;
    EXPECT_EQ(resNum, g_ResultNumErr);
    EXPECT_NE(resNum, g_ResultEmpty);
    EXPECT_NE(g_ResultNum, g_ResultNumErr);
}

TEST(ResultTests, GlobalOkString)
{
    EXPECT_FALSE(g_ResultStr.IsEmpty());
    EXPECT_FALSE(g_ResultStr.IsErr());
    EXPECT_TRUE(g_ResultStr.IsOk());
    EXPECT_EQ(g_ResultStr, std::string("Yeet!"));
    auto resStr = g_ResultStr;
    EXPECT_EQ(resStr, g_ResultStr);
    ResultStatus<std::string> strAllocStr = resStr;
    EXPECT_EQ(strAllocStr, resStr);
    ResultStatus<std::string> empty;
    EXPECT_TRUE(empty.IsEmpty());
    ResultStatus<std::string> second = empty;
    EXPECT_TRUE(second.IsEmpty());
}

TEST(ResultTests, GlobalErrString)
{
    EXPECT_FALSE(g_ResultStrErr.IsEmpty());
    EXPECT_TRUE(g_ResultStrErr.IsErr());
    EXPECT_FALSE(g_ResultStrErr.IsOk());
    EXPECT_EQ(g_ResultStrErr, MYSTATUS_UNSUCCESSFUL);
    auto resStr = g_ResultStrErr;
    EXPECT_EQ(resStr, g_ResultStrErr);
    EXPECT_NE(g_ResultStr, g_ResultStrErr);
}

TEST(ResultTests, GlobalErrStringOkString)
{
    EXPECT_FALSE(g_ResultStrStr.IsEmpty());
    EXPECT_FALSE(g_ResultStrStr.IsErr());
    EXPECT_TRUE(g_ResultStrStr.IsOk());
    EXPECT_EQ(g_ResultStrStr, rad::ResultOk<std::string>("Ok!"));
    auto resStrStr = g_ResultStrStr;
    EXPECT_EQ(resStrStr, g_ResultStrStr);
    ResultStatus<std::string> strAllocStrStr =
        rad::ResultOk<std::string>(resStrStr.Ok());
    EXPECT_EQ(strAllocStrStr.Ok(), resStrStr.Ok());
}

TEST(ResultTests, GlobalErrStringErrString)
{
    EXPECT_FALSE(g_ResultStrStrErr.IsEmpty());
    EXPECT_TRUE(g_ResultStrStrErr.IsErr());
    EXPECT_FALSE(g_ResultStrStrErr.IsOk());
    EXPECT_EQ(g_ResultStrStrErr, rad::ResultErr<std::string>("Err!"));
    auto resStrStr = g_ResultStrStrErr;
    ResultStatus<std::string> strAllocStrStr =
        rad::ResultOk<std::string>(resStrStr.Err());
    EXPECT_EQ(resStrStr, g_ResultStrStrErr);
    EXPECT_NE(g_ResultStrStr, g_ResultStrStrErr);
    ResultStatus<std::string> strAllocStrStrNonErr =
        rad::ResultOk<std::string>(resStrStr.Err());
    EXPECT_EQ(strAllocStrStrNonErr.Ok(), resStrStr.Err());
    EXPECT_EQ(strAllocStrStr, strAllocStrStrNonErr);
}

TEST(ResultTests, AssignCopyEmptyOk)
{
    ResultStatus<int> value(rad::ResultEmptyTag);
    ResultStatus<int> other = 1;

    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(value.Assign(other).IsOk());
    EXPECT_EQ(value.Ok(), 1);
    EXPECT_EQ(other.Ok(), 1);
}

TEST(ResultTests, AssignCopyEmptyErr)
{
    ResultStatus<int> value;
    ResultStatus<int> other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(value.Assign(other).IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
    EXPECT_EQ(other.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignCopyEmptyEmpty)
{
    ResultStatus<int> value;
    ResultStatus<int> other;

    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(value.Assign(other).IsEmpty());
    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignCopyOkOk)
{
    ResultStatus<int> value = 1337;
    ResultStatus<int> other = 1;

    EXPECT_TRUE(value.IsOk());
    EXPECT_TRUE(value.Assign(other).IsOk());
    EXPECT_EQ(value.Ok(), 1);
    EXPECT_EQ(other.Ok(), 1);
}

TEST(ResultTests, AssignCopyOkErr)
{
    ResultStatus<int> value = 1337;
    ResultStatus<int> other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsOk());
    EXPECT_TRUE(value.Assign(other).IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
    EXPECT_EQ(other.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignCopyOkEmpty)
{
    ResultStatus<int> value = 1337;
    ResultStatus<int> other;

    EXPECT_TRUE(value.IsOk());
    EXPECT_TRUE(value.Assign(other).IsEmpty());
    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignCopyErrOk)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    ResultStatus<int> other = 1;

    EXPECT_TRUE(value.IsErr());
    EXPECT_TRUE(value.Assign(other).IsOk());
    EXPECT_EQ(value.Ok(), 1);
    EXPECT_EQ(other.Ok(), 1);
}

TEST(ResultTests, AssignCopyErrErr)
{
    ResultStatus<int> value = MYSTATUS_INVALID_ADDRESS;
    ResultStatus<int> other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsErr());
    EXPECT_TRUE(value.Assign(other).IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
    EXPECT_EQ(other.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignCopyErrEmpty)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    ResultStatus<int> other;

    EXPECT_TRUE(value.IsErr());
    EXPECT_TRUE(value.Assign(other).IsEmpty());
    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignCopyOpEmptyOk)
{
    ResultStatus<int> value;
    ResultStatus<int> other = 1;

    EXPECT_TRUE(value.IsEmpty());
    value = other;
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
    EXPECT_EQ(other.Ok(), 1);
}

TEST(ResultTests, AssignCopyOpEmptyErr)
{
    ResultStatus<int> value;
    ResultStatus<int> other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsEmpty());
    value = other;
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
    EXPECT_EQ(other.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignCopyOpEmptyEmpty)
{
    ResultStatus<int> value;
    ResultStatus<int> other;

    EXPECT_TRUE(value.IsEmpty());
    value = other;
    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignCopyOpOkOk)
{
    ResultStatus<int> value = 1337;
    ResultStatus<int> other = 1;

    EXPECT_TRUE(value.IsOk());
    value = other;
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
    EXPECT_EQ(other.Ok(), 1);
}

TEST(ResultTests, AssignCopyOpOkErr)
{
    ResultStatus<int> value = 1337;
    ResultStatus<int> other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsOk());
    value = other;
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
    EXPECT_EQ(other.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignCopyOpOkEmpty)
{
    ResultStatus<int> value = 1337;
    ResultStatus<int> other;

    EXPECT_TRUE(value.IsOk());
    value = other;
    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignCopyOpErrOk)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    ResultStatus<int> other = 1;

    EXPECT_TRUE(value.IsErr());
    value = other;
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
    EXPECT_EQ(other.Ok(), 1);
}

TEST(ResultTests, AssignCopyOpErrErr)
{
    ResultStatus<int> value = MYSTATUS_INVALID_ADDRESS;
    ResultStatus<int> other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsErr());
    value = other;
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
    EXPECT_EQ(other.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignCopyOpErrEmpty)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    ResultStatus<int> other;

    EXPECT_TRUE(value.IsErr());
    value = other;
    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignConstCopyEmptyOk)
{
    ResultStatus<int> value;
    const ResultStatus<int> other = 1;

    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(value.Assign(other).IsOk());
    EXPECT_EQ(value.Ok(), 1);
    EXPECT_EQ(other.Ok(), 1);
}

TEST(ResultTests, AssignConstCopyEmptyErr)
{
    ResultStatus<int> value;
    const ResultStatus<int> other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(value.Assign(other).IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
    EXPECT_EQ(other.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignConstCopyEmptyEmpty)
{
    ResultStatus<int> value;
    const ResultStatus<int> other;

    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(value.Assign(other).IsEmpty());
    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignConstCopyOkOk)
{
    ResultStatus<int> value = 1337;
    const ResultStatus<int> other = 1;

    EXPECT_TRUE(value.IsOk());
    EXPECT_TRUE(value.Assign(other).IsOk());
    EXPECT_EQ(value.Ok(), 1);
    EXPECT_EQ(other.Ok(), 1);
}

TEST(ResultTests, AssignConstCopyOkErr)
{
    ResultStatus<int> value = 1337;
    const ResultStatus<int> other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsOk());
    EXPECT_TRUE(value.Assign(other).IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
    EXPECT_EQ(other.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignConstCopyOkEmpty)
{
    ResultStatus<int> value = 1337;
    const ResultStatus<int> other;

    EXPECT_TRUE(value.IsOk());
    EXPECT_TRUE(value.Assign(other).IsEmpty());
    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignConstCopyErrOk)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    const ResultStatus<int> other = 1;

    EXPECT_TRUE(value.IsErr());
    EXPECT_TRUE(value.Assign(other).IsOk());
    EXPECT_EQ(value.Ok(), 1);
    EXPECT_EQ(other.Ok(), 1);
}

TEST(ResultTests, AssignConstCopyErrErr)
{
    ResultStatus<int> value = MYSTATUS_INVALID_ADDRESS;
    const ResultStatus<int> other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsErr());
    EXPECT_TRUE(value.Assign(other).IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
    EXPECT_EQ(other.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignConstCopyErrEmpty)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    const ResultStatus<int> other;

    EXPECT_TRUE(value.IsErr());
    EXPECT_TRUE(value.Assign(other).IsEmpty());
    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignConstCopyOpEmptyOk)
{
    ResultStatus<int> value;
    const ResultStatus<int> other = 1;

    EXPECT_TRUE(value.IsEmpty());
    value = other;
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
    EXPECT_EQ(other.Ok(), 1);
}

TEST(ResultTests, AssignConstCopyOpEmptyErr)
{
    ResultStatus<int> value;
    const ResultStatus<int> other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsEmpty());
    value = other;
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
    EXPECT_EQ(other.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignConstCopyOpEmptyEmpty)
{
    ResultStatus<int> value;
    const ResultStatus<int> other;

    EXPECT_TRUE(value.IsEmpty());
    value = other;
    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignConstCopyOpOkOk)
{
    ResultStatus<int> value = 1337;
    const ResultStatus<int> other = 1;

    EXPECT_TRUE(value.IsOk());
    value = other;
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
    EXPECT_EQ(other.Ok(), 1);
}

TEST(ResultTests, AssignConstCopyOpOkErr)
{
    ResultStatus<int> value = 1337;
    const ResultStatus<int> other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsOk());
    value = other;
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
    EXPECT_EQ(other.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignConstCopyOpOkEmpty)
{
    ResultStatus<int> value = 1337;
    const ResultStatus<int> other;

    EXPECT_TRUE(value.IsOk());
    value = other;
    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignConstCopyOpErrOk)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    const ResultStatus<int> other = 1;

    EXPECT_TRUE(value.IsErr());
    value = other;
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
    EXPECT_EQ(other.Ok(), 1);
}

TEST(ResultTests, AssignConstCopyOpErrErr)
{
    ResultStatus<int> value = MYSTATUS_INVALID_ADDRESS;
    const ResultStatus<int> other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsErr());
    value = other;
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
    EXPECT_EQ(other.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignConstCopyOpErrEmpty)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    const ResultStatus<int> other;

    EXPECT_TRUE(value.IsErr());
    value = other;
    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignMoveEmptyOk)
{
    ResultStatus<int> value;
    ResultStatus<int> other = 1;

    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(value.Assign(rad::Move(other)).IsOk());
    EXPECT_EQ(value.Ok(), 1);
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignMoveEmptyErr)
{
    ResultStatus<int> value;
    ResultStatus<int> other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(value.Assign(rad::Move(other)).IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignMoveEmptyEmpty)
{
    ResultStatus<int> value;
    ResultStatus<int> other;

    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(value.Assign(rad::Move(other)).IsEmpty());
    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignMoveOkOk)
{
    ResultStatus<int> value = 1337;
    ResultStatus<int> other = 1;

    EXPECT_TRUE(value.IsOk());
    EXPECT_TRUE(value.Assign(rad::Move(other)).IsOk());
    EXPECT_EQ(value.Ok(), 1);
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignMoveOkErr)
{
    ResultStatus<int> value = 1337;
    ResultStatus<int> other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsOk());
    EXPECT_TRUE(value.Assign(rad::Move(other)).IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignMoveOkEmpty)
{
    ResultStatus<int> value = 1337;
    ResultStatus<int> other;

    EXPECT_TRUE(value.IsOk());
    EXPECT_TRUE(value.Assign(rad::Move(other)).IsEmpty());
    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignMoveErrOk)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    ResultStatus<int> other = 1;

    EXPECT_TRUE(value.IsErr());
    EXPECT_TRUE(value.Assign(rad::Move(other)).IsOk());
    EXPECT_EQ(value.Ok(), 1);
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignMoveErrErr)
{
    ResultStatus<int> value = MYSTATUS_INVALID_ADDRESS;
    ResultStatus<int> other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsErr());
    EXPECT_TRUE(value.Assign(rad::Move(other)).IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignMoveErrEmpty)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    ResultStatus<int> other;

    EXPECT_TRUE(value.IsErr());
    EXPECT_TRUE(value.Assign(rad::Move(other)).IsEmpty());
    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignMoveOpEmptyOk)
{
    ResultStatus<int> value;
    ResultStatus<int> other = 1;

    EXPECT_TRUE(value.IsEmpty());
    value = rad::Move(other);
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignMoveOpEmptyErr)
{
    ResultStatus<int> value;
    ResultStatus<int> other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsEmpty());
    value = rad::Move(other);
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignMoveOpEmptyEmpty)
{
    ResultStatus<int> value;
    ResultStatus<int> other;

    EXPECT_TRUE(value.IsEmpty());
    value = rad::Move(other);
    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignMoveOpOkOk)
{
    ResultStatus<int> value = 1337;
    ResultStatus<int> other = 1;

    EXPECT_TRUE(value.IsOk());
    value = rad::Move(other);
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignMoveOpOkErr)
{
    ResultStatus<int> value = 1337;
    ResultStatus<int> other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsOk());
    value = rad::Move(other);
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignMoveOpOkEmpty)
{
    ResultStatus<int> value = 1337;
    ResultStatus<int> other;

    EXPECT_TRUE(value.IsOk());
    value = rad::Move(other);
    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignMoveOpErrOk)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    ResultStatus<int> other = 1;

    EXPECT_TRUE(value.IsErr());
    value = rad::Move(other);
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignMoveOpErrErr)
{
    ResultStatus<int> value = MYSTATUS_INVALID_ADDRESS;
    ResultStatus<int> other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsErr());
    value = rad::Move(other);
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignMoveOpErrEmpty)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    ResultStatus<int> other;

    EXPECT_TRUE(value.IsErr());
    value = rad::Move(other);
    EXPECT_TRUE(value.IsEmpty());
    EXPECT_TRUE(other.IsEmpty());
}

TEST(ResultTests, AssignCopyResOkErr)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    rad::ResultOk<int> other = 1;

    EXPECT_TRUE(value.IsErr());
    value.Assign(other);
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, AssignCopyResOkOk)
{
    ResultStatus<int> value = 0;
    rad::ResultOk<int> other = 1;

    EXPECT_TRUE(value.IsOk());
    value.Assign(other);
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, AssignCopyOpResOkErr)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    rad::ResultOk<int> other = 1;

    EXPECT_TRUE(value.IsErr());
    value = other;
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, AssignCopyOpResOkOk)
{
    ResultStatus<int> value = 0;
    rad::ResultOk<int> other = 1;

    EXPECT_TRUE(value.IsOk());
    value = other;
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, AssignMoveResOkErr)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    rad::ResultOk<int> other = 1;

    EXPECT_TRUE(value.IsErr());
    value.Assign(rad::Move(other));
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, AssignMoveResOkOk)
{
    ResultStatus<int> value = 0;
    rad::ResultOk<int> other = 1;

    EXPECT_TRUE(value.IsOk());
    value.Assign(rad::Move(other));
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, AssignMoveOpResOkErr)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    rad::ResultOk<int> other = 1;

    EXPECT_TRUE(value.IsErr());
    value = rad::Move(other);
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, AssignMoveOpResOkOk)
{
    ResultStatus<int> value = 0;
    rad::ResultOk<int> other = 1;

    EXPECT_TRUE(value.IsOk());
    value = rad::Move(other);
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, AssignCopyResErrErr)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    rad::ResultErr<MYSTATUS> other = MYSTATUS_INVALID_ADDRESS;

    EXPECT_TRUE(value.IsErr());
    value.Assign(other);
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_INVALID_ADDRESS);
}

TEST(ResultTests, AssignCopyResErrOk)
{
    ResultStatus<int> value = 0;
    rad::ResultErr<MYSTATUS> other = MYSTATUS_INVALID_ADDRESS;

    EXPECT_TRUE(value.IsOk());
    value.Assign(other);
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_INVALID_ADDRESS);
}

TEST(ResultTests, AssignCopyOpResErrErr)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    rad::ResultErr<MYSTATUS> other = MYSTATUS_INVALID_ADDRESS;

    EXPECT_TRUE(value.IsErr());
    value = other;
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_INVALID_ADDRESS);
}

TEST(ResultTests, AssignCopyOpResErrOk)
{
    ResultStatus<int> value = 0;
    rad::ResultErr<MYSTATUS> other = MYSTATUS_INVALID_ADDRESS;

    EXPECT_TRUE(value.IsOk());
    value = other;
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_INVALID_ADDRESS);
}

TEST(ResultTests, AssignMoveResErrErr)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    rad::ResultErr<MYSTATUS> other = MYSTATUS_INVALID_ADDRESS;

    EXPECT_TRUE(value.IsErr());
    value.Assign(rad::Move(other));
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_INVALID_ADDRESS);
}

TEST(ResultTests, AssignMoveResErrOk)
{
    ResultStatus<int> value = 0;
    rad::ResultErr<MYSTATUS> other = MYSTATUS_INVALID_ADDRESS;

    EXPECT_TRUE(value.IsOk());
    value.Assign(rad::Move(other));
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_INVALID_ADDRESS);
}

TEST(ResultTests, AssignMoveOpResErrErr)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    rad::ResultErr<MYSTATUS> other = MYSTATUS_INVALID_ADDRESS;

    EXPECT_TRUE(value.IsErr());
    value = rad::Move(other);
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_INVALID_ADDRESS);
}

TEST(ResultTests, AssignMoveOpResErrOk)
{
    ResultStatus<int> value = 0;
    rad::ResultErr<MYSTATUS> other = MYSTATUS_INVALID_ADDRESS;

    EXPECT_TRUE(value.IsOk());
    value = rad::Move(other);
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_INVALID_ADDRESS);
}

TEST(ResultTests, AssignCopyValueEmpty)
{
    ResultStatus<int> value;

    EXPECT_TRUE(value.IsEmpty());
    value.Assign(1);
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, AssignCopyValueOk)
{
    ResultStatus<int> value = 0;

    EXPECT_TRUE(value.IsOk());
    value.Assign(1);
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, AssignCopyValueErr)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsErr());
    value.Assign(1);
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, AssignCopyConstValueEmpty)
{
    ResultStatus<int> value;
    const int other = 1;

    EXPECT_TRUE(value.IsEmpty());
    value.Assign(other);
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, AssignCopyConstValueOk)
{
    ResultStatus<int> value = 0;
    const int other = 1;

    EXPECT_TRUE(value.IsOk());
    value.Assign(other);
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, AssignCopyConstValueErr)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    const int other = 1;

    EXPECT_TRUE(value.IsErr());
    value.Assign(other);
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, AssignMoveValueEmpty)
{
    ResultStatus<std::string> value;
    std::string other = "yeet";

    EXPECT_TRUE(value.IsEmpty());
    value.Assign(rad::Move(other));
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), "yeet");
    EXPECT_TRUE(other.empty());
}

TEST(ResultTests, AssignMoveValueOk)
{
    ResultStatus<std::string> value("derp");
    std::string other = "yeet";

    EXPECT_TRUE(value.IsOk());
    value.Assign(rad::Move(other));
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), "yeet");
    EXPECT_TRUE(other.empty());
}

TEST(ResultTests, AssignMoveValueErr)
{
    ResultStatus<std::string> value = MYSTATUS_UNSUCCESSFUL;
    std::string other = "yeet";

    EXPECT_TRUE(value.IsErr());
    value.Assign(rad::Move(other));
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), "yeet");
    EXPECT_TRUE(other.empty());
}

TEST(ResultTests, AssignCopyOpValueEmpty)
{
    ResultStatus<int> value;

    EXPECT_TRUE(value.IsEmpty());
    value = 1;
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, AssignCopyOpValueOk)
{
    ResultStatus<int> value = 0;

    EXPECT_TRUE(value.IsOk());
    value = 1;
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, AssignCopyOpValueErr)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsErr());
    value = 1;
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, AssignCopyOpConstValueEmpty)
{
    ResultStatus<int> value;
    const int other = 1;

    EXPECT_TRUE(value.IsEmpty());
    value = other;
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, AssignCopyOpConstValueOk)
{
    ResultStatus<int> value = 0;
    const int other = 1;

    EXPECT_TRUE(value.IsOk());
    value = other;
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, AssignCopyOpConstValueErr)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    const int other = 1;

    EXPECT_TRUE(value.IsErr());
    value = other;
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, AssignMoveOpValueEmpty)
{
    ResultStatus<std::string> value;
    std::string other = "yeet";

    EXPECT_TRUE(value.IsEmpty());
    value = rad::Move(other);
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), "yeet");
    EXPECT_TRUE(other.empty());
}

TEST(ResultTests, AssignMoveOpValueOk)
{
    ResultStatus<std::string> value("derp");
    std::string other = "yeet";

    EXPECT_TRUE(value.IsOk());
    value = rad::Move(other);
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), "yeet");
    EXPECT_TRUE(other.empty());
}

TEST(ResultTests, AssignMoveOpValueErr)
{
    ResultStatus<std::string> value = MYSTATUS_UNSUCCESSFUL;
    std::string other = "yeet";

    EXPECT_TRUE(value.IsErr());
    value = rad::Move(other);
    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), "yeet");
    EXPECT_TRUE(other.empty());
}

TEST(ResultTests, AssignCopyErrValueEmpty)
{
    ResultStatus<int> value;

    EXPECT_TRUE(value.IsEmpty());
    value.Assign(MYSTATUS_UNSUCCESSFUL);
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignCopyErrValueOk)
{
    ResultStatus<int> value = 0;

    EXPECT_TRUE(value.IsOk());
    value.Assign(MYSTATUS_UNSUCCESSFUL);
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignCopyErrValueErr)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsErr());
    value.Assign(MYSTATUS_UNSUCCESSFUL);
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignCopyConsErrValueEmpty)
{
    ResultStatus<int> value;
    const MYSTATUS other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsEmpty());
    value.Assign(other);
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignCopyConstErrValueOk)
{
    ResultStatus<int> value = 0;
    const MYSTATUS other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsOk());
    value.Assign(other);
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignCopyConstErrValueErr)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    const MYSTATUS other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsErr());
    value.Assign(other);
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignMoveErrValueEmpty)
{
    rad::Result<int, std::string> value;
    std::string other = "yeet";

    EXPECT_TRUE(value.IsEmpty());
    value.Assign(rad::Move(other));
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), "yeet");
    EXPECT_TRUE(other.empty());
}

TEST(ResultTests, AssignMoveErrValueOk)
{
    rad::Result<int, std::string> value = 1;
    std::string other = "yeet";

    EXPECT_TRUE(value.IsOk());
    value.Assign(rad::Move(other));
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), "yeet");
    EXPECT_TRUE(other.empty());
}

TEST(ResultTests, AssignMoveErrValueErr)
{
    rad::Result<int, std::string> value("derp");
    std::string other = "yeet";

    EXPECT_TRUE(value.IsErr());
    value.Assign(rad::Move(other));
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), "yeet");
    EXPECT_TRUE(other.empty());
}

TEST(ResultTests, AssignCopyOpErrValueEmpty)
{
    ResultStatus<int> value;

    EXPECT_TRUE(value.IsEmpty());
    value = MYSTATUS_UNSUCCESSFUL;
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignCopyOpErrValueOk)
{
    ResultStatus<int> value = 0;

    EXPECT_TRUE(value.IsOk());
    value = MYSTATUS_UNSUCCESSFUL;
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignCopyOpErrValueErr)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsErr());
    value = MYSTATUS_UNSUCCESSFUL;
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignCopyOpConstErrValueEmpty)
{
    ResultStatus<int> value;
    const MYSTATUS other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsEmpty());
    value = other;
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignCopyOpConstErrValueOk)
{
    ResultStatus<int> value = 0;
    const MYSTATUS other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsOk());
    value = other;
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignCopyOpConstErrValueErr)
{
    ResultStatus<int> value = MYSTATUS_UNSUCCESSFUL;
    const MYSTATUS other = MYSTATUS_UNSUCCESSFUL;

    EXPECT_TRUE(value.IsErr());
    value = other;
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignMoveOpErrValueEmpty)
{
    rad::Result<int, std::string> value;
    std::string other = "yeet";

    EXPECT_TRUE(value.IsEmpty());
    value = rad::Move(other);
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), "yeet");
    EXPECT_TRUE(other.empty());
}

TEST(ResultTests, AssignMoveOpErrValueOk)
{
    rad::Result<int, std::string> value = 1;
    std::string other = "yeet";

    EXPECT_TRUE(value.IsOk());
    value = rad::Move(other);
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), "yeet");
    EXPECT_TRUE(other.empty());
}

TEST(ResultTests, AssignMoveOpErrValueErr)
{
    rad::Result<int, std::string> value("derp");
    std::string other = "yeet";

    EXPECT_TRUE(value.IsErr());
    value = rad::Move(other);
    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), "yeet");
    EXPECT_TRUE(other.empty());
}

TEST(ResultTests, ConstructCopyOk)
{
    int other = 1;
    ResultStatus<int> value(other);

    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, ConstructConstCopyOk)
{
    const int other = 1;
    ResultStatus<int> value(other);

    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, ConstructCopyErr)
{
    MYSTATUS other = MYSTATUS_UNSUCCESSFUL;
    ResultStatus<int> value(other);

    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, ConstructConstCopyErr)
{
    const MYSTATUS other = MYSTATUS_UNSUCCESSFUL;
    ResultStatus<int> value(other);

    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, ConstructCopyResOk)
{
    auto other = rad::ResultOk<int>(1);
    ResultStatus<int> value(other);

    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, ConstructConstCopyResOk)
{
    const auto other = rad::ResultOk<int>(1);
    ResultStatus<int> value(other);

    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok(), 1);
}

TEST(ResultTests, ConstructCopyResErr)
{
    auto other = rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL);
    ResultStatus<int> value(other);

    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, ConstructConstCopyResErr)
{
    const auto other = rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL);
    ResultStatus<int> value(other);

    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, ConstructEmplaceOk)
{
    rad::Result<std::pair<int, int>, std::pair<MYSTATUS, MYSTATUS>> value(
        rad::ResultOkTag, 1, 2);

    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok().first, 1);
    EXPECT_EQ(value.Ok().second, 2);
}

TEST(ResultTests, ConstructEmplaceErr)
{
    rad::Result<std::pair<int, int>, std::pair<MYSTATUS, MYSTATUS>> value(
        rad::ResultErrTag, MYSTATUS_UNSUCCESSFUL, MYSTATUS_INVALID_ADDRESS);

    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err().first, MYSTATUS_UNSUCCESSFUL);
    EXPECT_EQ(value.Err().second, MYSTATUS_INVALID_ADDRESS);
}

#if RAD_ENABLE_STD

TEST(ResultTests, ConstructListOk)
{
    rad::Result<std::vector<int>, std::vector<MYSTATUS>> value(rad::ResultOkTag,
                                                               { 1, 2 });

    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok().front(), 1);
    EXPECT_EQ(value.Ok().back(), 2);
}

TEST(ResultTests, ConstructListErr)
{
    rad::Result<std::vector<int>, std::vector<MYSTATUS>> value(
        rad::ResultErrTag, { MYSTATUS_UNSUCCESSFUL, MYSTATUS_INVALID_ADDRESS });

    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err().front(), MYSTATUS_UNSUCCESSFUL);
    EXPECT_EQ(value.Err().back(), MYSTATUS_INVALID_ADDRESS);
}

#endif // RAD_ENABLE_STD

TEST(ResultTests, CopyAssignErr)
{
    auto status = MYSTATUS_UNSUCCESSFUL;
    ResultStatus<int> copy = status;
    EXPECT_FALSE(copy.IsOk());
    EXPECT_EQ(copy, MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, CopyConstructErr)
{
    auto status = MYSTATUS_UNSUCCESSFUL;
    ResultStatus<int> copy2(status);
    EXPECT_FALSE(copy2.IsOk());
    EXPECT_EQ(copy2, MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, CopyAssignInitErr)
{
    ResultStatus<int> temp = MYSTATUS_UNSUCCESSFUL;
    EXPECT_FALSE(temp.IsOk());
    EXPECT_EQ(temp, MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, CopyConstructRValErr)
{
    ResultStatus<int> temp2(MYSTATUS_UNSUCCESSFUL);
    EXPECT_FALSE(temp2.IsOk());
    EXPECT_EQ(temp2, MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, CopyConstructErrToErr)
{
    ResultStatus<int> temp = MYSTATUS_UNSUCCESSFUL;
    EXPECT_FALSE(temp.IsOk());
    EXPECT_EQ(temp, MYSTATUS_UNSUCCESSFUL);

    ResultStatus<int> temp2(MYSTATUS_UNSUCCESSFUL);
    EXPECT_FALSE(temp2.IsOk());
    EXPECT_EQ(temp2, MYSTATUS_UNSUCCESSFUL);

    ResultStatus<int> other(temp);
    EXPECT_FALSE(other.IsOk());
    EXPECT_EQ(other, MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, ConstructNonTrivialCopy)
{
    std::vector<int> vec({ 1, 2, 3 });
    ResultStatus<std::vector<int>> value(vec);

    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok().size(), 3u);
    EXPECT_EQ(value.Ok().front(), 1);
    EXPECT_EQ(value.Ok().back(), 3);
    EXPECT_EQ(vec, value.Ok());
}

TEST(ResultTests, ConstructNonTrivialConstCopy)
{
    const std::vector<int> vec({ 1, 2, 3 });
    ResultStatus<std::vector<int>> value(vec);

    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok().size(), 3u);
    EXPECT_EQ(value.Ok().front(), 1);
    EXPECT_EQ(value.Ok().back(), 3);
    EXPECT_EQ(vec, value.Ok());
}

TEST(ResultTests, ConstructNonTrivialCopyRes)
{
    rad::ResultOk<std::vector<int>> vec = std::vector<int>({ 1, 2, 3 });
    ResultStatus<std::vector<int>> value(vec);

    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok().size(), 3u);
    EXPECT_EQ(value.Ok().front(), 1);
    EXPECT_EQ(value.Ok().back(), 3);
    EXPECT_EQ(vec.Get(), value.Ok());
}

TEST(ResultTests, ConstructNonTrivialConstCopyRes)
{
    const rad::ResultOk<std::vector<int>> vec = std::vector<int>({ 1, 2, 3 });
    ResultStatus<std::vector<int>> value(vec);

    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok().size(), 3u);
    EXPECT_EQ(value.Ok().front(), 1);
    EXPECT_EQ(value.Ok().back(), 3);
    EXPECT_EQ(vec.Get(), value.Ok());
}

TEST(ResultTests, ConstructNonTrivialCopyErr)
{
    std::vector<int> vec({ 1, 2, 3 });
    rad::Result<int, std::vector<int>> value(vec);

    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err().size(), 3u);
    EXPECT_EQ(value.Err().front(), 1);
    EXPECT_EQ(value.Err().back(), 3);
    EXPECT_EQ(vec, value.Err());
}

TEST(ResultTests, ConstructNonTrivialConstCopyErr)
{
    const std::vector<int> vec({ 1, 2, 3 });
    rad::Result<int, std::vector<int>> value(vec);

    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err().size(), 3u);
    EXPECT_EQ(value.Err().front(), 1);
    EXPECT_EQ(value.Err().back(), 3);
    EXPECT_EQ(vec, value.Err());
}

TEST(ResultTests, ConstructNonTrivialCopyErrRes)
{
    rad::ResultErr<std::vector<int>> vec = std::vector<int>({ 1, 2, 3 });
    rad::Result<int, std::vector<int>> value(vec);

    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err().size(), 3u);
    EXPECT_EQ(value.Err().front(), 1);
    EXPECT_EQ(value.Err().back(), 3);
    EXPECT_EQ(vec.Get(), value.Err());
}

TEST(ResultTests, ConstructNonTrivialConstCopyErrRes)
{
    const rad::ResultErr<std::vector<int>> vec = std::vector<int>({ 1, 2, 3 });
    rad::Result<int, std::vector<int>> value(vec);

    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err().size(), 3u);
    EXPECT_EQ(value.Err().front(), 1);
    EXPECT_EQ(value.Err().back(), 3);
    EXPECT_EQ(vec.Get(), value.Err());
}

TEST(ResultTests, EmplaceNonTrivialOk)
{
    rad::Result<std::vector<int>, int> value(rad::ResultOkTag, 7u, 1337);

    EXPECT_TRUE(value.IsOk());
    EXPECT_EQ(value.Ok().size(), 7u);
    EXPECT_EQ(value.Ok().front(), 1337);
    EXPECT_EQ(value.Ok().back(), 1337);
}

TEST(ResultTests, EmplaceNonTrivialErr)
{
    rad::Result<int, std::vector<int>> value(rad::ResultErrTag, 7u, 1337);

    EXPECT_TRUE(value.IsErr());
    EXPECT_EQ(value.Err().size(), 7u);
    EXPECT_EQ(value.Err().front(), 1337);
    EXPECT_EQ(value.Err().back(), 1337);
}

TEST(ResultTests, MoveConstructErr)
{
    ResultStatus<int> moved(ResultStatus<int>(MYSTATUS_UNSUCCESSFUL));
    EXPECT_FALSE(moved.IsOk());
    EXPECT_EQ(moved, MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, MoveAssignRValErr)
{
    ResultStatus<int> moved2 = ResultStatus<int>(MYSTATUS_UNSUCCESSFUL);
    EXPECT_FALSE(moved2.IsOk());
    EXPECT_EQ(moved2, MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, AssignRValOkOk)
{
    auto value = 0xc001;
    ResultStatus<int> copy = rad::ResultOk<int>(value);
    EXPECT_TRUE(copy.IsOk());
    EXPECT_EQ(copy, 0xc001);
}

TEST(ResultTests, AssignRValOk)
{
    ResultStatus<int> temp = 0xc001;
    EXPECT_TRUE(temp.IsOk());
    EXPECT_EQ(temp, 0xc001);
}

TEST(ResultTests, CopyConstructOk)
{
    ResultStatus<int> temp = 0xc001;
    EXPECT_TRUE(temp.IsOk());
    EXPECT_EQ(temp, 0xc001);

    ResultStatus<int> other(temp);
    EXPECT_TRUE(other.IsOk());
    EXPECT_EQ(other, 0xc001);
}

TEST(ResultTests, MoveConstructOk)
{
    ResultStatus<int> moved(ResultStatus<int>(rad::ResultOk<int>(0xc001)));
    EXPECT_TRUE(moved.IsOk());
    EXPECT_EQ(moved, 0xc001);
}

TEST(ResultTests, AssignRefOk)
{
    int value = 0xc001;

    ResultStatus<int&> test = value;
    EXPECT_TRUE(test.IsOk());
    EXPECT_EQ(&test.Ok(), &value);
    EXPECT_EQ(test, value);
}

TEST(ResultTests, ConstructRefOkRefOk)
{
    int value = 0xc001;
    ResultStatus<int&> test = value;

    ResultStatus<int&> other(test);
    EXPECT_TRUE(other.IsOk());
    EXPECT_EQ(&other.Ok(), &value);
    EXPECT_EQ(other, value);
}

TEST(ResultTests, ConstructRefError)
{
    ResultStatus<int&> test(rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL));
    EXPECT_FALSE(test.IsOk());
    EXPECT_EQ(test, MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, ConstructRefErrorRefError)
{
    ResultStatus<int&> test(rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL));
    EXPECT_FALSE(test.IsOk());
    EXPECT_EQ(test, MYSTATUS_UNSUCCESSFUL);

    ResultStatus<int&> other(test);
    EXPECT_FALSE(other.IsOk());
    EXPECT_EQ(other, MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, MoveRefErrRVal)
{
    ResultStatus<int&> moved(
        ResultStatus<int&>(rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL)));
    EXPECT_FALSE(moved.IsOk());
    EXPECT_EQ(moved, MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, CopyAssignOkContainer)
{
    auto value = std::vector<int>({ 1, 2, 3 });

    ResultStatus<std::vector<int>> copy =
        rad::ResultOk<std::vector<int>>(value);
    EXPECT_TRUE(copy.IsOk());
    EXPECT_EQ(copy, value);
}

#if RAD_ENABLE_STD

TEST(ResultTests, EmplaceConstructOkContainer)
{
    auto value = std::vector<int>({ 1, 2, 3 });
    ResultStatus<std::vector<int>> temp(rad::ResultOkTag, { 1, 2, 3 });
    EXPECT_TRUE(temp.IsOk());
    EXPECT_EQ(temp, value);
}

TEST(ResultTests, ConstructFromRValueEmplacedOkContainer)
{
    auto value = std::vector<int>({ 1, 2, 3 });
    ResultStatus<std::vector<int>> moved(
        ResultStatus<std::vector<int>>(rad::ResultOkTag, { 1, 2, 3 }));
    EXPECT_TRUE(moved.IsOk());
    EXPECT_EQ(moved, value);
}

#endif // RAD_ENABLE_STD

TEST(ResultTests, RefOkContainer)
{
    auto value = std::vector<int>({ 1, 2, 3 });
    ResultStatus<std::vector<int>&> ref =
        rad::ResultOk<std::vector<int>&>(value);
    EXPECT_TRUE(ref.IsOk());
    EXPECT_EQ(&ref.Ok(), &value);
    EXPECT_EQ(ref, value);
}

TEST(ResultTests, RefOkContainerMultiple)
{
    auto value = std::vector<int>({ 1, 2, 3 });
    ResultStatus<std::vector<int>&> ref =
        rad::ResultOk<std::vector<int>&>(value);
    EXPECT_TRUE(ref.IsOk());
    EXPECT_EQ(&ref.Ok(), &value);
    EXPECT_EQ(ref, value);

    ResultStatus<const std::vector<int>&> ref2 =
        rad::ResultOk<const std::vector<int>&>(value);
    EXPECT_TRUE(ref2.IsOk());
    EXPECT_EQ(&ref2.Ok(), &value);
    EXPECT_EQ(ref2, value);

    ResultStatus<const std::vector<int>&> ref3(ref);
    EXPECT_TRUE(ref3.IsOk());
    EXPECT_EQ(&ref3.Ok(), &value);
    EXPECT_EQ(ref3, value);
}

TEST(ResultTests, ConstructErrContainer)
{
    ResultStatus<std::vector<int>> test(
        rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL));
    EXPECT_FALSE(test.IsOk());
    EXPECT_EQ(test.Err(), MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, EmplaceOkString)
{
    ResultStatus<std::string> test(rad::ResultOkTag, "Hello");
    EXPECT_TRUE(test.IsOk());
    EXPECT_EQ(test, rad::ResultOk<std::string>("Hello"));
}

TEST(ResultTests, RefOkString)
{
    ResultStatus<std::string> test(rad::ResultOkTag, "Hello");
    ResultStatus<std::string&> ref(test);
    EXPECT_TRUE(ref.IsOk());
    EXPECT_EQ(&ref.Ok(), &test.Ok());
    EXPECT_EQ(ref, test);
}

TEST(ResultTests, RefOkStringMultiple)
{
    ResultStatus<std::string> test(rad::ResultOkTag, "Hello");
    ResultStatus<std::string&> ref(test);
    EXPECT_TRUE(ref.IsOk());
    EXPECT_EQ(&ref.Ok(), &test.Ok());
    EXPECT_EQ(ref, test);

    ResultStatus<const std::string&> ref2(test);
    EXPECT_TRUE(ref2.IsOk());
    EXPECT_EQ(&ref2.Ok(), &test.Ok());
    EXPECT_EQ(ref2, test);

    ResultStatus<const std::string&> ref3(ref);
    EXPECT_TRUE(ref3.IsOk());
    EXPECT_EQ(&ref3.Ok(), &test.Ok());
    EXPECT_EQ(ref3, test);
}

TEST(ResultTests, ConstructEmptyResultThenAssign)
{
    ResultStatus<void*> test;
    EXPECT_TRUE(test.IsEmpty());
    EXPECT_FALSE(test.IsErr());
    EXPECT_FALSE(test.IsOk());

    test = rad::Add2Ptr(nullptr, 0x123);
    EXPECT_TRUE(test.IsOk());
    EXPECT_EQ(test, rad::Add2Ptr(nullptr, 0x123));
}

TEST(ResultTests, ConstructErrPointer)
{
    ResultStatus<void*> test2(rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL));
    EXPECT_FALSE(test2.IsOk());
    EXPECT_EQ(test2, MYSTATUS_UNSUCCESSFUL);
}

TEST(ResultTests, SameErrAndOkTypeConstruct)
{
    rad::Result<std::string, std::string> err =
        rad::ResultErr<std::string>("Errant");
    rad::Result<std::string, std::string> ok =
        rad::ResultOk<std::string>("Valid");
    EXPECT_EQ(err, rad::ResultErr<std::string>("Errant"));
    EXPECT_EQ(ok, rad::ResultOk<std::string>("Valid"));
    EXPECT_NE(err, rad::ResultOk<std::string>("Errant"));
    EXPECT_NE(ok, rad::ResultErr<std::string>("Valid"));
    EXPECT_TRUE(err.IsErr());
    EXPECT_FALSE(err.IsOk());
    EXPECT_FALSE(err.IsEmpty());
    EXPECT_FALSE(ok.IsErr());
    EXPECT_TRUE(ok.IsOk());
    EXPECT_FALSE(ok.IsEmpty());
}

struct ConvA
{
    int value;
};

struct ConvB
{
    explicit ConvB(const ConvA& a) : value(a.value) {}
    explicit ConvB(ConvA&& a) : value(rad::Forward<int>(a.value)) {}
    int value;
};

TEST(ResultTests, ConstructConvResult)
{
    ConvA a;
    a.value = 123;

    rad::Result<ConvA, int> value(a);

    rad::Result<ConvB, int> one(static_cast<const decltype(value)&>(value));
    EXPECT_EQ(one.Ok().value, 123);

    rad::Result<ConvB, int> two(rad::Move(value));
    EXPECT_EQ(two.Ok().value, 123);
}

TEST(ResultTests, ConstructMoveResult)
{
    radtest::MoveTester::ResetCounts();
    radtest::MoveTester v;
    v.value = 123;
    rad::Result<radtest::MoveTester, int> one(v);
    rad::Result<radtest::MoveTester, int> two(rad::Move(one));
    EXPECT_EQ(radtest::MoveTester::MoveCount(), 1u);
}

TEST(ResultTests, AssignMoveResult)
{
    radtest::MoveTester::ResetCounts();
    radtest::MoveTester v;
    v.value = 123;
    rad::Result<radtest::MoveTester, int> one(v);
    rad::Result<radtest::MoveTester, int> two(v);
    two = rad::Move(one);
    EXPECT_EQ(radtest::MoveTester::MoveCount(), 1u);
}

TEST(ResultTests, AccessOkByDeref)
{
    ResultStatus<int> test(rad::ResultOk<int>(123));
    EXPECT_EQ(*test, 123);
}

TEST(ResultTests, AccessOkRefByDeref)
{
    ResultStatus<int> test(rad::ResultOk<int>(123));
    EXPECT_EQ(*test, 123);

    ResultStatus<int&> ref(test);
    EXPECT_EQ(&(*ref), &(*test));
    EXPECT_EQ(*ref, *test);

    *ref = 456;
    EXPECT_EQ(*ref, 456);
    EXPECT_EQ(*test, 456);
}

TEST(ResultTests, AccessOkRefByDerefMultiple)
{
    ResultStatus<int> test(rad::ResultOk<int>(123));
    EXPECT_EQ(*test, 123);

    ResultStatus<int&> ref(test);
    EXPECT_EQ(&(*ref), &(*test));
    EXPECT_EQ(*ref, *test);

    ResultStatus<const int&> ref2(test);
    EXPECT_EQ(&(*ref2), &(*test));
    EXPECT_EQ(*ref2, *test);

    ResultStatus<const int&> ref3(ref);
    EXPECT_EQ(&(*ref3), &(*test));
    EXPECT_EQ(*ref3, *test);

    ResultStatus<const int&> ref4(ref3);
    EXPECT_EQ(&(*ref4), &(*test));
    EXPECT_EQ(*ref4, *test);
}

TEST(ResultTests, AccessOkContainerByPointerOperator)
{
    auto value = std::vector<int>({ 1, 2, 3 });

    ResultStatus<std::vector<int>> test =
        rad::ResultOk<std::vector<int>>(value);
    EXPECT_EQ(*test, value);
    EXPECT_EQ(test->front(), value.front());

    const auto& test2 = test;
    EXPECT_EQ(test2->front(), value.front());
}

TEST(ResultTests, AccessOkContainerByPointerOperatorThroughRef)
{
    auto value = std::vector<int>({ 1, 2, 3 });

    ResultStatus<std::vector<int>> test =
        rad::ResultOk<std::vector<int>>(value);
    EXPECT_EQ(*test, value);
    EXPECT_EQ(test->front(), value.front());

    ResultStatus<std::vector<int>&> ref(test);
    EXPECT_EQ(&(*ref), &(*test));
    EXPECT_EQ(*ref, *test);
    EXPECT_EQ(ref->front(), test->front());
}

TEST(ResultTests, AccessOkContainerByPointerOperatorThroughRefMultiple)
{
    auto value = std::vector<int>({ 1, 2, 3 });

    ResultStatus<std::vector<int>> test =
        rad::ResultOk<std::vector<int>>(value);
    EXPECT_EQ(*test, value);
    EXPECT_EQ(test->front(), value.front());

    ResultStatus<std::vector<int>&> ref(test);
    EXPECT_EQ(&(*ref), &(*test));
    EXPECT_EQ(*ref, *test);
    EXPECT_EQ(ref->front(), test->front());

    ResultStatus<const std::vector<int>&> ref2(test);
    EXPECT_EQ(&(*ref2), &(*test));
    EXPECT_EQ(*ref2, *test);
    EXPECT_EQ(ref2->front(), test->front());

    ResultStatus<const std::vector<int>&> ref3(ref);
    EXPECT_EQ(&(*ref3), &(*test));
    EXPECT_EQ(*ref3, *test);
    EXPECT_EQ(ref3->front(), test->front());

    ResultStatus<const std::vector<int>&> ref4(ref3);
    EXPECT_EQ(&(*ref4), &(*test));
    EXPECT_EQ(*ref4, *test);
    EXPECT_EQ(ref4->front(), test->front());
}

TEST(ResultTests, AccessOkPointerThroughDeref)
{
    auto value = rad::Add2Ptr(nullptr, 123);
    ResultStatus<void*> test = rad::ResultOk<void*>(value);
    EXPECT_EQ(*test, value);
}

TEST(ResultTests, AccessOkContainerPointerThroughDerefAndPointerOperator)
{
    auto value = new std::vector<int>({ 1, 2, 3 });
    ResultStatus<std::vector<int>*> test =
        rad::ResultOk<std::vector<int>*>(value);
    EXPECT_EQ(*test, value);
    EXPECT_EQ(**test, *value);
    EXPECT_EQ((*test)->front(), value->front());
    delete value;
}

TEST(ResultTests, AccessConstOkDeref)
{
    const ResultStatus<int> value = 1;
    EXPECT_EQ(*value, 1);
}

TEST(ResultTests, ResCopyAssign)
{
    const rad::ResultOk<std::vector<int>> value({ 1, 2, 3 });
    rad::ResultOk<std::vector<int>> other;
    other = value;
    EXPECT_EQ(value.Get().size(), 3u);
    EXPECT_EQ(value.Get().front(), 1);
    EXPECT_EQ(value.Get().back(), 3);
    EXPECT_EQ(other.Get().size(), 3u);
    EXPECT_EQ(other.Get().front(), 1);
    EXPECT_EQ(other.Get().back(), 3);
}

TEST(ResultTests, ResMoveAssign)
{
    rad::ResultOk<std::vector<int>> value({ 1, 2, 3 });
    rad::ResultOk<std::vector<int>> other;
    other = rad::Move(value);
    EXPECT_EQ(value.Get().size(), 0u);
    EXPECT_TRUE(value.Get().empty());
    EXPECT_EQ(other.Get().size(), 3u);
    EXPECT_EQ(other.Get().front(), 1);
    EXPECT_EQ(other.Get().back(), 3);
}

TEST(ResultTests, CoverNonTrivalEmptyDestruct)
{
    ResultStatus<std::vector<int>> value;
    EXPECT_TRUE(value.IsEmpty());
}

TEST(ResultTests, SortableMap)
{
    std::map<ResultStatus<int>, int> map;
    for (int i = 1; i <= 10; i++)
    {
        map[i] = i;
        map[MYSTATUS_INVALID_ADDRESS] = i;
    }

    EXPECT_EQ(map.size(), 11u);
    EXPECT_EQ(map[MYSTATUS_INVALID_ADDRESS], 10);

    for (int i = 1; i <= 10; i++)
    {
        EXPECT_EQ(map[i], i);
    }

    for (int i = 1; i <= 10; i++)
    {
        map[rad::ResultErr<MYSTATUS>(i)] = i;
    }

    EXPECT_EQ(map.size(), 21u);

    for (int i = 1; i <= 10; i++)
    {
        EXPECT_EQ(map[i], i);
    }

    for (int i = 1; i <= 10; i++)
    {
        EXPECT_EQ(map[rad::ResultErr<MYSTATUS>(i)], i);
    }

    auto empty = ResultStatus<int>();
    map[empty] = 999;
    EXPECT_EQ(map[empty], 999);
    map[empty] = 1000;
    EXPECT_EQ(map[empty], 1000);

    int iok = 0;
    int ierr = 0;
    bool foundEmpty = false;
    bool foundErr = false;
    for (const auto& entry : map)
    {
        if (entry.first.IsEmpty())
        {
            EXPECT_EQ(entry.second, 1000);
            foundEmpty = true;
        }
        else if (entry.first.IsErr())
        {
            if (entry.first == MYSTATUS_INVALID_ADDRESS)
            {
                EXPECT_EQ(entry.second, 10);
                foundErr = true;
            }
            else
            {
                ierr++;
                EXPECT_EQ(entry.first, rad::ResultErr<MYSTATUS>(ierr));
                EXPECT_EQ(entry.second, ierr);
            }
        }
        else if (entry.first.IsOk())
        {
            iok++;
            EXPECT_EQ(entry.first, iok);
            EXPECT_EQ(entry.second, iok);
        }
    }

    EXPECT_EQ(foundEmpty, true);
    EXPECT_EQ(foundErr, true);
    EXPECT_EQ(iok, 10);
    EXPECT_EQ(ierr, 10);
}

TEST(ResultTests, SortableSet)
{
    std::set<ResultStatus<int>> set;

    set.emplace(rad::ResultOk<int>(4));
    set.emplace(rad::ResultErr<MYSTATUS>(1));
    set.emplace(rad::ResultOk<int>(2));
    set.emplace(rad::ResultErr<MYSTATUS>(5));
    set.emplace(rad::ResultOk<int>(5));
    set.emplace(ResultStatus<int>());
    set.emplace(rad::ResultOk<int>(1));
    set.emplace(rad::ResultErr<MYSTATUS>(3));

    EXPECT_EQ(set.size(), 8u);
    auto it = set.begin();
    EXPECT_EQ(it->IsEmpty(), true);
    EXPECT_EQ(it->IsOk(), false);
    EXPECT_EQ(it->IsErr(), false);
    ++it;
    EXPECT_EQ(it->IsEmpty(), false);
    EXPECT_EQ(it->IsOk(), true);
    EXPECT_EQ(it->IsErr(), false);
    EXPECT_EQ(it->Ok(), 1);
    ++it;
    EXPECT_EQ(it->IsEmpty(), false);
    EXPECT_EQ(it->IsOk(), true);
    EXPECT_EQ(it->IsErr(), false);
    EXPECT_EQ(it->Ok(), 2);
    ++it;
    EXPECT_EQ(it->IsEmpty(), false);
    EXPECT_EQ(it->IsOk(), true);
    EXPECT_EQ(it->IsErr(), false);
    EXPECT_EQ(it->Ok(), 4);
    ++it;
    EXPECT_EQ(it->IsEmpty(), false);
    EXPECT_EQ(it->IsOk(), true);
    EXPECT_EQ(it->IsErr(), false);
    EXPECT_EQ(it->Ok(), 5);
    ++it;
    EXPECT_EQ(it->IsEmpty(), false);
    EXPECT_EQ(it->IsOk(), false);
    EXPECT_EQ(it->IsErr(), true);
    EXPECT_EQ(it->Err(), 1);
    ++it;
    EXPECT_EQ(it->IsEmpty(), false);
    EXPECT_EQ(it->IsOk(), false);
    EXPECT_EQ(it->IsErr(), true);
    EXPECT_EQ(it->Err(), 3);
    ++it;
    EXPECT_EQ(it->IsEmpty(), false);
    EXPECT_EQ(it->IsOk(), false);
    EXPECT_EQ(it->IsErr(), true);
    EXPECT_EQ(it->Err(), 5);
}

TEST(ResultTests, Or)
{
    ResultStatus<int> res = rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL);
    EXPECT_EQ(res.Or(123), 123);
    res = 456;
    EXPECT_EQ(res.Or(123), 456);
    int value = 789;
    EXPECT_EQ(res.Or(static_cast<const int&>(value)), 456);
    res = MYSTATUS_UNSUCCESSFUL;
    EXPECT_EQ(res.Or(static_cast<const int&>(value)), 789);
}

TEST(ResultTests, OnErr)
{
    ResultStatus<int> res = rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL);
    EXPECT_EQ(res.OnErr<unsigned long>(12u).Err(), 12u);
    res = 456;
    EXPECT_EQ(res.OnErr<unsigned long>(12u).Ok(), 456);
}

TEST(ResultTests, ConstOnErr)
{
    const ResultStatus<int> resErr = rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL);
    EXPECT_EQ(resErr.OnErr<unsigned long>(12u).Err(), 12u);
    const ResultStatus<int> resOk = 456;
    EXPECT_EQ(resOk.OnErr<unsigned long>(12u).Ok(), 456);
}

TEST(ResultTests, OnOk)
{
    ResultStatus<int> res = rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL);
    EXPECT_EQ(res.OnOk<unsigned long>(12u).Err(), MYSTATUS_UNSUCCESSFUL);
    res = 456;
    EXPECT_EQ(res.OnOk<unsigned long>(12u).Ok(), 12u);
}

TEST(ResultTests, ConstOnOk)
{
    const ResultStatus<int> resErr = rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL);
    EXPECT_EQ(resErr.OnOk<unsigned long>(12u).Err(), MYSTATUS_UNSUCCESSFUL);
    const ResultStatus<int> resOk = 456;
    EXPECT_EQ(resOk.OnOk<unsigned long>(12u).Ok(), 12u);
}

TEST(ResultTests, ConstOr)
{
    const ResultStatus<int> resErr =
        rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL);
    EXPECT_EQ(resErr.Or(123), 123);
    const ResultStatus<int> resOk = 456;
    EXPECT_EQ(resOk.Or(123), 456);
}

TEST(ResultTests, ExplicitBool)
{
    ResultStatus<int> value;

    EXPECT_FALSE(value);

    if (value)
    {
        EXPECT_TRUE(false);
    }
    else
    {
        EXPECT_TRUE(true);
    }

    value = MYSTATUS_UNSUCCESSFUL;

    EXPECT_FALSE(value);

    if (value)
    {
        EXPECT_TRUE(false);
    }
    else
    {
        EXPECT_TRUE(true);
    }

    value = 123;

    EXPECT_TRUE(value);

    if (value)
    {
        EXPECT_TRUE(true);
    }
    else
    {
        EXPECT_FALSE(true);
    }
}

TEST(ResultTests, CompareOkOk)
{
    ResultStatus<int> left = 1;
    ResultStatus<int> right = 1;

    EXPECT_TRUE(left == right);
    EXPECT_TRUE(right == left);
    EXPECT_FALSE(left != right);
    EXPECT_FALSE(right != left);

    EXPECT_FALSE(left < right);
    EXPECT_FALSE(right < left);
}

TEST(ResultTests, CompareOkErr)
{
    ResultStatus<int> left = 1;
    ResultStatus<int> right = MYSTATUS_UNSUCCESSFUL;

    EXPECT_FALSE(left == right);
    EXPECT_FALSE(right == left);
    EXPECT_TRUE(left != right);
    EXPECT_TRUE(right != left);

    EXPECT_TRUE(left < right);
    EXPECT_FALSE(right < left);
}

TEST(ResultTests, CompareOkEmpty)
{
    ResultStatus<int> left = 1;
    ResultStatus<int> right;

    EXPECT_FALSE(left == right);
    EXPECT_FALSE(right == left);
    EXPECT_TRUE(left != right);
    EXPECT_TRUE(right != left);

    EXPECT_FALSE(left < right);
    EXPECT_TRUE(right < left);
}

TEST(ResultTests, CompareValue)
{
    ResultStatus<int> value;

    EXPECT_FALSE(value == 1);
    EXPECT_FALSE(1 == value);
    EXPECT_TRUE(value != 1);
    EXPECT_TRUE(1 != value);
    EXPECT_FALSE(value == MYSTATUS_UNSUCCESSFUL);
    EXPECT_FALSE(MYSTATUS_UNSUCCESSFUL == value);
    EXPECT_TRUE(value != MYSTATUS_UNSUCCESSFUL);
    EXPECT_TRUE(MYSTATUS_UNSUCCESSFUL != value);

    value = 1;

    EXPECT_TRUE(value == 1);
    EXPECT_TRUE(1 == value);
    EXPECT_FALSE(value != 1);
    EXPECT_FALSE(1 != value);
    EXPECT_FALSE(value == MYSTATUS_UNSUCCESSFUL);
    EXPECT_FALSE(MYSTATUS_UNSUCCESSFUL == value);
    EXPECT_TRUE(value != MYSTATUS_UNSUCCESSFUL);
    EXPECT_TRUE(MYSTATUS_UNSUCCESSFUL != value);

    value = MYSTATUS_UNSUCCESSFUL;

    EXPECT_FALSE(value == 1);
    EXPECT_FALSE(1 == value);
    EXPECT_TRUE(value != 1);
    EXPECT_TRUE(1 != value);
    EXPECT_TRUE(value == MYSTATUS_UNSUCCESSFUL);
    EXPECT_TRUE(MYSTATUS_UNSUCCESSFUL == value);
    EXPECT_FALSE(value != MYSTATUS_UNSUCCESSFUL);
    EXPECT_FALSE(MYSTATUS_UNSUCCESSFUL != value);
}

TEST(ResultTests, CompareRes)
{
    ResultStatus<int> value;

    EXPECT_FALSE(value == rad::ResultOk<int>(1));
    EXPECT_FALSE(rad::ResultOk<int>(1) == value);
    EXPECT_TRUE(value != rad::ResultOk<int>(1));
    EXPECT_TRUE(rad::ResultOk<int>(1) != value);
    EXPECT_FALSE(value == rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL));
    EXPECT_FALSE(rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL) == value);
    EXPECT_TRUE(value != rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL));
    EXPECT_TRUE(rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL) != value);

    value = 1;

    EXPECT_TRUE(value == rad::ResultOk<int>(1));
    EXPECT_TRUE(rad::ResultOk<int>(1) == value);
    EXPECT_FALSE(value != rad::ResultOk<int>(1));
    EXPECT_FALSE(rad::ResultOk<int>(1) != value);
    EXPECT_FALSE(value == rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL));
    EXPECT_FALSE(rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL) == value);
    EXPECT_TRUE(value != rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL));
    EXPECT_TRUE(rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL) != value);

    value = MYSTATUS_UNSUCCESSFUL;

    EXPECT_FALSE(value == rad::ResultOk<int>(1));
    EXPECT_FALSE(rad::ResultOk<int>(1) == value);
    EXPECT_TRUE(value != rad::ResultOk<int>(1));
    EXPECT_TRUE(rad::ResultOk<int>(1) != value);
    EXPECT_TRUE(value == rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL));
    EXPECT_TRUE(rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL) == value);
    EXPECT_FALSE(value != rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL));
    EXPECT_FALSE(rad::ResultErr<MYSTATUS>(MYSTATUS_UNSUCCESSFUL) != value);
}
