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

#ifndef FASTINITIALLAYOUT_H
#define FASTINITIALLAYOUT_H

#include "layout.h"

class FastInitialLayout : public Layout
{
    Q_OBJECT

private:
    void positionNode(QVector3D& offsetPosition, const QMatrix4x4& orientationMatrix,
                      const QVector3D& parentNodePosition, NodeId childNodeId,
                      NodeArray<QVector3D>& directionNodeVectors);
public:
    FastInitialLayout(const IGraphComponent& graphComponent, NodeLayoutPositions& positions)
        : Layout(graphComponent, positions)
    {}

    void execute(bool, Dimensionality dimensionality) override;
};

#endif // FASTINITIALLAYOUT_H
