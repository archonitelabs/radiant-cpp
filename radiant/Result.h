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

#include "radiant/TypeWrapper.h"

#include <initializer_list>

namespace rad
{

/// @brief States that a result can be in. Indicates what information the result
/// holds. A "valid" result is "Ok", an "errant" result is "Err", and an "Empty"
/// result holds neither "Ok" or "err" state. Generally, results are never in
/// the empty state, users need not check for it.
enum class ResultState : uint8_t
{
    Empty,
    Valid,
    Errant
};

/// @brief Result "Ok" tag type, indicator for emplacement construction.
struct ResultEmptyTagType
{
};

/// @brief Result "Err" tag type, indicator for emplacement construction.
struct ResultOkTagType
{
};

/// @brief Result "Empty" tag type, indicator for explicit default construction.
struct ResultErrTagType
{
};

/// @brief Result "Ok" tag, indicator for emplacement construction.
RAD_INLINE_VAR constexpr ResultOkTagType ResultOkTag{};

/// @brief Result "Err" tag type, indicator for emplacement construction.
RAD_INLINE_VAR constexpr ResultErrTagType ResultErrTag{};

/// @brief Result "Empty" tag, indicator for explicit default construction.
RAD_INLINE_VAR constexpr ResultEmptyTagType ResultEmptyTag{};

namespace detail
{

template <typename T, typename E, bool = (IsTrivDtor<T> && IsTrivDtor<E>)>
struct ResultStorage;

/// @brief Result storage specialization for trivially destructible data.
/// @tparam T "Ok" type.
/// @tparam E "Err" type.
template <typename T, typename E>
struct ResultStorage<T, E, true>
{
    using OkWrap = TypeWrapper<T>;
    using ErrWrap = TypeWrapper<E>;
    using OkType = T;
    using ErrType = E;

    ~ResultStorage() noexcept = default;

    constexpr ResultStorage() noexcept
        : m_state(ResultState::Empty)
#if !RAD_CPP17
          ,
          m_default(false)
#endif
    {
    }

    constexpr ResultStorage(ResultEmptyTagType) noexcept
        : ResultStorage()
    {
    }

    template <typename... TArgs>
    constexpr ResultStorage(ResultOkTagType, TArgs&&... args) noexcept(
        IsNoThrowCtor<OkWrap, TArgs&&...>)
        : m_state(ResultState::Valid),
          m_ok(Forward<TArgs>(args)...)
    {
        RAD_S_ASSERT_NOTHROW((IsNoThrowCtor<OkWrap, TArgs...>));
    }

    template <typename... TArgs>
    constexpr ResultStorage(ResultErrTagType, TArgs&&... args) noexcept(
        IsNoThrowCtor<ErrWrap, TArgs&&...>)
        : m_state(ResultState::Errant),
          m_err(Forward<TArgs>(args)...)
    {
        RAD_S_ASSERT_NOTHROW((IsNoThrowCtor<ErrWrap, TArgs&&...>));
    }

    constexpr void Construct(ResultEmptyTagType) noexcept
    {
        m_state = ResultState::Empty;
    }

    template <typename... TArgs>
    constexpr void Construct(ResultOkTagType, TArgs&&... args) noexcept(
        IsNoThrowCtor<OkWrap, TArgs&&...>)
    {
        RAD_S_ASSERT_NOTHROW((IsNoThrowCtor<OkWrap, TArgs&&...>));
        RAD_ASSERT(m_state == ResultState::Empty);

        new (&m_ok) OkWrap(Forward<TArgs>(args)...);
        m_state = ResultState::Valid;
    }

    template <typename... TArgs>
    constexpr void Construct(ResultErrTagType, TArgs&&... args) noexcept(
        IsNoThrowCtor<ErrWrap, TArgs&&...>)
    {
        RAD_S_ASSERT_NOTHROW(noexcept(IsNoThrowCtor<ErrWrap, TArgs&&...>));
        RAD_ASSERT(m_state == ResultState::Empty);

        new (&m_err) ErrWrap(Forward<TArgs>(args)...);
        m_state = ResultState::Errant;
    }

    void Destruct() noexcept
    {
        m_state = ResultState::Empty;
    }

    ResultState m_state;

    union
    {
        OkWrap m_ok;
        ErrWrap m_err;
        bool m_default;
    };
};

/// @brief Result storage specialization for non-trivially destructible data.
/// @tparam T "Ok" type.
/// @tparam E "Err" type.
template <typename T, typename E>
struct ResultStorage<T, E, false>
{
    using OkWrap = TypeWrapper<T>;
    using ErrWrap = TypeWrapper<E>;
    using OkType = T;
    using ErrType = E;

    ~ResultStorage() noexcept
    {
        RAD_S_ASSERT_NOTHROW_DTOR(IsNoThrowDtor<T> && IsNoThrowDtor<E>);

        Destruct();
    }

    constexpr ResultStorage() noexcept
        : m_state(ResultState::Empty)
#if !RAD_CPP17
          ,
          m_default(false)
#endif
    {
    }

    constexpr ResultStorage(ResultEmptyTagType) noexcept
        : ResultStorage()
    {
    }

