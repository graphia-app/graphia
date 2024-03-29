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

#ifndef STATIC_BLOCK_H
#define STATIC_BLOCK_H

#include <map>
#include <string>

// This allows for the execution of static blocks of code, in a deterministic order,
// whenever execute_static_blocks() is called. Note that if a static_block is included
// in a static library, its compilation unit (obviously) must be linked for it to work,
// but that isn't necessarily the case if the compilation unit is otherwise unreferenced.

inline std::map<std::string, void(*)()> _static_blocks;

void execute_static_blocks();

// NOLINTBEGIN(cppcoreguidelines-macro-usage)

#define STATIC_BLOCK_ID_CONCAT(p, l) p ## l
#define STATIC_BLOCK_ID_EXPAND(p, l) STATIC_BLOCK_ID_CONCAT(p, l)
#define STATIC_BLOCK_ID STATIC_BLOCK_ID_EXPAND(static_block_, __COUNTER__)

#define STATIC_BLOCK_2(f, c) /* NOLINT cppcoreguidelines-macro-usage */ \
    static void f(); \
    namespace \
    { \
    static const struct c \
    { \
        inline c() \
        { \
            std::string sid(__FILE__); sid += std::to_string(__LINE__); \
            _static_blocks[sid] = f; \
        } \
    } STATIC_BLOCK_ID_CONCAT(c, _instance); \
    } \
    static void f()

#define STATIC_BLOCK_1(id) /* NOLINT cppcoreguidelines-macro-usage */ \
    STATIC_BLOCK_2( \
        STATIC_BLOCK_ID_CONCAT(id, _function), \
        STATIC_BLOCK_ID_CONCAT(id, _class) \
    )

#define static_block STATIC_BLOCK_1(STATIC_BLOCK_ID)

// NOLINTEND(cppcoreguidelines-macro-usage)

#endif // STATIC_BLOCK_H
