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

#ifndef GRAPHCOMPONENTINTERACTOR_H
#define GRAPHCOMPONENTINTERACTOR_H

#include "graphcommoninteractor.h"
#include "rendering/graphcomponentscene.h"

#include <QPoint>

#include <memory>

class GraphModel;
class CommandManager;
class SelectionManager;
class GraphQuickItem;

class GraphComponentInteractor : public GraphCommonInteractor
{
    Q_OBJECT
public:
    GraphComponentInteractor(GraphModel* graphModel,
                             GraphComponentScene* graphComponentScene,
                             CommandManager* commandManager,
                             SelectionManager* selectionManager,
                             GraphRenderer* graphRenderer);

private:
    GraphComponentScene* _scene;

    void rightMouseDown() override;
    void rightMouseUp() override;
    void rightDrag() override;

    void leftDoubleClick() override;

    void wheelMove(float angle, float x, float y) override;
    void trackpadZoomGesture(float value, float x, float y) override;
    void trackpadPanGesture(float dx, float dy, float x, float y) override;

    GraphComponentRenderer* componentRendererAtPosition(const QPoint& position) const override;
    QPoint componentLocalCursorPosition(const ComponentId& componentId, const QPoint& position) const override;
    NodeIdSet selectionForRect(const QRectF& rect) const override;
};

#endif // GRAPHCOMPONENTINTERACTOR_H
