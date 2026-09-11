/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#ifndef LAYOUT_H
#define LAYOUT_H

#include "shared/graph/igraphcomponent.h"
#include "shared/graph/elementid.h"
#include "shared/graph/elementid_containers.h"

#include "shared/utils/cancellable.h"

#include "nodepositions.h"
#include "layoutsettings.h"

#include <QObject>

#include <cstddef>
#include <vector>

class Layout : public QObject, public Cancellable
{
    Q_OBJECT

public:
    enum class Iterative : unsigned char
    {
        Yes,
        No
    };

    enum class Dimensionality : unsigned char
    {
        ThreeDee        = 0x1,
        TwoDee          = 0x2,
        TwoOrThreeDee   = TwoDee | ThreeDee
    };

private:
    Iterative _iterative;
    Dimensionality _dimensionality;
    float _scaling;
    size_t _smoothing;
    const IGraphComponent* _graphComponent;
    NodeLayoutPositions* _positions;

protected:
    const LayoutSettings* _settings;

    NodeLayoutPositions& positions() { return *_positions; }

public:
    Layout(const IGraphComponent& graphComponent,
        NodeLayoutPositions& positions,
        const LayoutSettings* settings = nullptr,
        Iterative iterative = Iterative::No,
        Dimensionality dimensionality = Dimensionality::TwoOrThreeDee,
        float scaling = 1.0f,
        size_t smoothing = 1) :
        _iterative(iterative),
        _dimensionality(dimensionality),
        _scaling(scaling),
        _smoothing(smoothing),
        _graphComponent(&graphComponent),
        _positions(&positions),
        _settings(settings)
    {}

    float scaling() const { return _scaling; }
    size_t smoothing() const { return _smoothing; }

    const IGraphComponent& graphComponent() const { return *_graphComponent; }
    const std::vector<NodeId>& nodeIds() const { return _graphComponent->nodeIds(); }
    const std::vector<EdgeId>& edgeIds() const { return _graphComponent->edgeIds(); }

    virtual void execute(bool firstIteration, Dimensionality dimensionalityMode) = 0;

    // Indicates that the algorithm is doing no useful work
    virtual bool finished() const { return false; }

    // Resets the state of the algorithm such that finished() no longer returns true
    virtual void unfinish() { qFatal("unfinish not implemented"); }

    virtual bool iterative() const { return _iterative == Iterative::Yes; }
    virtual Dimensionality dimensionality() const { return _dimensionality; }

signals:
    void progress(int percentage);
};

#endif // LAYOUT_H
