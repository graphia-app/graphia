/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#ifndef VISITOR_H
#define VISITOR_H

// This is a template to facilitate construction of a functor which
// has multiple overloaded operator()s each with a different parameter type,
// thus the object as a whole being a type matching functor. e.g.:
//
// Visitor
// {
//     [](int) { ... },
//     [](double) { ... },
//     [](const Struct&) { ... }
// }

template<class... Ts> struct Visitor : Ts... { using Ts::operator()...; };

//FIXME: in theory this deduction guide is unnecssary for C++20, but this doesn't appear to be the case (yet?)
template<class... Ts> Visitor(Ts...) -> Visitor<Ts...>;

#endif // VISITOR_H
