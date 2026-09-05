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

#ifndef VISUALISATIONBUILDER_H
#define VISUALISATIONBUILDER_H

#include "visualisationinfo.h"

#include "shared/graph/grapharray.h"

#include <array>
#include <cstddef>
#include <vector>

class Graph;
class Attribute;
class VisualisationChannel;
struct VisualisationConfig;
struct ElementVisual;

template<typename ElementId>
class VisualisationsBuilder
{
public:
    VisualisationsBuilder(const Graph& graph,
        ElementIdArray<ElementId, ElementVisual>& visuals) :
        _graph(&graph), _visuals(&visuals)
    {}

    void findOverrideAlerts(VisualisationInfosMap& infos);

    void build(const Attribute& attribute,
        const VisualisationChannel& channel,
        const VisualisationConfig& config,
        int index, VisualisationInfo& visualisationInfo);

private:
    const Graph* _graph;
    ElementIdArray<ElementId, ElementVisual>* _visuals;
    size_t _numAppliedVisualisations = 0;

    static const size_t NumChannels = 3;

    struct Applied
    {
        Applied(int index, const IGraphArrayClient& graph) :
            _index(index), _array(graph)
        {}

        int _index;
        ElementIdArray<ElementId, bool> _array;
    };

    std::array<std::vector<Applied>, NumChannels> _applications;

    template<typename T>
    void apply(T value, const VisualisationChannel& channel,
               ElementId elementId, size_t index);

    template<typename G>
    std::vector<ElementId> elementIds(const G* graph) const;

    std::vector<ElementId> elementIds() const;
};

#endif // VISUALISATIONBUILDER_H
