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

#ifndef INTERACTOR_H
#define INTERACTOR_H

#include "graph/qmlelementid.h"

#include <QObject>
#include <Qt>

class QPoint;

class Interactor : public QObject
{
    Q_OBJECT

public:
    explicit Interactor(QObject* parent = nullptr) :
        QObject(parent)
    {}

    ~Interactor() override = default;

    virtual void mousePressEvent(const QPoint& pos, Qt::KeyboardModifiers modifiers, Qt::MouseButton button) = 0;
    virtual void mouseReleaseEvent(const QPoint& pos, Qt::KeyboardModifiers modifiers, Qt::MouseButton button) = 0;
    virtual void mouseMoveEvent(const QPoint& pos, Qt::KeyboardModifiers modifiers, Qt::MouseButton button) = 0;
    virtual void mouseDoubleClickEvent(const QPoint& pos, Qt::KeyboardModifiers modifiers, Qt::MouseButton button) = 0;
    virtual void wheelEvent(const QPoint& pos, int angle) = 0;
    virtual void zoomGestureEvent(const QPoint& pos, float value) = 0;
    virtual void panGestureEvent(const QPoint& pos, const QPoint& delta) = 0;

signals:
    void userInteractionStarted();
    void userInteractionFinished();

    void clicked(int button, int modifiers, QmlNodeId nodeId);
};

#endif // INTERACTOR_H
