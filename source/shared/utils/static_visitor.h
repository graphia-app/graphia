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

#ifndef STATIC_VISITOR_H
#define STATIC_VISITOR_H

namespace u
{
    // This is a template that multiply inherits from its parameters,
    // and exposes the operator() of each, the net result being the
    // ability to dispatch different bits of code depending on the type
    // of the thing passed to operator(). e.g.:
    //
    // u::static_visitor
    // {
    //     [](TypeA thing) { /* do a TypeA thing */ },
    //     [](TypeB thing) { /* do a TypeB thing */ }
    // } v;
    //
    // TypeA a;
    // TypeB b;
    //
    // v(a);
    // v(b);

    template<typename... Base>
    struct static_visitor : Base...
    {
        using Base::operator()...;
    };

    // Deduction guide, so we can invoke the static_visitor constructor without
    // having to specify various complicated template parameters
    template<typename... T> static_visitor(T...) -> static_visitor<T...>;
} // namespace u

#endif // STATIC_VISITOR_H