    template <typename... TArgs>
    constexpr ResultStorage(ResultOkTagType, TArgs&&... args) noexcept(
        IsNoThrowCtor<OkWrap, TArgs&&...>)
        : m_state(ResultState::Valid),
          m_ok(Forward<TArgs>(args)...)
    {
        RAD_S_ASSERT_NOTHROW((IsNoThrowCtor<OkWrap, TArgs&&...>));
    }

    template <typename... TArgs>
    constexpr ResultStorage(ResultErrTagType, TArgs&&... args) noexcept(
        IsNoThrowCtor<ErrWrap, TArgs&&...>)
        : m_state(ResultState::Errant),
          m_err(Forward<TArgs>(args)...)
    {
        RAD_S_ASSERT_NOTHROW((IsNoThrowCtor<ErrWrap, TArgs&&...>));
    }

    constexpr void Construct(ResultEmptyTagType) noexcept
    {
        m_state = ResultState::Empty;
    }

    template <typename... TArgs>
    constexpr void Construct(ResultOkTagType, TArgs&&... args) noexcept(
        IsNoThrowCtor<OkWrap, TArgs&&...>)
    {
        RAD_S_ASSERT_NOTHROW((IsNoThrowCtor<OkWrap, TArgs&&...>));
        RAD_ASSERT(m_state == ResultState::Empty);

        new (&m_ok) OkWrap(Forward<TArgs>(args)...);
        m_state = ResultState::Valid;
    }

    template <typename... TArgs>
    constexpr void Construct(ResultErrTagType, TArgs&&... args) noexcept(
        IsNoThrowCtor<ErrWrap, TArgs&&...>)
    {
        RAD_S_ASSERT_NOTHROW((IsNoThrowCtor<ErrWrap, TArgs&&...>));
        RAD_ASSERT(m_state == ResultState::Empty);

        new (&m_err) ErrWrap(Forward<TArgs>(args)...);
        m_state = ResultState::Errant;
    }

    void Destruct() noexcept
    {
        if (m_state == ResultState::Valid)
        {
            m_ok.~OkWrap();
            m_state = ResultState::Empty;
        }
        else if (m_state == ResultState::Errant)
        {
            m_err.~ErrWrap();
            m_state = ResultState::Empty;
        }
        else
        {
            RAD_ASSERT(m_state == ResultState::Empty);
        }
    }

    ResultState m_state;

    union
    {
        OkWrap m_ok;
        ErrWrap m_err;
        bool m_default;
    };
};

template <typename T, ResultState State>
struct ResultTypeWrapper : public TypeWrapper<T>
{
    using TypeWrapper<T>::TypeWrapper;
};

} // namespace detail

/// @brief Container that associates a type with valid "Ok" state.
/// @tparam T Type to associate.
/// @details Generally used for "casting" to an "Ok" result when a result object
/// holds identical "Ok" and "Err" types. May also be used to declare some
/// commonly used "Ok" result.
template <typename T>
using ResultOk = detail::ResultTypeWrapper<T, ResultState::Valid>;

/// @brief Container that associates a type with valid "Err" state.
/// @tparam T Type to associate.
/// @details Generally used for "casting" to an "Err" result when a result
/// object holds identical "Ok" and "Err" types. May also be used to declare
/// some commonly used "Err" result.
template <typename E>
using ResultErr = detail::ResultTypeWrapper<E, ResultState::Errant>;

/// @brief Result object that stores a valid "Ok" result or an errant "Err".
/// This is useful for communicating either some valid result or an error.
/// @tparam T "Ok" type.
/// @tparam E "Err" type.
/// @details A result may also be "empty". This state exists for the sake of
/// default initialization and semantics/patterns of use. An empty result should
/// never be returned to a caller in practice and should only be in one of two
/// states, either "Ok" or "Err". Therefore a user need not check if a result is
/// "empty".
/// @code{.cpp}
/// #include <iostream>
/// #include <string>
///
/// std::string g_HelloWorld;
///
/// rad::Result<const std::string&, int> GetGreeting()
/// {
///     if (g_HelloWold.empty())
///     {
///         return -1;
///     }
///
///     return g_HelloWorld;
/// }
///
/// int main()
/// {
///     auto hello = GetGreeting();
///     if (!hello.IsOk())
///     {
///         return hello.Err();
///     }
///
///     std::cout << hello.Ok();
///
///     return 0;
/// }
/// @endcode
template <typename T, typename E>
class RAD_NODISCARD Result final : private detail::ResultStorage<T, E>
{
public:

    using StorageType = detail::ResultStorage<T, E>;
    using ThisType = Result<T, E>;
    using typename StorageType::OkType;
    using typename StorageType::ErrType;

    RAD_S_ASSERT_NOTHROW_MOVE_T(T);
    RAD_S_ASSERT_NOTHROW_MOVE_T(E);

    constexpr Result() noexcept
        : StorageType(ResultEmptyTag)
    {
    }

    constexpr Result(const ResultEmptyTagType&) noexcept
        : StorageType(ResultEmptyTag)
    {
    }

    // Ok ctors
    template <typename... TArgs,
              EnIf<IsCtor<StorageType, ResultOkTagType, TArgs...>, int> = 0>
    constexpr Result(ResultOkTagType, TArgs&&... args) noexcept(
        IsNoThrowCtor<StorageType, ResultOkTagType, TArgs&&...>)
        : StorageType(ResultOkTag, Forward<TArgs>(args)...)
    {
        RAD_S_ASSERT_NOTHROW(
            (IsNoThrowCtor<StorageType, ResultOkTagType, TArgs&&...>));
    }

