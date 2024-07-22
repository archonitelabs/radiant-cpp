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

#pragma once

#define USING_STL_TYPETRAITS 0
#if USING_STL_TYPETRAITS

#include <type_traits>

namespace rad
{

using std::integral_constant;
using std::true_type;
using std::false_type;
using std::is_same;
using std::conditional;
using std::enable_if;
using std::remove_reference;
using std::remove_const;
using std::remove_volatile;
using std::remove_cv;
using std::remove_extent;
using std::remove_all_extents;
using std::add_rvalue_reference;
using std::add_pointer;
using std::add_cv;
using std::add_const;
using std::add_volatile;
using std::is_void;
using std::is_const;
using std::is_volatile;
using std::is_reference;
using std::is_lvalue_reference;
using std::is_rvalue_reference;
using std::is_pointer;
using std::is_null_pointer;
using std::is_member_pointer;
using std::is_array;
using std::is_enum;
using std::is_function;
using std::is_integral;
using std::is_floating_point;
using std::is_arithmetic;
using std::is_scalar;
using std::is_empty;
using std::is_polymorphic;
using std::is_assignable;
using std::is_constructible;
using std::is_copy_assignable;
using std::is_copy_constructible;
using std::is_default_constructible;
using std::is_destructible;
using std::is_move_assignable;
using std::is_move_constructible;
using std::is_trivially_assignable;
using std::is_trivially_constructible;
using std::is_trivially_copy_assignable;
using std::is_trivially_copy_constructible;
using std::is_trivially_default_constructible;
using std::is_trivially_destructible;
using std::is_trivially_move_assignable;
using std::is_trivially_move_constructible;
using std::is_nothrow_assignable;
using std::is_nothrow_constructible;
using std::is_nothrow_copy_assignable;
using std::is_nothrow_copy_constructible;
using std::is_nothrow_default_constructible;
using std::is_nothrow_destructible;
using std::is_nothrow_move_assignable;
using std::is_nothrow_move_constructible;
using std::is_trivial;
using std::is_signed;
using std::is_convertible;
using std::has_virtual_destructor;
using std::make_unsigned;
using std::decay;

// NOTE: The declval function can only be used in unevaluated contexts. As
// alias definitions happen in evaluated context it is not possible to define
// an alias for declval.
/// typename add_rvalue_reference<T>::type declval() noexcept;
/// @brief
/// @tparam T The type.
template <typename T>
typename add_rvalue_reference<T>::type declval() noexcept;

namespace detail
{
template <typename T>
struct DeclvalSentinel
{
    static const bool evaluated = false;
};
} // namespace detail

template <typename T>
typename add_rvalue_reference<T>::type declval() noexcept
{
    static_assert(detail::DeclvalSentinel<T>::evaluated,
                  "declval is not allowed in an evaluated context");
    return typename add_rvalue_reference<T>::type();
}
} // namespace rad

#else

#include "radiant/TotallyRad.h"
#include "radiant/detail/Meta.h"
#include "radiant/detail/IntrinsicTraits.h"

