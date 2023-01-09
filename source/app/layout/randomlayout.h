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

#ifndef RANDOMLAYOUT_H
#define RANDOMLAYOUT_H

#include "layout.h"

class RandomLayout : public Layout
{
    Q_OBJECT
private:
    float _spread = 10.0f;

public:
    RandomLayout(const IGraphComponent& graphComponent, NodeLayoutPositions& positions) :
        Layout(graphComponent, positions)
    {}

    void setSpread(float spread) { _spread = spread; }
    void execute(bool, Dimensionality) override;
};

#endif // RANDOMLAYOUT_H
