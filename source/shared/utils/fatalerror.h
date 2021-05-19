/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#if defined(_MSC_VER)
#define NOINLINE __declspec(noinline)
#elif defined(__GNUC__)
#define NOINLINE __attribute__ ((noinline)) /* NOLINT cppcoreguidelines-macro-usage */
#endif

// Deliberately crash in order to help track down bugs
// MESSAGE becomes a struct name, so quotes and spaces are not allowed
#define FATAL_ERROR(MESSAGE) /* NOLINT cppcoreguidelines-macro-usage */ \
    do { struct MESSAGE \
        { \
            NOINLINE void operator()() \
            { \
                int* p = nullptr; \
                *p = 0; \
            } \
        } s; s(); \
    } while(0)

#endif // FATALERROR_H