namespace rad
{

/// template <typename T, T val> integral_constant;
/// @brief The base class for c++ type traits
/// @tparam T The type.
/// @tparam T The value which must be of type T.
template <typename T, T val>
using integral_constant = meta::integral_constant<T, val>;

/// true_type;
/// @brief The type representing the boolean value true
using true_type = meta::true_type;

/// false_type;
/// @brief The type representing the boolean value false
using false_type = meta::false_type;

/// template <typename L, typename R> is_same;
/// @brief A meta function returning true if L and R are the same type
/// @tparam L The left hand type.
/// @tparam R The right hand type.
template <typename L, typename R>
using is_same = meta::is_same<L, R>;

/// template <bool Cond, typename T, typename F> conditional;
/// @brief Returns the type T if Cond is true otherwise returns the type F
/// @tparam Cond The condition.
/// @tparam T The type to return on true.
/// @tparam F They type to return on false.
template <bool Cond, typename T, typename F>
using conditional = meta::conditional<Cond, T, F>;

/// template <typename T> enable_if;
/// @brief Enables a template if bool is true
/// @tparam T a dummy type.
template <bool, typename T = void>
struct enable_if
{
};

template <typename T>
struct enable_if<true, T>
{
    using type = T;
};

namespace detail
{

/// template <typename T> is_referenceable;
/// @brief A meta function that returns true if a reference can be added to T
/// @tparam T The type.
template <typename T, typename = void>
struct is_referenceable : public false_type
{
};

template <typename T>
struct is_referenceable<T, meta::VoidT<T&>> : public true_type
{
};

} // namespace detail

/// template <typename T> remove_reference;
/// @brief A meta function that removes a reference from T if one exists
/// @tparam T The type.
template <typename T>
struct remove_reference
{
    using type = T;
};

template <typename T>
struct remove_reference<T&>
{
    using type = T;
};

template <typename T>
struct remove_reference<T&&>
{
    using type = T;
};

/// template<typename T> remove_const;
/// @brief A meta function that removes const from T if T is const
/// @tparam T The type.
template <typename T>
struct remove_const
{
    using type = T;
};

template <typename T>
struct remove_const<const T>
{
    using type = T;
};

/// template<typename T> remove_volatile;
/// @brief A meta function that removes volatile from T if T is volatile
/// @tparam T The type.
template <typename T>
struct remove_volatile
{
    using type = T;
};

///
template <typename T>
struct remove_volatile<volatile T>
{
    using type = T;
};

/// template<typename T> remove_cv;
/// @brief a meta function that removes const and/or volatile if T is const
///        and/or volatile
/// @tparam T The type.
template <typename T>
struct remove_cv
{
    using type = T;
};

template <typename T>
struct remove_cv<const T>
{
    using type = T;
};

template <typename T>
struct remove_cv<volatile T>
{
    using type = T;
};

template <typename T>
struct remove_cv<const volatile T>
{
    using type = T;
};

/// template<typename T> remove_extent;
/// @brief A meta function that removes a single extent from an array
/// @tparam T The type.
template <typename T>
struct remove_extent
{
    using type = T;
};

template <typename T>
struct remove_extent<T[]>
{
    using type = T;
};

template <typename T, size_t N>
struct remove_extent<T[N]>
{
    using type = T;
};

/// template<typename T> remove_all_extents;
/// @brief A meta function that removes all extents from an array
/// @tparam T The type.
template <typename T>
struct remove_all_extents
{
    using type = T;
};

template <typename T>
struct remove_all_extents<T[]>
{
    using type = typename remove_all_extents<T>::type;
};

template <typename T, std::size_t N>
struct remove_all_extents<T[N]>
{
    using type = typename remove_all_extents<T>::type;
};

/// template<typename T> add_rvalue_reference;
/// @brief A meta function that adds an rvalue reference to T
/// @tparam T The type.
template <typename T, bool = detail::is_referenceable<T>::value>
struct add_rvalue_reference
{
    using type = T;
};

template <typename T>
struct add_rvalue_reference<T, true>
{
    using type = T&&;
};

/// template<typename T> add_pointer;
/// @brief A meta function that turns T into a pointer to T if T* is well formed
/// @tparam T The type.
namespace detail
{
template <typename T>
meta::Identity<typename remove_reference<T>::type*> try_add_pointer(int);

template <typename T>
meta::Identity<T> try_add_pointer(...);
} // namespace detail

template <typename T>
struct add_pointer : decltype(detail::try_add_pointer<T>(0))
{
};

/// template<typename T> add_cv;
/// @brief A meta function that adds const and/or volatile to T
/// @tparam T The type.
template <typename T>
struct add_cv
{
    using type = const volatile T;
};

/// template<typename T> add_const;
/// @brief  A meta function that adds const to T
/// @tparam T The type.
template <typename T>
struct add_const
{
    using type = const T;
};

/// template<typename T> add_volatile;
/// @brief A meta function that adds volatile to T
/// @tparam T The type.
template <typename T>
struct add_volatile
{
    using type = volatile T;
};

/// typename add_rvalue_reference<T>::type declval() noexcept;
/// @brief A meta function that converts type T to a reference type
/// @tparam T The type.
template <typename T>
typename add_rvalue_reference<T>::type declval() noexcept;

namespace detail
{
template <typename T>
struct DeclvalSentinel
{
    static const bool evaluated = false;
};
} // namespace detail

template <typename T>
typename add_rvalue_reference<T>::type declval() noexcept
{
    static_assert(detail::DeclvalSentinel<T>::evaluated,
                  "declval is not allowed in an evaluated context");
    return typename add_rvalue_reference<T>::type();
}

/// template<typename T> is_void;
/// @brief A meta function that returns true if T is void
/// @tparam T The type.
template <typename T>
struct is_void : public false_type
{
};

template <>
struct is_void<void> : public true_type
{
};

template <>
struct is_void<const void> : public true_type
{
};

template <>
struct is_void<volatile void> : public true_type
{
};

template <>
struct is_void<const volatile void> : public true_type
{
};

/// template<typename T> is_const;
/// @brief A meta function that returns true if T is const
/// @tparam T The type.
template <typename T>
struct is_const : false_type
{
};

template <typename T>
struct is_const<const T> : true_type
{
};

/// template<typename T> is_volatile;
/// @brief A meta function that returns true if T is volatile
/// @tparam T The type.
template <typename T>
struct is_volatile : false_type
{
};

template <typename T>
struct is_volatile<volatile T> : true_type
{
};

/// template<typename T> is_reference;
/// @brief A mete function that returns true if T is a reference type
/// @tparam T The type.
template <typename T>
struct is_reference : false_type
{
};

template <typename T>
struct is_reference<T&> : true_type
{
};

template <typename T>
struct is_reference<T&&> : true_type
{
};

/// template<typename T> is_lvalue_reference;
/// @brief A meta function that returns true if T is an lvalue reference
/// @tparam T The type.
template <typename T>
struct is_lvalue_reference : false_type
{
};

template <typename T>
struct is_lvalue_reference<T&> : true_type
{
};

/// template<typename T> is_rvalue_reference;
/// @brief A meta function that returns true if T is an rvalue reference
/// @tparam T The type.
template <typename T>
struct is_rvalue_reference : false_type
{
};

template <typename T>
struct is_rvalue_reference<T&&> : true_type
{
};

/// template<typename T> is_pointer;
/// @brief A meta function that returns true if T is a pointer
/// @tparam T The type.
namespace detail
{
template <typename T>
struct is_pointer : public false_type
{
};

template <typename T>
struct is_pointer<T*> : public true_type
{
};
} // namespace detail

template <typename T>
struct is_pointer : public detail::is_pointer<typename remove_cv<T>::type>
{
};

/// template<typename T> is_null_pointer;
/// @brief A meta function that returns true if T is nullptr
/// @tparam T The type.
namespace detail
{
template <typename T>
struct is_null_pointer : public false_type
{
};

template <>
struct is_null_pointer<nullptr_t> : public true_type
{
};
} // namespace detail

template <typename T>
struct is_null_pointer
    : public detail::is_null_pointer<typename remove_cv<T>::type>
{
};

/// template<typename T> is_member_pointer;
/// @brief A meta function that returns true if T is a pointer to a class member
/// @tparam T The type.
namespace detail
{
template <typename T>
struct is_member_pointer : public false_type
{
};

template <typename T, typename C>
struct is_member_pointer<T C::*> : public true_type
{
};
} // namespace detail

template <typename T>
struct is_member_pointer
    : public detail::is_member_pointer<typename remove_cv<T>::type>
{
};

/// template<typename T> is_array;
/// @brief A meta function that returns true if T is an array
/// @tparam T The type.
template <typename T>
struct is_array : false_type
{
};

template <typename T>
struct is_array<T[]> : true_type
{
};

template <typename T, size_t N>
struct is_array<T[N]> : true_type
{
};

/// template<typename T> is_enum;
/// @brief A meta function that returns true if T is an enum
/// @tparam T The type.
template <typename T>
struct is_enum : public integral_constant<bool, INTRINSIC_IS_ENUM(T)>
{
};

/// template<typename T> is_function;
/// @brief A meta function that returns true if T is a function
/// @tparam T The type.

// So this is a clever trick someone invented and all compiler vendors
// implement is_function this way.  Basically, you can const any type
// in C++ other than a function and a reference.  So, if you have
// some type and you try to add const to it and and get back something
// that isn't const then T must be a function or a reference. So, if
// it is not a reference then it has to be a function.
// Unfortunately, MSVC likes to warn you if you try to const something
// that can't be const so we have to disable this warning here.
#ifdef RAD_MSC_VERSION
#pragma warning(push)
#pragma warning(disable : 4180)
#endif
template <typename T>
struct is_function : integral_constant<bool,
                                       (!is_const<const T>::value) &&
                                           (!is_reference<T>::value)>
{
};
#ifdef RAD_MSC_VERSION
#pragma warning(pop)
#endif

/// template<typename T> is_integral;
/// @brief A meta function that returns true if T is an integral type
/// @tparam T The type
namespace detail
{
#if RAD_CPP20
using IntegralTypes = meta::TypeList<bool,
                                     char,
                                     signed char,
                                     unsigned char,
                                     char8_t,
                                     char16_t,
                                     char32_t,
                                     wchar_t,
                                     short,
                                     unsigned short,
                                     int,
                                     unsigned int,
                                     long,
                                     unsigned long,
                                     long,
                                     unsigned long long,
                                     long long>;
#else
using IntegralTypes = meta::TypeList<bool,
                                     char,
                                     signed char,
                                     unsigned char,
                                     char16_t,
                                     char32_t,
                                     wchar_t,
                                     short,
                                     unsigned short,
                                     int,
                                     unsigned int,
                                     long,
                                     unsigned long,
                                     long,
                                     unsigned long long,
                                     long long>;
#endif
} // namespace detail

template <typename T, typename U = typename remove_cv<T>::type>
using is_integral = typename meta::Contains<U, detail::IntegralTypes>;

/// template<typename T> is_floating_point;
/// @brief A meta function that returns true if T is a floating point type
/// @tparam T The type
namespace detail
{
using FloatingPointTypes = meta::TypeList<float, double, long double>;

#ifdef RAD_CPP23
using FloatingPointTypes = meta::PushFront<FloatingPointTypes, std::float16_t>;
using FloatingPointTypes = meta::PushFront<FloatingPointTypes, std::float32_t>;
using FloatingPointTypes = meta::PushFront<FloatingPointTypes, std::float64_t>;
using FloatingPointTypes = meta::PushFront<FloatingPointTypes, std::float128_t>;
using FloatingPointTypes = meta::PushFront<FloatingPointTypes, std::bfloat16_t>;
#endif
} // namespace detail

template <typename T, typename U = typename remove_cv<T>::type>
using is_floating_point =
    typename meta::Contains<U, detail::FloatingPointTypes>;

/// template<typename T> is_arithmetic;
/// @brief A meta function that returns true if T is an integral or floating
///        point type
/// @tparam T The type.
template <typename T>
struct is_arithmetic : public integral_constant<bool,
                                                is_integral<T>::value ||
                                                    is_floating_point<T>::value>
{
};

/// template<typename T> is_scalar;
/// @brief A meta function that returns true if T is a scalar type
/// @tparam T The type.
template <typename T>
struct is_scalar : public meta::Or<is_arithmetic<T>,
                                   is_enum<T>,
                                   is_pointer<T>,
                                   is_member_pointer<T>,
                                   is_null_pointer<T>>::type
{
};

/// template<typename T> is_empty;
/// @brief A meta function that returns true if T is a class type with no non-
///        static data members of size > 0
/// @tparam T The type.
template <typename T>
struct is_empty : public integral_constant<bool, INTRINSIC_IS_EMPTY(T)>
{
};

/// template<typename T> is_polymorphic;
/// @brief A meta function that returns true if T is a class that declares or
///        inherits at least one virtual function
/// @tparam T The type.
template <typename T>
struct is_polymorphic
    : public integral_constant<bool, INTRINSIC_IS_POLYMORPHIC(T)>
{
};

/// template<typename T, typename U> is_assignable;
/// @brief A meta function that returns true if T is assignable from U
/// @tparam T The type we are assigning to.
/// @tparam U The type we are assigning from.
template <typename T, typename U>
struct is_assignable
    : public integral_constant<bool, INTRINSIC_IS_ASSIGNABLE(T, U)>
{
};

/// template<typename T, typename... Args> is_constructible;
/// @brief A meta function that returns true if T has a constructor that can
///        be called with the parameter pack Args
/// @tparam T The type.
/// @tparam Args The Arguments to pass to the constructor
template <typename T, typename... Args>
struct is_constructible
    : public integral_constant<bool, INTRINSIC_IS_CONSTRUCTIBLE(T, Args...)>
{
};

/// template<typename T> is_copy_assignable;
/// @brief A meta function that returns true if T has a copy assignment operator
/// @tparam T The type.
template <typename T, bool = detail::is_referenceable<T>::value>
struct is_copy_assignable;

template <typename T>
struct is_copy_assignable<T, false> : public false_type
{
};

template <typename T>
struct is_copy_assignable<T, true>
    : public integral_constant<bool, INTRINSIC_IS_ASSIGNABLE(T&, const T&)>
{
};

/// template<typename T> is_copy_constructible;
/// @brief A meta function that returns true if T has a copy constructor
/// @tparam T The type.
template <typename T, bool = detail::is_referenceable<T>::value>
struct is_copy_constructible;

template <typename T>
struct is_copy_constructible<T, false> : public false_type
{
};

template <typename T>
struct is_copy_constructible<T, true>
    : public integral_constant<bool, INTRINSIC_IS_CONSTRUCTIBLE(T, const T&)>
{
};

/// template<typename T> is_default_constructible;
/// @brief A meta function that returns true if T has a copy constructor
/// @tparam T The type.
template <typename T>
struct is_default_constructible
    : public integral_constant<bool, INTRINSIC_IS_CONSTRUCTIBLE(T)>
{
};

/// template<typename T> is_destructible;
/// @brief A meta function that returns true if T is destructible
/// @tparam T The type.
template <typename T,
          bool IsTrue = is_reference<T>::value || is_scalar<T>::value,
          bool IsFalse = is_void<T>::value || is_function<T>::value>
struct is_destructible;

// true if T is a scalar or reference type.
template <typename T>
struct is_destructible<T, true, false> : public true_type
{
};

// false if T is (possibly cv-qualified) void or a function type
template <typename T>
struct is_destructible<T, false, true> : public false_type
{
};

// false if T is an array of unknown bound
template <typename T>
struct is_destructible<T[], false, false> : public false_type
{
};

namespace detail
{
template <typename T>
class destructor_well_formed
{
    template <typename U, typename = decltype(declval<U&>().~U())>
    static true_type CanDestruct(long);