    template <
        typename U = OkType,
        EnIf<!IsRelated<U, ErrType> && IsCtor<StorageType, ResultOkTagType, U>,
             int> = 0>
    constexpr Result(const OkType& value) noexcept(
        IsNoThrowCtor<StorageType, ResultOkTagType, const OkType&>)
        : StorageType(ResultOkTag, value)
    {
        RAD_S_ASSERT_NOTHROW(
            (IsNoThrowCtor<StorageType, ResultOkTagType, const OkType&>));
    }

    template <typename U = OkType,
              EnIf<!IsRef<U> && !IsRelated<U, ErrType> &&
                       IsCtor<StorageType, ResultOkTagType, U>,
                   int> = 0>
    constexpr Result(OkType&& value) noexcept(
        IsNoThrowCtor<StorageType, ResultOkTagType, OkType&&>)
        : StorageType(ResultOkTag, Forward<OkType>(value))
    {
        RAD_S_ASSERT_NOTHROW(
            (IsNoThrowCtor<StorageType, ResultOkTagType, OkType&&>));
    }

#if RAD_ENABLE_STD
    template <typename U>
    constexpr Result(ResultOkTagType, std::initializer_list<U> init) noexcept(
        IsNoThrowCtor<StorageType, ResultOkTagType, std::initializer_list<U>>)
        : StorageType(ResultOkTag, Forward<std::initializer_list<U>>(init))
    {
        RAD_S_ASSERT_NOTHROW((IsNoThrowCtor<StorageType,
                                            ResultOkTagType,
                                            std::initializer_list<U>>));
    }
#endif

    template <typename U>
    constexpr Result(const ResultOk<U>& value) noexcept(
        IsNoThrowCtor<StorageType, ResultOkTagType, const U&>)
        : StorageType(ResultOkTag, value.Get())
    {
        RAD_S_ASSERT_NOTHROW(
            (IsNoThrowCtor<StorageType, ResultOkTagType, const U&>));
    }

    template <typename U>
    constexpr Result(ResultOk<U>&& value) noexcept(
        IsNoThrowCtor<StorageType, ResultOkTagType, ResultOk<U>&&>)
        : StorageType(ResultOkTag, Forward<ResultOk<U>>(value))
    {
        RAD_S_ASSERT_NOTHROW(
            (IsNoThrowCtor<StorageType, ResultOkTagType, ResultOk<U>&&>));
    }

    // Err ctors
    template <typename... TArgs,
              EnIf<IsCtor<StorageType, ResultErrTagType, TArgs...>, int> = 0>
    constexpr Result(ResultErrTagType, TArgs&&... args) noexcept(
        IsNoThrowCtor<StorageType, ResultErrTagType, TArgs&&...>)
        : StorageType(ResultErrTag, Forward<TArgs>(args)...)
    {
        RAD_S_ASSERT_NOTHROW(
            (IsNoThrowCtor<StorageType, ResultErrTagType, TArgs&&...>));
    }

    template <typename U = ErrType,
              EnIf<!IsRelated<OkType, U> &&
                       IsCtor<StorageType, ResultErrTagType, const U&>,
                   int> = 0>
    constexpr Result(const ErrType& value) noexcept(
        IsNoThrowCtor<StorageType, ResultErrTagType, const ErrType&>)
        : StorageType(ResultErrTag, value)
    {
        RAD_S_ASSERT_NOTHROW(
            (IsNoThrowCtor<StorageType, ResultErrTagType, const ErrType&>));
    }

    template <typename U = ErrType,
              EnIf<!IsRef<U> && !IsRelated<U, OkType> &&
                       IsCtor<StorageType, ResultErrTagType, U>,
                   int> = 0>
    constexpr Result(ErrType&& value) noexcept(
        IsNoThrowCtor<StorageType, ResultErrTagType, ErrType&&>)
        : StorageType(ResultErrTag, Forward<ErrType>(value))
    {
        RAD_S_ASSERT_NOTHROW(
            (IsNoThrowCtor<StorageType, ResultErrTagType, ErrType&&>));
    }

#if RAD_ENABLE_STD
    template <typename U>
    constexpr Result(ResultErrTagType, std::initializer_list<U> init) noexcept(
        IsNoThrowCtor<StorageType, ResultErrTagType, std::initializer_list<U>>)
        : StorageType(ResultErrTag, Forward<std::initializer_list<U>>(init))
    {
        RAD_S_ASSERT_NOTHROW((IsNoThrowCtor<StorageType,
                                            ResultErrTagType,
                                            std::initializer_list<U>>));
    }
#endif

    template <typename U = ErrType, EnIf<IsCtor<ErrType, U>, int> = 0>
    constexpr Result(const ResultErr<U>& value) noexcept(
        IsNoThrowCtor<StorageType, ResultErrTagType, const U&>)
        : StorageType(ResultErrTag, value.Get())
    {
        RAD_S_ASSERT_NOTHROW(
            (IsNoThrowCtor<StorageType, ResultErrTagType, const U&>));
    }

