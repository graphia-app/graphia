/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#ifndef NORMALISER_H
#define NORMALISER_H

#include "correlationdatavector.h"

#include <vector>
#include <cstdlib>

class Cancellable;
class IParser;

class Normaliser
{
public:
    virtual ~Normaliser() = default;
    virtual bool process(ContinuousDataVectors& dataRows, IParser* parser = nullptr) const = 0;
};

#endif // NORMALISER_H
