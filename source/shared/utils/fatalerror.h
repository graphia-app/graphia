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

#ifndef FATALERROR_H
#define FATALERROR_H

#include <iostream>

#if defined(_MSC_VER)
#include <intrin.h>
#define NOINLINE __declspec(noinline)
#elif defined(__GNUC__)
#define NOINLINE __attribute__ ((noinline))
#endif

namespace u
{
// Inlined so that the crashing frame is the one FATAL_ERROR names
#if defined(_MSC_VER)
__forceinline void trap() { __ud2(); }
#elif defined(__GNUC__)
__attribute__ ((always_inline)) inline void trap() { __builtin_trap(); }
#endif
} // namespace u

// Deliberately crash in order to help track down bugs
// MESSAGE becomes a struct name, so quotes and spaces are not allowed
#define FATAL_ERROR(MESSAGE) \
    do { struct MESSAGE \
        { \
            NOINLINE void operator()() \
            { \
                std::cerr << "Fatal error: " #MESSAGE << "\n"; \
                u::trap(); \
            } \
        } s; s(); \
    } while(0)

#endif // FATALERROR_H
