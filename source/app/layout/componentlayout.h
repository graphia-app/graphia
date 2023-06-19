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

#ifndef COMPONENTLAYOUT_H
#define COMPONENTLAYOUT_H

#include "shared/graph/grapharray.h"
#include "maths/circle.h"

#include <QRectF>

using ComponentLayoutData = ComponentArray<Circle>;

class Graph;

class ComponentLayout
{
public:
    virtual ~ComponentLayout() = default;

    void execute(const Graph& graph, const std::vector<ComponentId>& componentIds,
        ComponentLayoutData& componentLayoutData);

    QRectF boundingBoxFor(const std::vector<ComponentId>& componentIds,
        ComponentLayoutData& componentLayoutData) const;

    float boundingWidth() const { return static_cast<float>(_boundingBox.width()); }
    float boundingHeight() const { return static_cast<float>(_boundingBox.height()); }

private:
    virtual void executeReal(const Graph& graph, const std::vector<ComponentId>& componentIds,
        ComponentLayoutData& componentLayoutData) = 0;

    QRectF _boundingBox;
};

#endif // COMPONENTLAYOUT_H

