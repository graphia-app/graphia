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

#ifndef TEXTVISUAL_H
#define TEXTVISUAL_H

#include "shared/graph/elementid.h"

#include <QString>
#include <QColor>
#include <QVector3D>

#include <vector>
#include <map>

class NodePositions;

struct TextVisual
{
    QString _text;
    float _size = -1.0f;
    QColor _color;

    std::vector<NodeId> _nodeIds;

    QVector3D _centre;
    float _radius = 0.0f;

    void updatePositions(const NodePositions& nodePositions);
};

using TextVisuals = std::map<ComponentId, std::vector<TextVisual>>;

#endif // TEXTVISUAL_H