    template <typename>
    static false_type CanDestruct(...);

public:

    using Type = decltype(CanDestruct<T>(0));
};
} // namespace detail

// true if with the extents removed the expression declval<U&>().~U() is
// well-formed in unevaluated context
template <typename T, size_t N>
struct is_destructible<T[N], false, false>
    : public detail::destructor_well_formed<
          typename remove_all_extents<T>::type>::Type
{
};

// true if the expression declval<U&>().~U() is well-formed in unevaluated
// context
template <typename T>
struct is_destructible<T, false, false>
    : public detail::destructor_well_formed<T>::Type
{
};

/// template<typename T> is_move_assignable;
/// @brief A meta function that returns true if T has a move assignment operator
/// @tparam T The type.
template <typename T, bool = detail::is_referenceable<T>::value>
struct is_move_assignable
    : public integral_constant<bool, INTRINSIC_IS_ASSIGNABLE(T&, T&&)>
{
};

template <typename T>
struct is_move_assignable<T, false> : public false_type
{
};

/// template<typename T> is_move_constructible;
/// @brief A meta function that returns true if T has a move constructor
/// @tparam T The type.
template <typename T, bool = detail::is_referenceable<T>::value>
struct is_move_constructible : public is_constructible<T, T&&>
{
};

template <typename T>
struct is_move_constructible<T, false> : public false_type
{
};

/// template<typename T> is_trivially_assignable;
/// @brief A meta function that returns true if T has a trivial assignment
///        operator from U
/// @tparam T The type we are assigning to.
/// @tparam U The type we are assigning from.
template <typename T, typename U>
struct is_trivially_assignable
    : public integral_constant<bool, INTRINSIC_IS_TRIVIALLY_ASSIGNABLE(T, U)>
{
};

/// template<typename T, typename... Args> is_trivially_constructible;
/// @brief A meta function that returns true if T has a trivial constructor that
///        can take the parameter pack Args as its arguments
/// @tparam T The type.
/// @tparam Args The Arguments to pass to the constructor
template <typename T, typename... Args>
struct is_trivially_constructible
    : public integral_constant<bool,
                               INTRINSIC_IS_TRIVIALLY_CONSTRUCTIBLE(T, Args...)>
{
};

/// template<typename T> is_trivially_copy_assignable;
/// @brief A meta function that returns true if T has a trivial copy assignment
///        operator
/// @tparam T The type.
template <typename T, bool = detail::is_referenceable<T>::value>
struct is_trivially_copy_assignable
    : public integral_constant<bool,
                               INTRINSIC_IS_TRIVIALLY_ASSIGNABLE(T&, const T&)>
{
};

template <typename T>
struct is_trivially_copy_assignable<T, false> : public false_type
{
};

/// template<typename T> is_trivially_copy_constructible;
/// @brief A meta function that returns true if T has a trivial copy constructor
/// @tparam T The type.
template <typename T,
          bool = detail::is_referenceable<T>::value &&
                 is_copy_constructible<T>::value>
struct is_trivially_copy_constructible
    : public integral_constant<bool,
                               INTRINSIC_IS_TRIVIALLY_CONSTRUCTIBLE(T,
                                                                    const T&)>
{
};

template <typename T>
struct is_trivially_copy_constructible<T, false> : public false_type
{
};

/// template<typename T> is_trivially_default_constructible;
/// @brief A meta function that returns true if T has a trivial default
///        constructor
/// @tparam T The type.
template <typename T>
struct is_trivially_default_constructible
    : public integral_constant<bool, INTRINSIC_IS_TRIVIALLY_CONSTRUCTIBLE(T)>
{
};

/// template<typename T> is_trivially_destructible;
/// @brief A meta function that returns true if T has a trivial destructor
/// @tparam T The type.
template <typename T, bool = is_destructible<T>::value>
struct is_trivially_destructible
    : public integral_constant<bool, INTRINSIC_IS_TRIVIALLY_DESTRUCTIBLE(T)>
{
};

template <typename T>
struct is_trivially_destructible<T, false> : public false_type
{
};

/// template<typename T> is_trivially_move_assignable;
/// @brief A meta function that returns true if T has a trivial move assignment
///        operator
/// @tparam T The type.
template <typename T, bool = detail::is_referenceable<T>::value>
struct is_trivially_move_assignable
    : public integral_constant<bool, INTRINSIC_IS_TRIVIALLY_ASSIGNABLE(T&, T&&)>
{
};

template <typename T>
struct is_trivially_move_assignable<T, false> : public false_type
{
};

/// template<typename T> is_trivially_move_constructible;
/// @brief A meta function that returns true if T has a trivial move constructor
/// @tparam T The type.
template <typename T,
          bool = detail::is_referenceable<T>::value &&
                 is_move_constructible<T>::value>
struct is_trivially_move_constructible
    : public integral_constant<bool,
                               INTRINSIC_IS_TRIVIALLY_CONSTRUCTIBLE(T, T&&)>
{
};

template <typename T>
struct is_trivially_move_constructible<T, false> : public false_type
{
};

/// template<typename T, typename U> is_nothrow_assignable;
/// @brief A meta function that returns true if T has a noexcept assignment
///        operator that can take type U as input
/// @tparam T The type we are assigning to.
/// @tparam U The type we are assigning from.
template <typename T, typename U>
struct is_nothrow_assignable
    : public integral_constant<bool, INTRINSIC_IS_NOTHROW_ASSIGNABLE(T, U)>
{
};

/// template<typename T, typename... Args> is_nothrow_constructible;
/// @brief A meta function that returns true if T has a noexcept constructor
///        that can take the expanded parameter pack Args as input
/// @tparam T The type.
/// @tparam Args The Arguments to pass to the constructor
template <typename T, typename... Args>
struct is_nothrow_constructible
    : public integral_constant<bool,
                               INTRINSIC_IS_NOTHROW_CONSTRUCTIBLE(T, Args...)>
{
};

/// template<typename T> is_nothrow_copy_assignable;
/// @brief A meta function that returns true if T has a noexcept copy assignment
///        operator
/// @tparam T The type.
template <typename T, bool = detail::is_referenceable<T>::value>
struct is_nothrow_copy_assignable : public is_nothrow_assignable<T&, const T&>
{
};

template <typename T>
struct is_nothrow_copy_assignable<T, false> : public false_type
{
};

/// template<typename T> is_nothrow_copy_constructible;
/// @brief A meta function that returns true if T has a noexcept copy
///        constructor
/// @tparam T The type.
template <typename T, bool = detail::is_referenceable<T>::value>
struct is_nothrow_copy_constructible
    : public is_nothrow_constructible<T, const T&>
{
};

template <typename T>
struct is_nothrow_copy_constructible<T, false> : public false_type
{
};

/// template<typename T> is_nothrow_default_constructible;
/// @brief A meta function that returns true if T has a noexcept default
///        constructor
/// @tparam T The type.
template <typename T>
struct is_nothrow_default_constructible
    : public integral_constant<bool, INTRINSIC_IS_NOTHROW_CONSTRUCTIBLE(T)>
{
};

/// template<typename T> is_nothrow_destructible;
/// @brief A meta function that returns true if T has a noexcept destructor
/// @tparam T The type.
template <typename T,
          bool IsTrue = is_reference<T>::value || is_scalar<T>::value,
          bool IsFalse = is_void<T>::value || is_function<T>::value>
struct is_nothrow_destructible;

namespace detail
{
template <typename T>
class noexcept_destructor_well_formed
{
    template <typename U>
    static integral_constant<bool, noexcept(declval<U&>().~U())> CanDestruct(
        long);

