#ifndef ENUMBITMASK_H
#define ENUMBITMASK_H

// To enable bitwise operators for enum classes, include this file, then
// specialise the templated bool as follows:
//
// template<> constexpr bool EnableBitMaskOperators<ExampleEnum> = true;

#include <type_traits>

template<typename Enum>
constexpr bool EnableBitMaskOperators = false;

template<typename Enum>
typename std::enable_if<EnableBitMaskOperators<Enum>, Enum>::type
operator|(Enum lhs, Enum rhs)
{
    using underlying = typename std::underlying_type<Enum>::type;

    return static_cast<Enum>(
        static_cast<underlying>(lhs) |
        static_cast<underlying>(rhs)
    );
}

template<typename Enum>
typename std::enable_if<EnableBitMaskOperators<Enum>, Enum>::type
operator&(Enum lhs, Enum rhs)
{
    using underlying = typename std::underlying_type<Enum>::type;

    return static_cast<Enum>(
        static_cast<underlying>(lhs) &
        static_cast<underlying>(rhs)
    );
}

template<typename Lhs, typename Enum>
typename std::enable_if<EnableBitMaskOperators<Enum>, bool>::type
operator&&(Lhs lhs, Enum rhs)
{
    using underlying = typename std::underlying_type<Enum>::type;

    return lhs && (
        static_cast<underlying>(rhs) !=
        static_cast<underlying>(0)
    );
}

template<typename Enum, typename Rhs>
typename std::enable_if<EnableBitMaskOperators<Enum>, bool>::type
operator&&(Enum lhs, Rhs rhs)
{
    using underlying = typename std::underlying_type<Enum>::type;

    return (
        static_cast<underlying>(lhs) !=
        static_cast<underlying>(0)
    ) && rhs;
}

#endif // ENUMBITMASK_H
