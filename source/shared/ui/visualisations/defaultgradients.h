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

#ifndef DEFAULTGRADIENTS_H
#define DEFAULTGRADIENTS_H

namespace Defaults
{
const char* const GRADIENT_PRESETS =
R"([{
    "0":    "Red",
    "0.66": "Yellow",
    "1":    "White"
},
{
    "0":   "Black",
    "0.4": "Purple",
    "0.6": "Red",
    "0.8": "Yellow",
    "1":   "White"
},
{
    "0.00": "#F0F",
    "0.25": "#00F",
    "0.50": "#0F0",
    "0.75": "#FF0",
    "1.00": "#F00"
},
{
    "0":    "Navy",
    "0.25": "Navy",
    "0.26": "Green",
    "0.5":  "Green",
    "0.51": "Yellow",
    "0.75": "Yellow",
    "0.76": "Red",
    "1":    "Red"
},
{
    "0":    "Black",
    "0.33": "DarkRed",
    "0.66": "Yellow",
    "1":    "White"
},
{
    "0":   "Black",
    "0.5": "Aqua",
    "1":   "White"
},
{
    "0.0": "blue",
    "1":   "red"
},
{
    "0.0":  "#000000",
    "0.6":  "#183567",
    "0.75": "#2E649E",
    "0.9":  "#17ADCB",
    "1.0":  "#00FAFA"
}])";

const char* const GRADIENT =
R"({
    "0":    "White",
    "0.33": "Yellow",
    "1":    "Red"
})";
} // namespace Defaults

#endif // DEFAULTGRADIENTS_H