    template <typename>
    static false_type CanDestruct(...);

public:

    using Type = decltype(CanDestruct<T>(0));
};
} // namespace detail

// true if T is a scalar or reference type.
template <typename T>
struct is_nothrow_destructible<T, true, false> : public true_type
{
};

// false if T is (possibly cv-qualified) void or a function type
template <typename T>
struct is_nothrow_destructible<T, false, true> : public false_type
{
};

// false if T is an array of unknown bound
template <typename T>
struct is_nothrow_destructible<T[], false, false> : public false_type
{
};

// true if with the extents removed the expression declval<U&>().~U() is
// well-formed in unevaluated context
template <typename T, size_t N>
struct is_nothrow_destructible<T[N], false, false>
    : public detail::noexcept_destructor_well_formed<
          typename remove_all_extents<T>::type>::Type
{
};

// true if the expression declval<U&>().~U() is well-formed in unevaluated
// context
template <typename T>
struct is_nothrow_destructible<T, false, false>
    : public detail::noexcept_destructor_well_formed<T>::Type
{
};

/// template<typename T> is_nothrow_move_assignable;
/// @brief A meta function that returns true if T has a noexcept move assignment
///        operator
/// @tparam T The type.
template <typename T, bool = detail::is_referenceable<T>::value>
struct is_nothrow_move_assignable : public is_nothrow_assignable<T&, T&&>
{
};

template <typename T>
struct is_nothrow_move_assignable<T, false> : public false_type
{
};

/// template<typename T> is_nothrow_move_constructible;
/// @brief A meta function that returns true if T has a noexcept move
///        constructor
/// @tparam T The type.
template <typename T, bool = detail::is_referenceable<T>::value>
struct is_nothrow_move_constructible : public is_nothrow_constructible<T, T&&>
{
};

template <typename T>
struct is_nothrow_move_constructible<T, false> : public false_type
{
};

/// template<typename T> is_trivial;
/// @brief A meta function that returns true if T is a trivial type
/// @tparam T The type.
template <typename T>
struct is_trivial : public integral_constant<bool, IS_TRIVIAL_IMPL(T)>
{
};

/// template<typename T> is_signed;
/// @brief A meta function that returns true if T is a signed type
/// @tparam T The type.
template <typename T, bool = is_arithmetic<T>::value>
struct is_signed : integral_constant<bool, T(-1) < T(0)>
{
};

template <typename T>
struct is_signed<T, false> : false_type
{
};

/// template<typename From, typename To> is_convertible;
/// @brief A meta function that returns true if From can be implicitly converted
///        to To
/// @tparam From The input type
/// @tparam To The output type
template <typename From, typename To>
class is_convertible
{