    template <typename U = ErrType, EnIf<IsCtor<ErrType, U>, int> = 0>
    constexpr Result(ResultErr<U>&& value) noexcept(
        IsNoThrowCtor<StorageType, ResultErrTagType, ResultErr<U>&&>)
        : StorageType(ResultErrTag, Forward<ResultErr<U>>(value))
    {
        RAD_S_ASSERT_NOTHROW(
            (IsNoThrowCtor<StorageType, ResultErrTagType, ResultErr<U>&&>));
    }

    // Copy/Move ctors
    constexpr Result(const Result& r) noexcept(
        noexcept(DeclVal<Result*>()->CopyCtor(DeclVal<const Result&>())))
        : StorageType()
    {
        RAD_S_ASSERT_NOTHROW(noexcept(CopyCtor(r)));
        CopyCtor(r);
    }

    constexpr Result(Result&& r) noexcept(
        noexcept(DeclVal<Result*>()->MoveCtor(DeclVal<Result&&>())))
        : StorageType()
    {
        RAD_S_ASSERT_NOTHROW(noexcept(MoveCtor(r)));
        MoveCtor(r);
    }

    template <typename O,
              typename F,
              EnIf<(IsCtor<OkType, O> || IsLRefBindable<OkType, O>) &&
                       (IsCtor<ErrType, F> || IsLRefBindable<ErrType, F>),
                   int> = 0>
    constexpr Result(Result<O, F>& r) noexcept(
        noexcept(DeclVal<Result*>()->CopyCtor(r)))
        : StorageType()
    {
        RAD_S_ASSERT_NOTHROW(noexcept(CopyCtor(r)));
        CopyCtor(r);
    }

    template <
        typename O,
        typename F,
        EnIf<(IsCtor<OkType, const O&> && IsCtor<ErrType, const F&>), int> = 0>
    constexpr Result(const Result<O, F>& r) noexcept(
        noexcept(DeclVal<Result*>()->CopyCtor(r)))
        : StorageType()
    {
        RAD_S_ASSERT_NOTHROW(noexcept(CopyCtor(r)));
        CopyCtor(r);
    }

    template <typename O,
              typename F,
              EnIf<(IsCtor<OkType, O&&> && IsCtor<ErrType, F&&>), int> = 0>
    constexpr Result(Result<O, F>&& r) noexcept(
        noexcept(DeclVal<Result*>()->MoveCtor(r)))
        : StorageType()
    {
        RAD_S_ASSERT_NOTHROW(noexcept(MoveCtor(r)));
        MoveCtor(r);
    }

