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

#ifndef GRAPHOVERVIEWINTERACTOR_H
#define GRAPHOVERVIEWINTERACTOR_H

#include "graphcommoninteractor.h"

#include <QPoint>

#include <memory>

class GraphModel;
class GraphOverviewScene;
class CommandManager;
class SelectionManager;
class GraphQuickItem;

class GraphOverviewInteractor : public GraphCommonInteractor
{
    Q_OBJECT
public:
    GraphOverviewInteractor(GraphModel* graphModel,
                            GraphOverviewScene* graphOverviewScene,
                            CommandManager* commandManager,
                            SelectionManager* selectionManager,
                            GraphRenderer* graphRenderer);

private:
    GraphOverviewScene* _scene;
    QPoint _panStartPosition;

    void rightMouseDown() override;
    void rightMouseUp() override;
    void rightDrag() override;

    void leftDoubleClick() override;

    void wheelMove(float angle, float x, float y) override;
    void trackpadZoomGesture(float value, float x, float y) override;

    ComponentId componentIdAtPosition(const QPoint& position) const;
    GraphComponentRenderer* componentRendererAtPosition(const QPoint& position) const override;
    QPoint componentLocalCursorPosition(const ComponentId& componentId, const QPoint& position) const override;
    NodeIdSet selectionForRect(const QRectF& rect) const override;
};

#endif // GRAPHOVERVIEWINTERACTOR_H
