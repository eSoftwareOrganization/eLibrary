#pragma once

#ifndef eLibraryHeaderCoreType
#define eLibraryHeaderCoreType

#include <concepts>
#include <type_traits>

namespace eLibrary::Core::Type {
    template<typename T>
    ::std::true_type isBaseOf0(const volatile T*);
    template<typename>
    ::std::false_type isBaseOf0(const volatile void*);

    template<typename Tb, typename Td>
    auto isBaseOf0(int) -> decltype(isBaseOf0<Tb>(static_cast<Td*>(nullptr)));
    template<typename, typename>
    auto isBaseOf0(...) -> ::std::true_type;

    template<typename T>
    auto isConvertible0(int) -> decltype(void(static_cast<T(*)()>(nullptr)), ::std::true_type{});
    template<typename>
    auto isConvertible0(...) -> ::std::false_type;

    template<typename Ts, typename Tt>
    auto isConvertible0(int) -> decltype(void(::std::declval<void(&)(Tt)>()(::std::declval<Ts>())), ::std::true_type{});
    template<typename, typename>
    auto isConvertible0(...) -> ::std::false_type;

    template<bool V0, bool V1, bool V2>
    constexpr bool Conditional = V0 ? V1 : V2;

    template<typename T>
    constexpr bool isClass = ::std::is_class_v<T>;

    template<typename T>
    constexpr bool isFloatingPoint = ::std::is_floating_point_v<T>;

    template<typename T>
    constexpr bool isIntegral = ::std::is_integral_v<T>;

    template<typename T>
    constexpr bool isArithmetic = isIntegral<T> || isFloatingPoint<T>;

    template<typename>
    constexpr bool isPointer = false;

    template<typename T>
    constexpr bool isPointer<T *> = true;

    template<typename>
    constexpr bool isReference = false;

    template<typename T>
    constexpr bool isReference<T &> = true;

    template<typename T>
    constexpr bool isReference<T &&> = true;

    template<typename, typename>
    constexpr bool isSame = false;

    template<typename T>
    constexpr bool isSame<T, T> = true;

    template<typename>
    constexpr bool isVoid = false;

    template<>
    constexpr bool isVoid<void> = true;

    template<typename Tb, typename Td>
    constexpr bool isBaseOf = isClass<Tb> && isClass<Td> && decltype(isBaseOf0<Tb, Td>(0))::value;

    template<typename Ts, typename Tt>
    constexpr bool isConvertible = (decltype(isConvertible0<Tt>(0))::value && decltype(isConvertible0<Ts, Tt>(0))::value) || (isVoid<Ts> && isVoid<Tt>);

    template<typename T>
    struct noConst0 {
        using Type = T;
    };

    template<typename T>
    struct noConst0<const T> {
        using Type = T;
    };

    template<typename T>
    struct noReference0 {
        using Type = T;
    };

    template<typename T>
    struct noReference0<T&> {
        using Type = T;
    };

    template<typename T>
    struct noReference0<T&&> {
        using Type = T;
    };

    template<typename T>
    struct noVolatile0 {
        using Type = T;
    };

    template<typename T>
    struct noVolatile0<volatile T> {
        using Type = T;
    };

    template<typename T>
    using noConst = noConst0<T>::Type;

    template<typename T>
    using noReference = noReference0<T>::Type;

    template<typename T>
    using noVolatile = noVolatile0<T>::Type;

    template<typename T>
    concept Arithmetic = isArithmetic<T>;

    template<typename T>
    concept Pointer = isPointer<T>;

    template<typename T1, typename T2>
    concept SizeEqual = sizeof(T1) == sizeof(T2);

    template<typename T1, typename T2>
    concept TypeEqual = isSame<T1, T2> && isSame<T2, T1>;

    template<typename T>
    concept Comparable = requires(const T &ObjectSource) {
        { ObjectSource.doCompare(ObjectSource) } -> TypeEqual<intmax_t>;
    };
}

#endif