    // Assignment
    Result& Assign(const Result& r) noexcept(
        noexcept(DeclVal<Result*>()->Copy(r)))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(Copy(r)));
        Copy(r);
        return *this;
    }

    Result& Assign(Result&& r) noexcept(
        noexcept(DeclVal<Result*>()->MoveAssign(Forward<Result>(r))))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(MoveAssign(Forward<Result>(r))));
        MoveAssign(Forward<Result>(r));
        return *this;
    }

    template <typename U = OkType, EnIf<!IsRelated<U, ErrType>, int> = 0>
    constexpr Result& Assign(const OkType& r) noexcept(
        noexcept(DeclVal<Result*>()->m_ok = r) && //
        noexcept(DeclVal<Result*>()->Construct(ResultOkTag, r)))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(this->m_ok = r) &&
                             noexcept(Construct(ResultOkTag, r)));
        if (IsOk())
        {
            this->m_ok = r;
        }
        else
        {
            Destruct();
            Construct(ResultOkTag, r);
        }
        return *this;
    }

    template <typename U = OkType,
              EnIf<!IsRef<U> && !IsRelated<U, ErrType>, int> = 0>
    constexpr Result& Assign(OkType&& r) noexcept(
        noexcept(DeclVal<Result*>()->m_ok = Forward<OkType>(r)) && //
        noexcept(DeclVal<Result*>()->Construct(ResultOkTag,
                                               Forward<OkType>(r))))
    {
        RAD_S_ASSERT_NOTHROW(
            noexcept(this->m_ok = Forward<OkType>(r)) &&
            noexcept(Construct(ResultOkTag, Forward<OkType>(r))));
        if (IsOk())
        {
            this->m_ok = Forward<OkType>(r);
        }
        else
        {
            Destruct();
            Construct(ResultOkTag, Forward<OkType>(r));
        }
        return *this;
    }

    template <typename U = ErrType, EnIf<!IsRelated<OkType, U>, int> = 0>
    constexpr Result& Assign(const ErrType& r) noexcept(
        noexcept(DeclVal<Result*>()->m_err = r) && //
        noexcept(DeclVal<Result*>()->Construct(ResultErrTag, r)))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(this->m_err = r) &&
                             noexcept(Construct(ResultErrTag, r)));
        if (IsErr())
        {
            this->m_err = r;
        }
        else
        {
            Destruct();
            Construct(ResultErrTag, r);
        }
        return *this;
    }

    template <typename U = ErrType,
              EnIf<!IsRef<U> && !IsRelated<OkType, U>, int> = 0>
    constexpr Result& Assign(ErrType&& r) noexcept(
        noexcept(DeclVal<Result*>()->m_err = Forward<ErrType>(r)) && //
        noexcept(DeclVal<Result*>()->Construct(ResultErrTag,
                                               Forward<ErrType>(r))))
    {
        RAD_S_ASSERT_NOTHROW(
            noexcept(this->m_err = Forward<ErrType>(r)) &&
            noexcept(Construct(ResultErrTag, Forward<ErrType>(r))));
        if (IsErr())
        {
            this->m_err = Forward<ErrType>(r);
        }
        else
        {
            Destruct();
            Construct(ResultErrTag, Forward<ErrType>(r));
        }
        return *this;
    }

    template <typename U>
    constexpr Result& Assign(const ResultOk<U>& r) noexcept(
        noexcept(DeclVal<Result*>()->m_ok = r.Get()) && //
        noexcept(DeclVal<Result*>()->Construct(ResultOkTag, r.Get())))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(this->m_ok = r.Get()) &&
                             noexcept(Construct(ResultOkTag, r.Get())));
        if (IsOk())
        {
            this->m_ok = r.Get();
        }
        else
        {
            Destruct();
            Construct(ResultOkTag, r.Get());
        }
        return *this;
    }

    template <typename U>
    constexpr Result& Assign(ResultOk<U>&& r) noexcept(
        noexcept(DeclVal<Result*>()->m_ok = Forward<ResultOk<U>>(r).Get()) && //
        noexcept(DeclVal<Result*>()->Construct(ResultOkTag,
                                               Forward<ResultOk<U>>(r).Get())))
    {
        // clang-format off
        RAD_S_ASSERT_NOTHROW(
            noexcept(this->m_ok = Forward<ResultOk<U>>(r) .Get()) &&
            noexcept(Construct(ResultOkTag, Forward<ResultOk<U>>(r).Get())));
        // clang-format on
        if (IsOk())
        {
            this->m_ok = Forward<ResultOk<U>>(r).Get();
        }
        else
        {
            Destruct();
            Construct(ResultOkTag, Forward<ResultOk<U>>(r).Get());
        }
        return *this;
    }

    template <typename U = ErrType>
    constexpr Result& Assign(const ResultErr<U>& r) noexcept(
        noexcept(DeclVal<Result*>()->m_err = r.Get()) && //
        noexcept(DeclVal<Result*>()->Construct(ResultErrTag, r.Get())))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(this->m_err = r.Get()) &&
                             noexcept(Construct(ResultErrTag, r.Get())));
        if (IsErr())
        {
            this->m_err = r.Get();
        }
        else
        {
            Destruct();
            Construct(ResultErrTag, r.Get());
        }
        return *this;
    }

    template <typename U = ErrType>
    constexpr Result& Assign(ResultErr<U>&& r) noexcept(
        noexcept(
            DeclVal<Result*>()->m_err = Forward<ResultErr<U>>(r).Get()) && //
        noexcept(DeclVal<Result*>()->Construct(ResultErrTag,
                                               Forward<ResultErr<U>>(r).Get())))
    {
        // clang-format off
        RAD_S_ASSERT_NOTHROW(
            noexcept(this->m_err = Forward<ResultErr<U>>(r).Get()) &&
            noexcept(Construct(ResultErrTag, Forward<ResultErr<U>>(r).Get())));
        // clang-format on
        if (IsErr())
        {
            this->m_err = Forward<ResultErr<U>>(r).Get();
        }
        else
        {
            Destruct();
            Construct(ResultErrTag, Forward<ResultErr<U>>(r).Get());
        }
        return *this;
    }

    constexpr Result& operator=(const Result& r) noexcept(
        noexcept(DeclVal<Result*>()->Assign(r)))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(Assign(r)));
        return Assign(r);
    }

    constexpr Result& operator=(Result&& r) noexcept(
        noexcept(DeclVal<Result*>()->Assign(Forward<Result>(r))))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(Assign(Forward<Result>(r))));
        return Assign(Forward<Result>(r));
    }

    template <typename U = OkType, EnIf<!IsRelated<U, ErrType>, int> = 0>
    constexpr Result& operator=(const OkType& r) noexcept(
        noexcept(DeclVal<Result*>()->Assign(r)))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(Assign(r)));
        return Assign(r);
    }

    template <typename U = OkType,
              EnIf<!IsRef<U> && !IsRelated<U, ErrType>, int> = 0>
    constexpr Result& operator=(OkType&& r) noexcept(
        noexcept(DeclVal<Result*>()->Assign(Forward<OkType>(r))))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(Assign(Forward<OkType>(r))));
        return Assign(Forward<OkType>(r));
    }

    template <typename U = ErrType, EnIf<!IsRelated<OkType, U>, int> = 0>
    constexpr Result& operator=(const ErrType& r) noexcept(
        noexcept(DeclVal<Result*>()->Assign(r)))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(Assign(r)));
        return Assign(r);
    }

    template <typename U = ErrType,
              EnIf<!IsRef<U> && !IsRelated<OkType, U>, int> = 0>
    constexpr Result& operator=(ErrType&& r) noexcept(
        noexcept(DeclVal<Result*>()->Assign(Forward<ErrType>(r))))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(Assign(Forward<ErrType>(r))));
        return Assign(Forward<ErrType>(r));
    }

    template <typename U = OkType>
    constexpr Result& operator=(const ResultOk<U>& value) noexcept(
        noexcept(DeclVal<Result*>()->Assign(value)))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(Assign(value)));
        return Assign(value);
    }

    template <typename U = OkType>
    constexpr Result& operator=(ResultOk<U>&& value) noexcept(
        noexcept(DeclVal<Result*>()->Assign(Forward<ResultOk<U>>(value))))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(Assign(Forward<ResultOk<U>>(value))));
        return Assign(Forward<ResultOk<U>>(value));
    }

    template <typename U = ErrType>
    constexpr Result& operator=(const ResultErr<U>& value) noexcept(
        noexcept(DeclVal<Result*>()->Assign(value)))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(Assign(value)));
        return Assign(value);
    }

    template <typename U = ErrType>
    constexpr Result& operator=(ResultErr<U>&& value) noexcept(
        noexcept(DeclVal<Result*>()->Assign(Forward<ResultErr<U>>(value))))
    {
        RAD_S_ASSERT_NOTHROW(noexcept(Assign(Forward<ResultErr<U>>(value))));
        return Assign(Forward<ResultErr<U>>(value));
    }

    constexpr explicit operator bool() const noexcept
    {
        return IsOk();
    }

    constexpr bool IsOk() const noexcept
    {
        return State() == ResultState::Valid;
    }

    constexpr bool IsErr() const noexcept
    {
        return State() == ResultState::Errant;
    }

    constexpr bool IsEmpty() const noexcept
    {
        return State() == ResultState::Empty;
    }

    constexpr ResultState State() const noexcept
    {
        return this->m_state;
    }

    constexpr OkType& Ok() & noexcept
    {
        RAD_ASSERT(IsOk());
        return this->m_ok.Get();
    }

    constexpr const OkType& Ok() const& noexcept
    {
        RAD_ASSERT(IsOk());
        return this->m_ok.Get();
    }

    constexpr OkType&& Ok() && noexcept
    {
        RAD_ASSERT(IsOk());
        return Move(this->m_ok).Get();
    }

    constexpr RemoveRef<OkType>* operator->() noexcept
    {
        RAD_ASSERT(IsOk());
        return &this->m_ok.Get();
    }

    constexpr const RemoveRef<OkType>* operator->() const noexcept
    {
        RAD_ASSERT(IsOk());
        return &this->m_ok.Get();
    }

    constexpr OkType& operator*() noexcept
    {
        RAD_ASSERT(IsOk());
        return this->m_ok.Get();
    }

    constexpr const OkType& operator*() const noexcept
    {
        RAD_ASSERT(IsOk());
        return this->m_ok.Get();
    }

    constexpr ErrType& Err() & noexcept
    {
        RAD_ASSERT(IsErr());
        return this->m_err.Get();
    }

    constexpr const ErrType& Err() const& noexcept
    {
        RAD_ASSERT(IsErr());
        return this->m_err.Get();
    }

    constexpr ErrType&& Err() && noexcept
    {
        RAD_ASSERT(IsErr());
        return Move(this->m_err).Get();
    }

    template <typename U, EnIf<IsCtor<Decay<OkType>, const U&>, int> = 0>
    constexpr Decay<OkType> Or(const U& value) const
        noexcept(IsNoThrowCtor<Decay<OkType>, OkType&> &&
                 IsNoThrowCtor<Decay<OkType>, U&&>)
    {
        RAD_S_ASSERT_NOTHROW((IsNoThrowCtor<Decay<OkType>, OkType&> &&
                              IsNoThrowCtor<Decay<OkType>, U&&>));
        if (IsOk())
        {
            return Ok();
        }
        return value;
    }

    template <typename U, EnIf<IsCtor<Decay<OkType>, U&&>, int> = 0>
    constexpr Decay<OkType> Or(U&& value) const
        noexcept(IsNoThrowCtor<Decay<OkType>, OkType&> &&
                 IsNoThrowCtor<Decay<OkType>, U&&>)
    {
        RAD_S_ASSERT_NOTHROW((IsNoThrowCtor<Decay<OkType>, OkType&> &&
                              IsNoThrowCtor<Decay<OkType>, U&&>));
        if (IsOk())
        {
            return Ok();
        }
        return Forward<T>(value);
    }

    template <typename U>
    constexpr Result<U, ErrType> OnOk(U&& value) const noexcept(
        IsNoThrowCtor<Result<U, ErrType>, ResultOkTagType, U&&> &&
        IsNoThrowCtor<Result<U, ErrType>, ResultErrTagType, const ErrType&>)
    {
        RAD_S_ASSERT_NOTHROW(
            (IsNoThrowCtor<Result<U, ErrType>, ResultOkTagType, U&&> &&
             IsNoThrowCtor<Result<U, ErrType>,
                           ResultErrTagType,
                           const ErrType&>));
        if (IsErr())
        {
            return Result<U, ErrType>(ResultErrTag, Err());
        }
        return Result<U, ErrType>(ResultOkTag, Forward<U>(value));
    }

    template <typename U>
    constexpr Result<OkType, U> OnErr(U&& value) const noexcept(
        IsNoThrowCtor<Result<OkType, U>, ResultErrTagType, U&&> &&
        IsNoThrowCtor<Result<OkType, U>, ResultOkTagType, const OkType&>)
    {
        RAD_S_ASSERT_NOTHROW(
            (IsNoThrowCtor<Result<OkType, U>, ResultErrTagType, U&&> &&
             IsNoThrowCtor<Result<OkType, U>, ResultOkTagType, const OkType&>));
        if (IsOk())
        {
            return Result<OkType, U>(ResultOkTag, Ok());
        }
        return Result<OkType, U>(ResultErrTag, Forward<U>(value));
    }

