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

#include "static_block.h"

#include <vector>
#include <algorithm>

void execute_static_blocks()
{
    // Sort the map so that the static_blocks are executed in a deterministic order
    std::vector<std::pair<std::string, void(*)()>> v(_static_blocks.begin(), _static_blocks.end());
    std::sort(v.begin(), v.end());

    for(const auto& [n, f] : v)
        f();

    _static_blocks.clear();
}
