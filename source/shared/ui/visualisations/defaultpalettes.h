/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#ifndef DEFAULTPALETTES_H
#define DEFAULTPALETTES_H

namespace Defaults
{
const char* const PALETTE_PRESETS =
R"([
  {
    "autoColors":[
      "#199311",
      "#FF7700",
      "#8126C0",
      "#1DF3F3",
      "#FFEE33",
      "#FD1111",
      "#FF69B4",
      "#003BFF",
      "#8AFF1E",
      "#9D2323",
      "#798FF0",
      "#111111"
    ],
    "defaultColor":"#DDDDDD"
  },
  {
    "autoColors":[
      "#199311",
      "#FF7700",
      "#8126C0",
      "#1DF3F3",
      "#FFEE33",
      "#FD1111",
      "#FF69B4",
      "#003BFF",
      "#8AFF1E",
      "#9D2323",
      "#798FF0",
      "#111111"
    ]
  },
  {
    "autoColors":[
      "#000000",
      "#00e4ff",
      "#0080ff",
      "#6a3efc"
    ],
    "defaultColor":"#ffffff"
  },
  {
    "autoColors":[
      "#A2DA22",
      "#c17d11",
      "#204a87",
      "#ec1a1a"
    ],
    "defaultColor":"#f3f3f3"
  },
  {
    "autoColors":[
      "#FF0000",
      "#00FF00",
      "#0000FF",
      "#fff800"
    ],
    "defaultColor":"#f3f3f3"
  },
  {
    "autoColors":[

    ],
    "defaultColor":"#f3f3f3",
    "fixedColors":{
      "True":"#ef2929"
    }
  }
])";

const char* const PALETTE =
R"({
  "autoColors":[
    "#199311",
    "#FF7700",
    "#8126C0",
    "#1DF3F3",
    "#FFEE33",
    "#FD1111",
    "#FF69B4",
    "#003BFF",
    "#8AFF1E",
    "#9D2323",
    "#798FF0",
    "#111111"
  ]
})";
} // namespace Defaults

#endif // DEFAULTPALETTES_H