private:

    using StorageType::Construct;
    using StorageType::Destruct;

    template <typename U, typename F>
    friend class Result;

    template <typename R>
    constexpr void CopyCtor(R& r) noexcept(
        // clang-format off
        noexcept(DeclVal<Result*>()->Construct(ResultOkTag, r.m_ok)) &&
        noexcept(DeclVal<Result*>()->Construct(ResultErrTag, r.m_err)) &&
        noexcept(DeclVal<Result*>()->Construct(ResultEmptyTag))
        // clang-format on
    )
    {
        // clang-format off
        RAD_S_ASSERT_NOTHROW(
                noexcept(Construct(ResultOkTag, r.m_ok)) &&
                noexcept(Construct(ResultErrTag, r.m_err)) &&
                noexcept(Construct(ResultEmptyTag)));
        // clang-format on
        if (r.IsOk())
        {
            Construct(ResultOkTag, r.m_ok);
        }
        else if (r.IsErr())
        {
            Construct(ResultErrTag, r.m_err);
        }
        else
        {
            Construct(ResultEmptyTag);
        }
    }

    template <typename R>
    constexpr void CopyCtor(const R& r) noexcept(
        // clang-format off
        noexcept(DeclVal<Result*>()->Construct(ResultOkTag, r.m_ok)) &&
        noexcept(DeclVal<Result*>()->Construct(ResultErrTag, r.m_err)) &&
        noexcept(DeclVal<Result*>()->Construct(ResultEmptyTag))
        // clang-format on
    )
    {
        // clang-format off
        RAD_S_ASSERT_NOTHROW(
                noexcept(Construct(ResultOkTag, r.m_ok)) &&
                noexcept(Construct(ResultErrTag, r.m_err)) &&
                noexcept(Construct(ResultEmptyTag)));
        // clang-format on
        if (r.IsOk())
        {
            Construct(ResultOkTag, r.m_ok);
        }
        else if (r.IsErr())
        {
            Construct(ResultErrTag, r.m_err);
        }
        else
        {
            Construct(ResultEmptyTag);
        }
    }

    template <typename R>
    constexpr void Copy(const R& r) noexcept(
        // clang-format off
        noexcept(DeclVal<Result*>()->CopyCtor(DeclVal<const R&>())) &&
        noexcept(DeclVal<Result*>()->m_ok = r.m_ok) &&
        noexcept(DeclVal<Result*>()->m_err = r.m_err)
        // clang-format on
    )
    {
        // clang-format off
        RAD_S_ASSERT_NOTHROW(
            noexcept(CopyCtor(r)) &&
            noexcept(this->m_ok = r.m_ok) &&
            noexcept(this->m_err = r.m_err));
        // clang-format on
        if (State() != r.State())
        {
            Destruct();
            CopyCtor(r);
        }
        else if (IsOk())
        {
            this->m_ok = r.m_ok;
        }
        else if (IsErr())
        {
            this->m_err = r.m_err;
        }
    }

    template <typename R>
    constexpr void MoveCtor(R&& r) noexcept(
        // clang-format off
        noexcept(DeclVal<Result*>()->Construct(ResultOkTag, Move(r).Ok())) &&
        noexcept(DeclVal<Result*>()->Construct(ResultErrTag, Move(r).Err())) &&
        noexcept(DeclVal<Result*>()->Construct(ResultEmptyTag))
        // clang-format on
    )
    {
        // clang-format off
        RAD_S_ASSERT_NOTHROW(
            noexcept(Construct(ResultOkTag, Move(r).Ok())) &&
            noexcept(Construct(ResultErrTag, Move(r).Err())) &&
            noexcept(Construct(ResultEmptyTag)));
        // clang-format on
        if (r.IsOk())
        {
            Construct(ResultOkTag, Move(r).Ok());
        }
        else if (r.IsErr())
        {
            Construct(ResultErrTag, Move(r).Err());
        }
        else
        {
            Construct(ResultEmptyTag);
        }
        r.Destruct();
    }

    template <typename R>
    constexpr void MoveAssign(R&& r) noexcept(
        // clang-format off
        noexcept(DeclVal<Result*>()->MoveCtor(Move(r))) &&
        noexcept(DeclVal<Result*>()->m_ok = Move(r).Ok()) &&
        noexcept(DeclVal<Result*>()->m_err = Move(r).Err())
        // clang-format on
    )
    {
        // clang-format off
        RAD_S_ASSERT_NOTHROW(
            noexcept(MoveCtor(Move(r))) &&
            noexcept(this->m_ok = Move(r).Ok()) &&
            noexcept(this->m_err = Move(r).Err()));
        // clang-format on
        if (State() != r.State())
        {
            Destruct();
            MoveCtor(Move(r));
            return;
        }

        if (IsOk())
        {
            this->m_ok = Move(r).Ok();
        }
        else if (IsErr())
        {
            this->m_err = Move(r).Err();
        }
        r.Destruct();
    }
};