    using NoCheckConversion =
        typename meta::Or<typename is_void<From>::type,
                          typename is_function<To>::type,
                          typename is_array<To>::type>::type;

    using IsVoid =
        typename meta::And<typename is_void<To>::type, NoCheckConversion>::type;

    template <typename Out>
    static void TestDecl(Out) noexcept;

    // This template is selected if a call to a function taking a type Out and
    // accepting as input the return of a function returning a reference to type
    // In is well-formed. This will only be well-formed if an implicit
    // conversion from type In to type Out exists.
    template <typename In,
              typename Out,
              typename = decltype(TestDecl<Out>(declval<In>()))>
    static true_type Convert(long);

    template <typename, typename>
    static false_type Convert(...);

public:

    // Here we first prune out some stuff we do not want to run through the
    // implicit conversion test (the only thing that converts in this case is
    // void to void), then we check for an implicit conversion.
    using type = typename conditional<NoCheckConversion::value,
                                      IsVoid,
                                      decltype(Convert<From, To>(0))>::type;
    static constexpr bool value = type::value;
};

/// template<typename T> has_virtual_destructor;
/// @brief Does the type have a virtual destructor
/// @tparam T The type.
template <typename T>
struct has_virtual_destructor
    : public integral_constant<bool, INTRINSIC_HAS_VIRTUAL_DESTRUCTOR(T)>
{
};

/// template<typename T> make_unsigned;
/// @brief Converts type T to the unsigned version of it if such a type exists
/// @tparam T The type.
template <typename T>
struct make_unsigned;

namespace detail
{

template <typename T>
struct unsign;

// Prevent bool conversions
template <>
struct unsign<bool>;

template <>
struct unsign<char>
{
    using type = unsigned char;
};

template <>
struct unsign<signed char>
{
    using type = unsigned char;
};

template <>
struct unsign<unsigned char>
{
    using type = unsigned char;
};

template <>
struct unsign<signed short>
{
    using type = unsigned short;
};

template <>
struct unsign<unsigned short>
{
    using type = unsigned short;
};

template <>
struct unsign<signed int>
{
    using type = unsigned int;
};

template <>
struct unsign<unsigned int>
{
    using type = unsigned int;
};

template <>
struct unsign<signed long>
{
    using type = unsigned long;
};

template <>
struct unsign<unsigned long>
{
    using type = unsigned long;
};

template <>
struct unsign<signed long long>
{
    using type = unsigned long long;
};

template <>
struct unsign<unsigned long long>
{
    using type = unsigned long long;
};

template <typename T>
using ConvertToUnsigned = typename conditional<
    sizeof(T) == 8,
    uint64_t,
    typename conditional<
        sizeof(T) == 4,
        uint32_t,
        typename conditional<
            sizeof(T) == 2,
            uint16_t,
            typename conditional<sizeof(T) == 1, uint8_t, void>::type>::type>::
        type>::type;

#if RAD_CPP20
using NonSignableChar = meta::TypeList<wchar_t, char8_t, char16_t, char32_t>;
#else
using NonSignableChar = meta::TypeList<wchar_t, char16_t, char32_t>;
#endif

template <typename T>
using IsNonSignableChar = typename meta::Contains<T, NonSignableChar>;

template <typename T,
          typename WithoutCV = typename remove_cv<T>::type,
          bool IsConst = is_const<T>::value,
          bool IsVolatile = is_volatile<T>::value,
          bool NeedsConversion =
              IsNonSignableChar<WithoutCV>::value || is_enum<WithoutCV>::value>
class cv_holder
{
    using converted_type = typename conditional<NeedsConversion,
                                                ConvertToUnsigned<WithoutCV>,
                                                WithoutCV>::type;
    using unsigned_type = typename unsign<converted_type>::type;
    using with_volatile =
        typename conditional<IsVolatile,
                             typename add_volatile<unsigned_type>::type,
                             unsigned_type>::type;

public:

    using type = typename conditional<IsConst,
                                      typename add_const<with_volatile>::type,
                                      with_volatile>::type;
};
} // namespace detail

template <typename T>
struct make_unsigned : detail::cv_holder<T>
{
};

/// template<typename T> decay;
/// @brief Returns the type you would get if you passed T to a function by
///        value (i.e. arrays become pointers, functions become function
///        pointers...).
/// @tparam T The type.
template <typename T>
struct decay
{
    using TNoRef = typename remove_reference<T>::type;
    using type = typename conditional<
        is_array<TNoRef>::value,
        typename add_pointer<typename remove_extent<TNoRef>::type>::type,
        typename conditional<is_function<TNoRef>::value,
                             typename add_pointer<TNoRef>::type,
                             typename remove_cv<TNoRef>::type>::type>::type;
};
} // namespace rad

#endif
