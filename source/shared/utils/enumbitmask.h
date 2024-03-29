/* Copyright © 2013-2024 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

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
typename std::enable_if_t<EnableBitMaskOperators<Enum>, Enum>
operator|(Enum lhs, Enum rhs)
{
    using underlying = typename std::underlying_type_t<Enum>;

    return static_cast<Enum>(
        static_cast<underlying>(lhs) |
        static_cast<underlying>(rhs)
    );
}

template<typename Enum>
typename std::enable_if_t<EnableBitMaskOperators<Enum>, Enum>
operator&(Enum lhs, Enum rhs)
{
    using underlying = typename std::underlying_type_t<Enum>;

    return static_cast<Enum>(
        static_cast<underlying>(lhs) &
        static_cast<underlying>(rhs)
    );
}

template<typename Lhs, typename Enum>
typename std::enable_if_t<EnableBitMaskOperators<Enum>, bool>
operator&&(Lhs lhs, Enum rhs)
{
    using underlying = typename std::underlying_type_t<Enum>;

    return lhs && (
        static_cast<underlying>(rhs) !=
        static_cast<underlying>(0)
    );
}

template<typename Enum, typename Rhs>
typename std::enable_if_t<EnableBitMaskOperators<Enum>, bool>
operator&&(Enum lhs, Rhs rhs)
{
    using underlying = typename std::underlying_type_t<Enum>;

    return (
        static_cast<underlying>(lhs) !=
        static_cast<underlying>(0)
    ) && rhs;
}

#endif // ENUMBITMASK_H
