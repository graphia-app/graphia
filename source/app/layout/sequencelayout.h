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

#ifndef SEQUENCELAYOUT_H
#define SEQUENCELAYOUT_H

#include "layout.h"

#include <vector>
#include <memory>
#include <algorithm>

class SequenceLayout : public Layout
{
    Q_OBJECT
private:
    std::vector<Layout*> _subLayouts;

public:
    SequenceLayout(const IGraphComponent& graphComponent, NodeLayoutPositions& positions) :
        Layout(graphComponent, positions)
    {}

    SequenceLayout(const IGraphComponent& graphComponent,
                   NodeLayoutPositions& positions,
                   std::vector<Layout*> subLayouts) :
        Layout(graphComponent, positions), _subLayouts(subLayouts)
    {}

    virtual ~SequenceLayout()
    {}

    void addSubLayout(Layout* layout) { _subLayouts.push_back(layout); }

    void cancel()
    {
        for(auto subLayout : _subLayouts)
            subLayout->cancel();
    }

    void uncancel()
    {
        for(auto subLayout : _subLayouts)
            subLayout->uncancel();
    }

    bool finished() const
    {
        return std::any_of(_subLayouts.begin(), _subLayouts.end(),
                           [](Layout* layout) { return layout->finished(); });
    }

    void unfinish()
    {
        for(auto subLayout : _subLayouts)
            subLayout->unfinish();
    }

    bool iterative() const
    {
        return std::any_of(_subLayouts.begin(), _subLayouts.end(),
                           [](Layout* layout) { return layout->iterative(); });
    }

    Dimensionality dimensionality() const
    {
        if(std::any_of(_subLayouts.begin(), _subLayouts.end(),
            [](Layout* layout) { return layout->dimensionality() == Dimensionality::TwoDee; }))
        {
            return Dimensionality::TwoDee;
        }

        if(std::any_of(_subLayouts.begin(), _subLayouts.end(),
            [](Layout* layout) { return layout->dimensionality() == Dimensionality::TwoOrThreeDee; }))
        {
            return Dimensionality::TwoOrThreeDee;
        }

        return Dimensionality::ThreeDee;
    }

    void execute(bool firstIteration, Dimensionality dimensionality)
    {
        for(auto subLayout : _subLayouts)
            subLayout->execute(firstIteration, dimensionality);
    }
};

#endif // SEQUENCELAYOUT_H