template <typename T1, typename E1, typename T2, typename E2>
constexpr inline bool operator<(const Result<T1, E1>& Left,
                                const Result<T2, E2>& Right)
{
    if (Left.IsOk() && Right.IsOk())
    {
        return (Left.Ok() < Right.Ok());
    }
    if (Left.IsErr() && Right.IsErr())
    {
        return (Left.Err() < Right.Err());
    }
    return (Left.State() < Right.State());
}

template <typename T1, typename E1, typename T2, typename E2>
constexpr inline bool operator==(const Result<T1, E1>& Left,
                                 const Result<T2, E2>& Right)
{
    if (Left.IsOk() && Right.IsOk())
    {
        return (Left.Ok() == Right.Ok());
    }
    if (Left.IsErr() && Right.IsErr())
    {
        return (Left.Err() == Right.Err());
    }
    return (Left.State() == Right.State());
}

template <typename T1, typename E1, typename T2, typename E2>
constexpr inline bool operator!=(const Result<T1, E1>& Left,
                                 const Result<T2, E2>& Right)
{
    return !(Left == Right);
}

template <typename T, typename E, typename T2>
constexpr inline EnIf<!IsSame<T, E> && IsSame<Decay<T>, Decay<T2>>, bool>
operator==(const Result<T, E>& Left, const T2& Right)
{
    if (!Left.IsOk())
    {
        return false;
    }
    return (Left.Ok() == Right);
}

