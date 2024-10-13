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

#include "radiant/detail/Meta.h"
#include "radiant/detail/StdTypeTraits.h"

namespace rad
{

// Clang warns about this, but these are test data structures they are supposed
// to have unused private fields.
#ifdef RAD_CLANG_VERSION
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"
#endif
enum class Enum8 : uint8_t
{
};
enum class Enum16 : uint16_t
{
};
enum class Enum32 : uint32_t
{
};
enum class Enum64 : uint64_t
{
};

enum OldEnum
{
};

union Union
{
    int val1;
    long val2;
};

union EmptyUnion
{
};

class ClassWithEmptyUnion
{
    EmptyUnion e;
};

class Trivial
{
    int val;
};

struct NonTrivial
{
    NonTrivial()
    {
    }

    NonTrivial(int)
    {
    }

    NonTrivial(const NonTrivial&)
    {
    }

    NonTrivial(NonTrivial&&)
    {
    }

    NonTrivial& operator=(const NonTrivial&)
    {
        return *this;
    }

    NonTrivial& operator=(const NonTrivial&&)
    {
        return *this;
    }

    ~NonTrivial()
    {
    }
};

class Base
{
};

class Derived : public Base
{
};

class VirtualBase
{
    virtual ~VirtualBase()
    {
    }
};

class PureVirtualBase
{
    virtual int foo() = 0;
};

class VirtualDerivedEmpty : virtual Base
{
};

class DerivedPureVirtual : public PureVirtualBase
{
    int foo() override
    {
        return 1;
    }
};

class Convertable
{
};

class Convert
{
public:

    operator Convertable() const
    {
        return c;
    }

    Convertable c;
};

class AlwaysConvertable
{
public:

    template <class T>
    AlwaysConvertable(T&&)
    {
    }
};

int PlainFunction();

struct MemberFunctionTester
{
    void operator()()
    {
    }

    static int Static();
    int Member() const&;
};

struct NoThrow
{
    NoThrow() noexcept
    {
    }

    NoThrow(int) noexcept
    {
    }

    NoThrow(const NoThrow&) noexcept
    {
    }

    NoThrow(NoThrow&&) noexcept
    {
    }

    NoThrow& operator=(const NoThrow&) noexcept
    {
        return *this;
    }

    NoThrow& operator=(const NoThrow&&) noexcept
    {
        return *this;
    }

    ~NoThrow() noexcept
    {
    }
};

struct Throw
{
    Throw()
    {
    }

    Throw(int)
    {
    }

    Throw(const Throw&)
    {
    }

    Throw(Throw&&)
    {
    }

    Throw& operator=(const Throw&)
    {
        return *this;
    }

    Throw& operator=(const Throw&&)
    {
        return *this;
    }

    ~Throw()
    {
    }
};

struct NoOps
{
    NoOps() noexcept = delete;
    NoOps(int) noexcept = delete;
    NoOps(const NoOps&) noexcept = delete;
    NoOps(NoOps&&) noexcept = delete;

    NoOps& operator=(const NoOps&) noexcept = delete;
    NoOps& operator=(const NoOps&&) noexcept = delete;

private:

    ~NoOps() noexcept = default;
};

#ifdef RAD_CLANG_VERSION
#pragma clang diagnostic pop
#endif

RAD_S_ASSERT(meta::Or<>::value == false);
RAD_S_ASSERT(meta::Or<false_type>::value == false);
RAD_S_ASSERT(meta::Or<true_type>::value == true);
RAD_S_ASSERT((meta::Or<true_type, true_type>::value == true));
RAD_S_ASSERT((meta::Or<true_type, false_type>::value == true));
RAD_S_ASSERT((meta::Or<false_type, true_type>::value == true));
RAD_S_ASSERT((meta::Or<false_type, false_type>::value == false));
RAD_S_ASSERT((meta::Or<false_type, false_type, false_type>::value == false));
RAD_S_ASSERT((meta::Or<false_type, true_type, false_type>::value == true));
RAD_S_ASSERT((meta::Or<false_type, false_type, false_type, true_type>::value ==
              true));

RAD_S_ASSERT(meta::And<>::value == true);
RAD_S_ASSERT(meta::And<false_type>::value == false);
RAD_S_ASSERT(meta::And<true_type>::value == true);
RAD_S_ASSERT((meta::And<true_type, true_type>::value == true));
RAD_S_ASSERT((meta::And<true_type, false_type>::value == false));
RAD_S_ASSERT((meta::And<false_type, true_type>::value == false));
RAD_S_ASSERT((meta::And<false_type, false_type>::value == false));
RAD_S_ASSERT((meta::And<true_type, true_type, true_type>::value == true));
RAD_S_ASSERT((meta::And<true_type, true_type, false_type>::value == false));
RAD_S_ASSERT((meta::And<true_type, true_type, true_type, false_type>::value ==
              false));

/// template <typename L, typename R> is_same;
#define CheckIsSame(L, R)                                                      \
    RAD_S_ASSERT((is_same<L, R>::value == std::is_same<L, R>::value))

CheckIsSame(int, int);
CheckIsSame(int, long);
CheckIsSame(Base, Base);
CheckIsSame(Base, Derived);
CheckIsSame(Derived, Base);
CheckIsSame(Base, Derived);

/// template <bool Cond, typename T, typename F> conditional;
#define CheckConditional(Cond, T, F)                                           \
    RAD_S_ASSERT((is_same<conditional<Cond, T, F>::type,                       \
                          std::conditional<Cond, T, F>::type>::value))

CheckConditional(true, true_type, false_type);
CheckConditional(false, true_type, false_type);

/// template <typename T> remove_reference;
#define CheckRemoveReference(T)                                                \
    RAD_S_ASSERT((is_same<remove_reference<T>::type,                           \
                          std::remove_reference<T>::type>::value))

CheckRemoveReference(int);
CheckRemoveReference(int&);
CheckRemoveReference(int&&);
CheckRemoveReference(const int&);
CheckRemoveReference(volatile int&);
CheckRemoveReference(const volatile int&);

/// template<typename T> remove_const;
#define CheckRemoveConst(T)                                                    \
    RAD_S_ASSERT(                                                              \
        (is_same<remove_const<T>::type, std::remove_const<T>::type>::value))

CheckRemoveConst(int);
CheckRemoveConst(const int);
CheckRemoveConst(volatile int);
CheckRemoveConst(const volatile int);

/// template<typename T> remove_volatile;
#define CheckRemoveVolatile(T)                                                 \
    RAD_S_ASSERT((is_same<remove_volatile<T>::type,                            \
                          std::remove_volatile<T>::type>::value))

CheckRemoveVolatile(int);
CheckRemoveVolatile(const int);
CheckRemoveVolatile(volatile int);
CheckRemoveVolatile(const volatile int);

/// template<typename T> remove_cv;
#define CheckRemoveCv(T)                                                       \
    RAD_S_ASSERT((is_same<remove_cv<T>::type, std::remove_cv<T>::type>::value))

CheckRemoveCv(int);
CheckRemoveCv(const int);
CheckRemoveCv(volatile int);
CheckRemoveCv(const volatile int);

/// template<typename T> remove_extent;
#define CheckRemoveExtent(T)                                                   \
    RAD_S_ASSERT(                                                              \
        (is_same<remove_extent<T>::type, std::remove_extent<T>::type>::value))

CheckRemoveExtent(int);
CheckRemoveExtent(int[2]);
CheckRemoveExtent(int[4][2]);
CheckRemoveExtent(int[][4][2]);
CheckRemoveExtent(int[]);

/// template<typename T> remove_all_extents;
#define CheckRemoveAllExtents(T)                                               \
    RAD_S_ASSERT((is_same<remove_all_extents<T>::type,                         \
                          std::remove_all_extents<T>::type>::value))

CheckRemoveAllExtents(int);
CheckRemoveAllExtents(int[2]);
CheckRemoveAllExtents(int[4][2]);
CheckRemoveAllExtents(int[][4][2]);
CheckRemoveAllExtents(int[]);

/// template<typename T> add_rvalue_reference;
#define CheckAddRvalueReference(T)                                             \
    RAD_S_ASSERT((is_same<add_rvalue_reference<T>::type,                       \
                          std::add_rvalue_reference<T>::type>::value))

CheckAddRvalueReference(int);
CheckAddRvalueReference(const volatile int);
CheckAddRvalueReference(int&);
CheckAddRvalueReference(int&&);
CheckAddRvalueReference(int*);
CheckAddRvalueReference(void*);

/// template<typename T> add_pointer;
#define CheckAddPointer(T)                                                     \
    RAD_S_ASSERT(                                                              \
        (is_same<add_pointer<T>::type, std::add_pointer<T>::type>::value))

CheckAddPointer(int);
CheckAddPointer(int*);
CheckAddPointer(int&);
CheckAddPointer(int&&);
CheckAddPointer(int[]);
CheckAddPointer(int[2]);
CheckAddPointer(void);
CheckAddPointer(void(int));
CheckAddPointer(int (MemberFunctionTester::*)() const);

/// template<typename T> add_cv;
#define CheckAddCv(T)                                                          \
    RAD_S_ASSERT((is_same<add_cv<T>::type, std::add_cv<T>::type>::value))

CheckAddCv(int);
CheckAddCv(const int);
CheckAddCv(volatile int);

/// template<typename T> add_const;
#define CheckAddConst(T)                                                       \
    RAD_S_ASSERT((is_same<add_const<T>::type, std::add_const<T>::type>::value))

CheckAddConst(int);
CheckAddConst(const int);
CheckAddConst(volatile int);

/// template<typename T> add_volatile;
#define CheckAddVolatile(T)                                                    \
    RAD_S_ASSERT(                                                              \
        (is_same<add_volatile<T>::type, std::add_volatile<T>::type>::value))

CheckAddVolatile(int);
CheckAddVolatile(const int);
CheckAddVolatile(volatile int);

/// typename add_rvalue_reference<T>::type declval() noexcept;
#define CheckDeclval(T)                                                        \
    RAD_S_ASSERT(                                                              \
        (is_same<decltype(declval<T>()), decltype(std::declval<T>())>::value))

CheckDeclval(int);
CheckDeclval(const volatile int);
CheckDeclval(int&);
CheckDeclval(int&&);
CheckDeclval(int*);
CheckDeclval(void*);

/// template<typename T> is_void;
#define CheckIsVoid(T) RAD_S_ASSERT(is_void<T>::value == std::is_void<T>::value)

CheckIsVoid(int);
CheckIsVoid(const volatile int);
CheckIsVoid(const int);
CheckIsVoid(volatile int&&);
CheckIsVoid(int*);
CheckIsVoid(void*);
CheckIsVoid(Derived());

/// template<typename T> is_const;
#define CheckIsConst(T)                                                        \
    RAD_S_ASSERT(is_const<T>::value == std::is_const<T>::value)

CheckIsConst(int);
CheckIsConst(const volatile int);
CheckIsConst(const int);
CheckIsConst(volatile int&&);
CheckIsConst(int*);
CheckIsConst(void*);
CheckIsConst(Derived());

/// template<typename T> is_volatile;
#define CheckIsVolatile(T)                                                     \
    RAD_S_ASSERT(is_volatile<T>::value == std::is_volatile<T>::value)

CheckIsVolatile(int);
CheckIsVolatile(const volatile int);
CheckIsVolatile(const int);
CheckIsVolatile(volatile int&&);
CheckIsVolatile(int*);
CheckIsVolatile(void*);
CheckIsVolatile(Derived());

/// template<typename T> is_reference;
#define CheckIsReference(T)                                                    \
    RAD_S_ASSERT(is_reference<T>::value == std::is_reference<T>::value)

CheckIsReference(int);
CheckIsReference(const volatile int);
CheckIsReference(int&);
CheckIsReference(int&&);
CheckIsReference(int*);
CheckIsReference(void*);
CheckIsReference(Derived());

/// template<typename T> is_lvalue_reference;
#define CheckIsLvalueReference(T)                                              \
    RAD_S_ASSERT(is_lvalue_reference<T>::value ==                              \
                 std::is_lvalue_reference<T>::value)

CheckIsLvalueReference(int);
CheckIsLvalueReference(const volatile int);
CheckIsLvalueReference(int&);
CheckIsLvalueReference(int&&);
CheckIsLvalueReference(int*);
CheckIsLvalueReference(void*);
CheckIsLvalueReference(Derived());

/// template<typename T> is_rvalue_reference;
#define CheckIsRvalueReference(T)                                              \
    RAD_S_ASSERT(is_rvalue_reference<T>::value ==                              \
                 std::is_rvalue_reference<T>::value)

CheckIsRvalueReference(int);
CheckIsRvalueReference(const volatile int);
CheckIsRvalueReference(int&);
CheckIsRvalueReference(int&&);
CheckIsRvalueReference(int*);
CheckIsRvalueReference(Derived());

/// template<typename T> is_pointer;
#define CheckIsPointer(T)                                                      \
    RAD_S_ASSERT(is_pointer<T>::value == std::is_pointer<T>::value)

CheckIsPointer(int(long));
CheckIsPointer(int);
CheckIsPointer(std::function<void()>);
CheckIsPointer(int*);
CheckIsPointer(int&);
CheckIsPointer(int*&);
CheckIsPointer(decltype(nullptr));
CheckIsPointer(void*);

/// template<typename T> is_null_pointer;
#define CheckIsNullPointer(T)                                                  \
    RAD_S_ASSERT(is_null_pointer<T>::value == std::is_null_pointer<T>::value)

CheckIsNullPointer(int(long));
CheckIsNullPointer(int);
CheckIsNullPointer(std::function<void()>);
CheckIsNullPointer(int*);
CheckIsNullPointer(int&);
CheckIsNullPointer(int*&);
CheckIsNullPointer(decltype(nullptr));
CheckIsNullPointer(void*);

/// template<typename T> is_member_pointer;
#define CheckIsMemberPointer(T)                                                \
    RAD_S_ASSERT(is_member_pointer<T>::value ==                                \
                 std::is_member_pointer<T>::value)

CheckIsMemberPointer(int);
CheckIsMemberPointer(int&);
CheckIsMemberPointer(int&&);
CheckIsMemberPointer(int*);
CheckIsMemberPointer(const int);
CheckIsMemberPointer(void);
CheckIsMemberPointer(const char*);
CheckIsMemberPointer(float);
CheckIsMemberPointer(double);
CheckIsMemberPointer(wchar_t);
CheckIsMemberPointer(Enum8);
CheckIsMemberPointer(Base);
CheckIsMemberPointer(Union);
CheckIsMemberPointer(int(Base::*));

/// template<typename T> is_array;
#define CheckIsArray(T)                                                        \
    RAD_S_ASSERT(is_array<T>::value == std::is_array<T>::value)

CheckIsArray(int);
CheckIsArray(int&);
CheckIsArray(int&&);
CheckIsArray(int*);
CheckIsArray(const int);
CheckIsArray(const volatile int&);
CheckIsArray(void);
CheckIsArray(const volatile void);
CheckIsArray(int(long));
CheckIsArray(long[]);
CheckIsArray(long[3]);
CheckIsArray(long[3][3][4]);
CheckIsArray(long[][3][4]);

/// template<typename T> is_enum;
#define CheckIsEnum(T) RAD_S_ASSERT(is_enum<T>::value == std::is_enum<T>::value)

CheckIsEnum(int);
CheckIsEnum(int&);
CheckIsEnum(int&&);
CheckIsEnum(int*);
CheckIsEnum(const int);
CheckIsEnum(void);
CheckIsEnum(const char*);
CheckIsEnum(float);
CheckIsEnum(double);
CheckIsEnum(wchar_t);
CheckIsEnum(Enum8);
CheckIsEnum(Base);
CheckIsEnum(Union);

/// template<typename T> is_function;
#define CheckIsFunction(T)                                                     \
    RAD_S_ASSERT(is_function<T>::value == std::is_function<T>::value)

CheckIsFunction(int(long));
CheckIsFunction(int);
CheckIsFunction(std::function<void()>);
CheckIsFunction(decltype(PlainFunction));
CheckIsFunction(MemberFunctionTester());
CheckIsFunction(MemberFunctionTester);
CheckIsFunction(decltype(MemberFunctionTester::Static));
CheckIsFunction(decltype(&MemberFunctionTester::Member));
#if RAD_CPP20
CheckIsFunction(decltype([] {}));
#endif

/// template<typename T> is_integral;
#define CheckIsIntegral(T)                                                     \
    RAD_S_ASSERT(is_integral<T>::value == std::is_integral<T>::value)

CheckIsIntegral(int);
CheckIsIntegral(int&);
CheckIsIntegral(int&&);
CheckIsIntegral(int*);
CheckIsIntegral(const int);
CheckIsIntegral(void);
CheckIsIntegral(const char*);
CheckIsIntegral(float);
CheckIsIntegral(double);
CheckIsIntegral(wchar_t);
CheckIsIntegral(Enum8);
CheckIsIntegral(Base);
CheckIsIntegral(Union);

/// template<typename T> is_floating_point;
#define CheckIsFloatingPoint(T)                                                \
    RAD_S_ASSERT(is_floating_point<T>::value ==                                \
                 std::is_floating_point<T>::value)

CheckIsFloatingPoint(int);
CheckIsFloatingPoint(int&);
CheckIsFloatingPoint(int&&);
CheckIsFloatingPoint(int*);
CheckIsFloatingPoint(const int);
CheckIsFloatingPoint(void);
CheckIsFloatingPoint(const char*);
CheckIsFloatingPoint(float);
CheckIsFloatingPoint(double);
CheckIsFloatingPoint(Enum8);
CheckIsFloatingPoint(Base);
CheckIsFloatingPoint(Union);

/// template<typename T> is_arithmetic;
#define CheckIsArithmetic(T)                                                   \
    RAD_S_ASSERT(is_arithmetic<T>::value == std::is_arithmetic<T>::value)

CheckIsArithmetic(int);
CheckIsArithmetic(int&);
CheckIsArithmetic(int&&);
CheckIsArithmetic(int*);
CheckIsArithmetic(const int);
CheckIsArithmetic(void);
CheckIsArithmetic(const char*);
CheckIsArithmetic(Enum8);
CheckIsArithmetic(Base);
CheckIsArithmetic(Union);

/// template<typename T> is_scalar;
#define CheckIsScalar(T)                                                       \
    RAD_S_ASSERT(is_scalar<T>::value == std::is_scalar<T>::value)

CheckIsScalar(int);
CheckIsScalar(int&);
CheckIsScalar(int&&);
CheckIsScalar(int*);
CheckIsScalar(const int);
CheckIsScalar(void);
CheckIsScalar(const char*);
CheckIsScalar(Enum8);
CheckIsScalar(Base);
CheckIsScalar(Union);

/// template<typename T> is_empty;
#define CheckIsEmpty(T)                                                        \
    RAD_S_ASSERT(is_empty<T>::value == std::is_empty<T>::value)

CheckIsEmpty(int);
CheckIsEmpty(int&);
CheckIsEmpty(int&&);
CheckIsEmpty(int*);
CheckIsEmpty(const int);
CheckIsEmpty(void);
CheckIsEmpty(Base);
CheckIsEmpty(Derived);
CheckIsEmpty(VirtualBase);
CheckIsEmpty(PureVirtualBase);
CheckIsEmpty(VirtualDerivedEmpty);
CheckIsEmpty(DerivedPureVirtual);
CheckIsEmpty(EmptyUnion);
CheckIsEmpty(ClassWithEmptyUnion);
CheckIsEmpty(Union);

/// template<typename T> is_polymorphic;
#define CheckIsPolymorphic(T)                                                  \
    RAD_S_ASSERT(is_polymorphic<T>::value == std::is_polymorphic<T>::value)

CheckIsPolymorphic(int);
CheckIsPolymorphic(int&);
CheckIsPolymorphic(int&&);
CheckIsPolymorphic(int*);
CheckIsPolymorphic(const int);
CheckIsPolymorphic(void);
CheckIsPolymorphic(Base);
CheckIsPolymorphic(Derived);
CheckIsPolymorphic(VirtualBase);
CheckIsPolymorphic(PureVirtualBase);
CheckIsPolymorphic(VirtualDerivedEmpty);
CheckIsPolymorphic(DerivedPureVirtual);

/// template<typename T, typename U> is_assignable;
#define CheckIsAssignable(T)                                                   \
    RAD_S_ASSERT(                                                              \
        (is_assignable<T, T>::value == std::is_assignable<T, T>::value))

CheckIsAssignable(NoThrow);
CheckIsAssignable(Throw);
CheckIsAssignable(NoOps);
CheckIsAssignable(int);
CheckIsAssignable(int&);
CheckIsAssignable(int&&);
CheckIsAssignable(int*);
CheckIsAssignable(const int);
CheckIsAssignable(void);

/// template<typename T, typename... Args> is_constructible;
#define CheckIsConstructible(T, U)                                             \
    RAD_S_ASSERT(                                                              \
        (is_constructible<T, U>::value == std::is_constructible<T, U>::value))

CheckIsConstructible(NoThrow, int);
CheckIsConstructible(Throw, int);
CheckIsConstructible(NoOps, int);
CheckIsConstructible(int, int);
CheckIsConstructible(int&, int);
CheckIsConstructible(int&&, int);
CheckIsConstructible(int*, int);
CheckIsConstructible(const int, int);
CheckIsConstructible(void, int);

/// template<typename T> is_copy_assignable;
#define CheckIsCopyAssignable(T)                                               \
    RAD_S_ASSERT(is_copy_assignable<T>::value ==                               \
                 std::is_copy_assignable<T>::value)

CheckIsCopyAssignable(NoThrow);
CheckIsCopyAssignable(Throw);
CheckIsCopyAssignable(NoOps);
CheckIsCopyAssignable(int);
CheckIsCopyAssignable(int&);
CheckIsCopyAssignable(int&&);
CheckIsCopyAssignable(int*);
CheckIsCopyAssignable(const int);
CheckIsCopyAssignable(void);

/// template<typename T> is_copy_constructible;
#define CheckIsCopyConstructible(T)                                            \
    RAD_S_ASSERT(is_copy_constructible<T>::value ==                            \
                 std::is_copy_constructible<T>::value)

CheckIsCopyConstructible(NoThrow);
CheckIsCopyConstructible(Throw);
CheckIsCopyConstructible(NoOps);
CheckIsCopyConstructible(int);
CheckIsCopyConstructible(int&);
CheckIsCopyConstructible(int&&);
CheckIsCopyConstructible(int*);
CheckIsCopyConstructible(const int);
CheckIsCopyConstructible(void);

/// template<typename T> is_default_constructible;
#define CheckIsDefaultConstructible(T)                                         \
    RAD_S_ASSERT(is_default_constructible<T>::value ==                         \
                 std::is_default_constructible<T>::value)

CheckIsDefaultConstructible(NoThrow);
CheckIsDefaultConstructible(Throw);
CheckIsDefaultConstructible(NoOps);
CheckIsDefaultConstructible(int);
CheckIsDefaultConstructible(int&);
CheckIsDefaultConstructible(int&&);
CheckIsDefaultConstructible(int*);
CheckIsDefaultConstructible(const int);
CheckIsDefaultConstructible(void);

/// template<typename T> is_destructible;
#define CheckIsDestructible(T)                                                 \
    RAD_S_ASSERT(is_destructible<T>::value == std::is_destructible<T>::value)

CheckIsDestructible(NoThrow);
CheckIsDestructible(Throw);
CheckIsDestructible(NoOps);
CheckIsDestructible(int);
CheckIsDestructible(int&);
CheckIsDestructible(int&&);
CheckIsDestructible(int*);
CheckIsDestructible(const int);
CheckIsDestructible(const volatile int&);
CheckIsDestructible(void);
CheckIsDestructible(const volatile void);
CheckIsDestructible(int(long));
CheckIsDestructible(long[]);
CheckIsDestructible(long[3]);
CheckIsDestructible(long[3][3][4]);
CheckIsDestructible(long[][3][4]);

/// template<typename T> is_move_assignable;
#define CheckIsMoveAssignable(T)                                               \
    RAD_S_ASSERT(is_move_assignable<T>::value ==                               \
                 std::is_move_assignable<T>::value)

CheckIsMoveAssignable(NoThrow);
CheckIsMoveAssignable(Throw);
CheckIsMoveAssignable(NoOps);
CheckIsMoveAssignable(int);
CheckIsMoveAssignable(int&);
CheckIsMoveAssignable(int&&);
CheckIsMoveAssignable(int*);
CheckIsMoveAssignable(const int);
CheckIsMoveAssignable(void);

/// template<typename T> is_move_constructible;
#define CheckIsMoveConstructible(T)                                            \
    RAD_S_ASSERT(is_move_constructible<T>::value ==                            \
                 std::is_move_constructible<T>::value)

CheckIsMoveConstructible(NoThrow);
CheckIsMoveConstructible(Throw);
CheckIsMoveConstructible(NoOps);
CheckIsMoveConstructible(int);
CheckIsMoveConstructible(int&);
CheckIsMoveConstructible(int&&);
CheckIsMoveConstructible(int*);
CheckIsMoveConstructible(const int);
CheckIsMoveConstructible(void);

/// template<typename T> is_trivially_assignable;
#define CheckIsTriviallyAssignable(T)                                          \
    RAD_S_ASSERT((is_trivially_assignable<T, T>::value ==                      \
                  std::is_trivially_assignable<T, T>::value))

CheckIsTriviallyAssignable(Trivial);
CheckIsTriviallyAssignable(NonTrivial);
CheckIsTriviallyAssignable(NoOps);
CheckIsTriviallyAssignable(int);
CheckIsTriviallyAssignable(int&);
CheckIsTriviallyAssignable(int&&);
CheckIsTriviallyAssignable(int*);
CheckIsTriviallyAssignable(const int);
CheckIsTriviallyAssignable(void);

/// template<typename T, typename... Args> is_trivially_constructible;
#define CheckIsTriviallyConstructible(T, U)                                    \
    RAD_S_ASSERT((is_trivially_constructible<T, U>::value ==                   \
                  std::is_trivially_constructible<T, U>::value))

CheckIsTriviallyConstructible(Trivial, int);
CheckIsTriviallyConstructible(NonTrivial, int);
CheckIsTriviallyConstructible(NoOps, int);
CheckIsTriviallyConstructible(int, int);
CheckIsTriviallyConstructible(int&, int);
CheckIsTriviallyConstructible(int&&, int);
CheckIsTriviallyConstructible(int*, int);
CheckIsTriviallyConstructible(const int, int);
CheckIsTriviallyConstructible(void, int);

/// template<typename T> is_trivially_copy_assignable;
#define CheckIsTriviallyCopyAssignable(T)                                      \
    RAD_S_ASSERT(is_trivially_copy_assignable<T>::value ==                     \
                 std::is_trivially_copy_assignable<T>::value)

CheckIsTriviallyCopyAssignable(Trivial);
CheckIsTriviallyCopyAssignable(NonTrivial);
CheckIsTriviallyCopyAssignable(NoOps);
CheckIsTriviallyCopyAssignable(int);
CheckIsTriviallyCopyAssignable(int&);
CheckIsTriviallyCopyAssignable(int&&);
CheckIsTriviallyCopyAssignable(int*);
CheckIsTriviallyCopyAssignable(const int);
CheckIsTriviallyCopyAssignable(void);

/// template<typename T> is_trivially_copy_constructible;
#define CheckIsTriviallyCopyConstructible(T)                                   \
    RAD_S_ASSERT(is_trivially_copy_constructible<T>::value ==                  \
                 std::is_trivially_copy_constructible<T>::value)

CheckIsTriviallyCopyConstructible(Trivial);
CheckIsTriviallyCopyConstructible(NonTrivial);
CheckIsTriviallyCopyConstructible(NoOps);
CheckIsTriviallyCopyConstructible(int);
CheckIsTriviallyCopyConstructible(int&);
CheckIsTriviallyCopyConstructible(int&&);
CheckIsTriviallyCopyConstructible(int*);
CheckIsTriviallyCopyConstructible(const int);
CheckIsTriviallyCopyConstructible(void);

/// template<typename T> is_trivially_default_constructible;
#define CheckIsTriviallyDefaultConstructible(T)                                \
    RAD_S_ASSERT(is_trivially_default_constructible<T>::value ==               \
                 std::is_trivially_default_constructible<T>::value)

CheckIsTriviallyDefaultConstructible(Trivial);
CheckIsTriviallyDefaultConstructible(NonTrivial);
CheckIsTriviallyDefaultConstructible(NoOps);
CheckIsTriviallyDefaultConstructible(int);
CheckIsTriviallyDefaultConstructible(int&);
CheckIsTriviallyDefaultConstructible(int&&);
CheckIsTriviallyDefaultConstructible(int*);
CheckIsTriviallyDefaultConstructible(const int);
CheckIsTriviallyDefaultConstructible(void);

/// template<typename T> is_trivially_destructible;
#define CheckIsTriviallyDestructible(T)                                        \
    RAD_S_ASSERT(is_trivially_destructible<T>::value ==                        \
                 std::is_trivially_destructible<T>::value)

CheckIsTriviallyDestructible(Trivial);
CheckIsTriviallyDestructible(NonTrivial);
CheckIsTriviallyDestructible(NoOps);
CheckIsTriviallyDestructible(int);
CheckIsTriviallyDestructible(int&);
CheckIsTriviallyDestructible(int&&);
CheckIsTriviallyDestructible(int*);
CheckIsTriviallyDestructible(const int);
CheckIsTriviallyDestructible(void);

/// template<typename T> is_trivially_move_assignable;
#define CheckIsTriviallyMoveAssignable(T)                                      \
    RAD_S_ASSERT(is_trivially_move_assignable<T>::value ==                     \
                 std::is_trivially_move_assignable<T>::value)

CheckIsTriviallyMoveAssignable(Trivial);
CheckIsTriviallyMoveAssignable(NonTrivial);
CheckIsTriviallyMoveAssignable(NoOps);
CheckIsTriviallyMoveAssignable(int);
CheckIsTriviallyMoveAssignable(int&);
CheckIsTriviallyMoveAssignable(int&&);
CheckIsTriviallyMoveAssignable(int*);
CheckIsTriviallyMoveAssignable(const int);
CheckIsTriviallyMoveAssignable(void);

/// template<typename T> is_trivially_move_constructible;
#define CheckIsTriviallyMoveConstructible(T)                                   \
    RAD_S_ASSERT(is_trivially_move_constructible<T>::value ==                  \
                 std::is_trivially_move_constructible<T>::value)

CheckIsTriviallyMoveConstructible(Trivial);
CheckIsTriviallyMoveConstructible(NonTrivial);
CheckIsTriviallyMoveConstructible(NoOps);
CheckIsTriviallyMoveConstructible(int);
CheckIsTriviallyMoveConstructible(int&);
CheckIsTriviallyMoveConstructible(int&&);
CheckIsTriviallyMoveConstructible(int*);
CheckIsTriviallyMoveConstructible(const int);
CheckIsTriviallyMoveConstructible(void);

/// template<typename T, typename U> is_nothrow_assignable;
#define CheckIsNoThrowAssignable(T)                                            \
    RAD_S_ASSERT((is_nothrow_assignable<T, T>::value ==                        \
                  std::is_nothrow_assignable<T, T>::value))

CheckIsNoThrowAssignable(NoThrow);
CheckIsNoThrowAssignable(Throw);
CheckIsNoThrowAssignable(NoOps);
CheckIsNoThrowAssignable(int);
CheckIsNoThrowAssignable(int&);
CheckIsNoThrowAssignable(int&&);
CheckIsNoThrowAssignable(int*);
CheckIsNoThrowAssignable(const int);
CheckIsNoThrowAssignable(void);

/// template<typename T, typename... Args> is_nothrow_constructible;
#define CheckIsNoThrowConstructible(T, U)                                      \
    RAD_S_ASSERT((is_nothrow_constructible<T, U>::value ==                     \
                  std::is_nothrow_constructible<T, U>::value))

CheckIsNoThrowConstructible(NoThrow, int);
CheckIsNoThrowConstructible(Throw, int);
CheckIsNoThrowConstructible(NoOps, int);
CheckIsNoThrowConstructible(int, int);
CheckIsNoThrowConstructible(int&, int);
CheckIsNoThrowConstructible(int&&, int);
CheckIsNoThrowConstructible(int*, int);
CheckIsNoThrowConstructible(const int, int);
CheckIsNoThrowConstructible(void, int);

/// template<typename T> is_nothrow_copy_assignable;
#define CheckIsNoThrowCopyAssignable(T)                                        \
    RAD_S_ASSERT(is_nothrow_copy_assignable<T>::value ==                       \
                 std::is_nothrow_copy_assignable<T>::value)

CheckIsNoThrowCopyAssignable(NoThrow);
CheckIsNoThrowCopyAssignable(Throw);
CheckIsNoThrowCopyAssignable(NoOps);
CheckIsNoThrowCopyAssignable(int);
CheckIsNoThrowCopyAssignable(int&);
CheckIsNoThrowCopyAssignable(int&&);
CheckIsNoThrowCopyAssignable(int*);
CheckIsNoThrowCopyAssignable(const int);
CheckIsNoThrowCopyAssignable(void);

/// template<typename T> is_nothrow_copy_constructible;
#define CheckIsNoThrowCopyConstructible(T)                                     \
    RAD_S_ASSERT(is_nothrow_copy_constructible<T>::value ==                    \
                 std::is_nothrow_copy_constructible<T>::value)

CheckIsNoThrowCopyConstructible(NoThrow);
CheckIsNoThrowCopyConstructible(Throw);
CheckIsNoThrowCopyConstructible(NoOps);
CheckIsNoThrowCopyConstructible(int);
CheckIsNoThrowCopyConstructible(int&);
CheckIsNoThrowCopyConstructible(int&&);
CheckIsNoThrowCopyConstructible(int*);
CheckIsNoThrowCopyConstructible(const int);
CheckIsNoThrowCopyConstructible(void);

/// template<typename T> is_nothrow_default_constructible;
#define CheckIsNoThrowDefaultConstructible(T)                                  \
    RAD_S_ASSERT(is_nothrow_default_constructible<T>::value ==                 \
                 std::is_nothrow_default_constructible<T>::value)

CheckIsNoThrowDefaultConstructible(NoThrow);
CheckIsNoThrowDefaultConstructible(Throw);
CheckIsNoThrowDefaultConstructible(NoOps);
CheckIsNoThrowDefaultConstructible(int);
CheckIsNoThrowDefaultConstructible(int&);
CheckIsNoThrowDefaultConstructible(int&&);
CheckIsNoThrowDefaultConstructible(int*);
CheckIsNoThrowDefaultConstructible(const int);
CheckIsNoThrowDefaultConstructible(void);

/// template<typename T> is_nothrow_destructible;
#define CheckIsNoThrowDestructible(T)                                          \
    RAD_S_ASSERT(is_nothrow_destructible<T>::value ==                          \
                 std::is_nothrow_destructible<T>::value)

CheckIsNoThrowDestructible(NoThrow);
CheckIsNoThrowDestructible(Throw);
CheckIsNoThrowDestructible(NoOps);
CheckIsNoThrowDestructible(int);
CheckIsNoThrowDestructible(int&);
CheckIsNoThrowDestructible(int&&);
CheckIsNoThrowDestructible(int*);
CheckIsNoThrowDestructible(const int);
CheckIsNoThrowDestructible(const volatile int&);
CheckIsNoThrowDestructible(void);
CheckIsNoThrowDestructible(const volatile void);
CheckIsNoThrowDestructible(int(long));
CheckIsNoThrowDestructible(long[]);
CheckIsNoThrowDestructible(long[3]);
CheckIsNoThrowDestructible(long[3][3][4]);
CheckIsNoThrowDestructible(long[][3][4]);

/// template<typename T> is_nothrow_move_assignable;
#define CheckIsNoThrowMoveAssignable(T)                                        \
    RAD_S_ASSERT(is_nothrow_move_assignable<T>::value ==                       \
                 std::is_nothrow_move_assignable<T>::value)

CheckIsNoThrowMoveAssignable(NoThrow);
CheckIsNoThrowMoveAssignable(Throw);
CheckIsNoThrowMoveAssignable(NoOps);
CheckIsNoThrowMoveAssignable(int);
CheckIsNoThrowMoveAssignable(int&);
CheckIsNoThrowMoveAssignable(int&&);
CheckIsNoThrowMoveAssignable(int*);
CheckIsNoThrowMoveAssignable(const int);
CheckIsNoThrowMoveAssignable(void);

/// template<typename T> is_nothrow_move_constructible;
#define CheckIsNoThrowMoveConstructible(T)                                     \
    RAD_S_ASSERT(is_nothrow_move_constructible<T>::value ==                    \
                 std::is_nothrow_move_constructible<T>::value)

CheckIsNoThrowMoveConstructible(NoThrow);
CheckIsNoThrowMoveConstructible(Throw);
CheckIsNoThrowMoveConstructible(NoOps);
CheckIsNoThrowMoveConstructible(int);
CheckIsNoThrowMoveConstructible(int&);
CheckIsNoThrowMoveConstructible(int&&);
CheckIsNoThrowMoveConstructible(int*);
CheckIsNoThrowMoveConstructible(const int);
CheckIsNoThrowMoveConstructible(void);

/// template<typename T> is_trivial;
#define CheckIsTrivial(T)                                                      \
    RAD_S_ASSERT(is_trivial<T>::value == std::is_trivial<T>::value)

CheckIsTrivial(int);
CheckIsTrivial(int&);
CheckIsTrivial(const int&);
CheckIsTrivial(int*);
CheckIsTrivial(void);
CheckIsTrivial(Trivial);
CheckIsTrivial(NonTrivial);

/// template<typename T> is_signed;
#define CheckIsSigned(T)                                                       \
    RAD_S_ASSERT(is_signed<T>::value == std::is_signed<T>::value)

CheckIsSigned(int);
CheckIsSigned(unsigned int);
CheckIsSigned(signed char);
CheckIsSigned(char);
CheckIsSigned(unsigned char);
CheckIsSigned(wchar_t);
CheckIsSigned(Base);
CheckIsSigned(float);
CheckIsSigned(void(int));

/// template<typename T> is_convertible;
#define CheckIsConvertible(from, to)                                           \
    RAD_S_ASSERT((is_convertible<from, to>::value ==                           \
                  std::is_convertible<from, to>::value))

CheckIsConvertible(void, void);
CheckIsConvertible(int, long);
CheckIsConvertible(long, int);
CheckIsConvertible(Derived, Base);
CheckIsConvertible(Base, Derived);
CheckIsConvertible(Derived*, Base*);
CheckIsConvertible(Convertable, Convert);
CheckIsConvertible(Convert, Convertable);
CheckIsConvertible(Convertable*, Convert*);
CheckIsConvertible(Convert*, Convertable*);
CheckIsConvertible(Derived, AlwaysConvertable);
CheckIsConvertible(int[3], int[3]);
CheckIsConvertible(int[3], long[3]);
CheckIsConvertible(long[3], int[3]);
CheckIsConvertible(int(int), void);
CheckIsConvertible(void, int(int));

/// template<typename T> has_virtual_destructor;
#define CheckHasVirtualDestructor(T)                                           \
    RAD_S_ASSERT(has_virtual_destructor<T>::value ==                           \
                 std::has_virtual_destructor<T>::value)

CheckHasVirtualDestructor(Derived);
CheckHasVirtualDestructor(Base);
CheckHasVirtualDestructor(DerivedPureVirtual);

/// template<typename T> make_unsigned;
#define CheckMakeUnsigned(T)                                                   \
    RAD_S_ASSERT((                                                             \
        is_same<make_unsigned<T>::type, std::make_unsigned<T>::type>::value)); \
    RAD_S_ASSERT((is_same<make_unsigned<const T>::type,                        \
                          std::make_unsigned<const T>::type>::value));         \
    RAD_S_ASSERT((is_same<make_unsigned<volatile T>::type,                     \
                          std::make_unsigned<volatile T>::type>::value));      \
    RAD_S_ASSERT((is_same<make_unsigned<const volatile T>::type,               \
                          std::make_unsigned<const volatile T>::type>::value))

CheckMakeUnsigned(signed char);
CheckMakeUnsigned(unsigned char);
CheckMakeUnsigned(signed short);
CheckMakeUnsigned(unsigned short);
CheckMakeUnsigned(signed int);
CheckMakeUnsigned(unsigned int);
CheckMakeUnsigned(signed long);
CheckMakeUnsigned(unsigned long);
CheckMakeUnsigned(signed long long);
CheckMakeUnsigned(unsigned long long);

CheckMakeUnsigned(char);
CheckMakeUnsigned(wchar_t);
CheckMakeUnsigned(char16_t);
CheckMakeUnsigned(char32_t);

CheckMakeUnsigned(Enum8);
CheckMakeUnsigned(Enum16);
CheckMakeUnsigned(Enum32);
CheckMakeUnsigned(OldEnum);

// TODO: The compilers agree that this should be a 64 bit value, but they do
// not agree on if that should mean unsigned long or unsigned long long, nor
// do the compilers agree on whether uint64_t should be unsigned long or
// unsigned long long
// CheckMakeUnsigned(Enum64);

#if RAD_CPP20
CheckMakeUnsigned(char8_t);
#endif

/// template<typename T> decay;
#define CheckDecay(T)                                                          \
    RAD_S_ASSERT((is_same<decay<T>::type, std::decay<T>::type>::value))

CheckDecay(int);
CheckDecay(int&);
CheckDecay(int&&);
CheckDecay(const int&);
CheckDecay(int[2]);
CheckDecay(int[4][2]);
CheckDecay(int(int));

namespace meta
{

RAD_S_ASSERT(Types::size() == 4);

RAD_S_ASSERT((is_same<short, Front<Types>::Type>::value));
RAD_S_ASSERT((is_same<FrontMissing, PopFront<Types>::Type>::value));
RAD_S_ASSERT((is_same<Types, PushFront<FrontMissing, short>::Type>::value));

RAD_S_ASSERT((is_same<short, GetAt<0, Types>::Type>::value));
RAD_S_ASSERT((is_same<int, GetAt<1, Types>::Type>::value));
RAD_S_ASSERT((is_same<int, GetAt<2, Types>::Type>::value));
RAD_S_ASSERT((is_same<long, GetAt<3, Types>::Type>::value));

RAD_S_ASSERT((Contains<int, Types>::Type::value));
RAD_S_ASSERT((Contains<short, Types>::Type::value));
RAD_S_ASSERT((Contains<long, Types>::Type::value));
RAD_S_ASSERT((!Contains<char, Types>::Type::value));

} // namespace meta

} // namespace rad
