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

#ifndef SCENE_H
#define SCENE_H

#include "projection.h"

#include <QObject>

#include <memory>

class Scene : public QObject
{
    Q_OBJECT

    friend class GraphRenderer;

public:
    explicit Scene(QObject* parent = nullptr) :
        QObject(parent)
    {}
    ~Scene() override = default;

    virtual void cleanup() {}
    virtual void update(float t) = 0;
    virtual void setViewportSize(int width, int height) = 0;

    virtual bool transitionActive() const = 0;

    virtual void onShow() {}
    virtual void onHide() {}

    virtual void resetView(bool doTransition = true) = 0;
    virtual bool viewIsReset() const = 0;

    virtual void setProjection(Projection projection) = 0;

protected:
    static constexpr float defaultDuration = 0.3f;

    bool visible() const { return _visible; }

private:
    bool _interactionEnabled = true;
    bool _initialised = false;
    bool _visible = false;

    void setVisible(bool visible) { _visible = visible; }
};

#endif // SCENE_H