template <typename T, typename E, typename T2>
constexpr inline EnIf<!IsSame<T, E> && IsSame<Decay<T>, Decay<T2>>, bool>
operator==(const T2& Left, const Result<T, E>& Right)
{
    return (Right == Left);
}

template <typename T, typename E, typename T2>
constexpr inline EnIf<!IsSame<T, E> && IsSame<Decay<T>, Decay<T2>>, bool>
operator!=(const Result<T, E>& Left, const T2& Right)
{
    return !(Left == Right);
}

template <typename T, typename E, typename T2>
constexpr inline EnIf<!IsSame<T, E> && IsSame<Decay<T>, Decay<T2>>, bool>
operator!=(const T2& Left, const Result<T, E>& Right)
{
    return !(Left == Right);
}

template <typename T, typename E, typename E2>
constexpr inline EnIf<!IsSame<T, E> && IsSame<Decay<E>, Decay<E2>>, bool>
operator==(const Result<T, E>& Left, const E2& Right)
{
    if (!Left.IsErr())
    {
        return false;
    }
    return (Left.Err() == Right);
}

template <typename T, typename E, typename E2>
constexpr inline EnIf<!IsSame<T, E> && IsSame<Decay<E>, Decay<E2>>, bool>
operator==(const E2& Left, const Result<T, E>& Right)
{
    return (Right == Left);
}

template <typename T, typename E, typename E2>
constexpr inline EnIf<!IsSame<T, E> && IsSame<Decay<E>, Decay<E2>>, bool>
operator!=(const Result<T, E>& Left, const E2& Right)
{
    return !(Left == Right);
}

template <typename T, typename E, typename E2>
constexpr inline EnIf<!IsSame<T, E> && IsSame<Decay<E>, Decay<E2>>, bool>
operator!=(const E2& Left, const Result<T, E>& Right)
{
    return !(Left == Right);
}

template <typename T, typename E, typename T2>
constexpr inline EnIf<IsSame<Decay<T>, Decay<T2>>, bool> operator==(
    const Result<T, E>& Left, const ResultOk<T2>& Right)
{
    if (!Left.IsOk())
    {
        return false;
    }
    return (Left.Ok() == Right.Get());
}

template <typename T, typename E, typename T2>
constexpr inline EnIf<IsSame<Decay<T>, Decay<T2>>, bool> operator==(
    const ResultOk<T2>& Left, const Result<T, E>& Right)
{
    return (Right == Left);
}

template <typename T, typename E, typename T2>
constexpr inline EnIf<IsSame<Decay<T>, Decay<T2>>, bool> operator!=(
    const Result<T, E>& Left, const ResultOk<T2>& Right)
{
    return !(Left == Right);
}

template <typename T, typename E, typename T2>
constexpr inline EnIf<IsSame<Decay<T>, Decay<T2>>, bool> operator!=(
    const ResultOk<T2>& Left, const Result<T, E>& Right)
{
    return !(Left == Right);
}

template <typename T, typename E, typename E2>
constexpr inline EnIf<IsSame<Decay<E>, Decay<E2>>, bool> operator==(
    const Result<T, E>& Left, const ResultErr<E2>& Right)
{
    if (!Left.IsErr())
    {
        return false;
    }
    return (Left.Err() == Right.Get());
}

template <typename T, typename E, typename E2>
constexpr inline EnIf<IsSame<Decay<E>, Decay<E2>>, bool> operator==(
    const ResultErr<E>& Left, const Result<T, E2>& Right)
{
    return (Right == Left);
}

template <typename T, typename E, typename E2>
constexpr inline EnIf<IsSame<Decay<E>, Decay<E2>>, bool> operator!=(
    const Result<T, E>& Left, const ResultErr<E2>& Right)
{
    return !(Left == Right);
}

template <typename T, typename E, typename E2>
constexpr inline EnIf<IsSame<Decay<E>, Decay<E2>>, bool> operator!=(
    const ResultErr<E2>& Left, const Result<T, E>& Right)
{
    return !(Left == Right);
}

} // namespace rad
