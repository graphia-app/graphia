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

#ifndef VISUALISATIONBUILDER_H
#define VISUALISATIONBUILDER_H

#include "visualisationchannel.h"
#include "visualisationconfig.h"
#include "visualisationinfo.h"
#include "visualisationmapping.h"

#include "graph/graph.h"
#include "shared/graph/grapharray.h"
#include "shared/utils/utils.h"
#include "shared/utils/container.h"
#include "attributes/attribute.h"

#include <vector>
#include <array>
#include <type_traits>

#include <QCollator>
#include <QtGlobal>

template<typename ElementId>
class VisualisationsBuilder
{
public:
    VisualisationsBuilder(const Graph& graph,
        ElementIdArray<ElementId, ElementVisual>& visuals) :
        _graph(&graph), _visuals(&visuals)
    {}

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
               ElementId elementId, size_t index)
    {
        auto& visual = (*_visuals)[elementId];
        auto oldVisual = visual;
        channel.apply(value, visual);

        Q_ASSERT(index < _applications[0].size());

        // Must not exceed NumChannels
        _applications[0].at(index)._array.set(elementId, oldVisual._size  != visual._size);
        _applications[1].at(index)._array.set(elementId, oldVisual._innerColor != visual._innerColor ||
                oldVisual._outerColor != visual._outerColor);
        _applications[2].at(index)._array.set(elementId, oldVisual._text  != visual._text);
    }

    template<typename G>
    std::vector<ElementId> elementIds(const G* graph) const
    {
        if constexpr(std::is_same_v<ElementId, NodeId>)
            return u::vectorFrom(_graph->mergedNodeIdsForNodeIds(graph->nodeIds()));

        if constexpr(std::is_same_v<ElementId, EdgeId>)
            return u::vectorFrom(_graph->mergedEdgeIdsForEdgeIds(graph->edgeIds()));

        return {};
    }

    std::vector<ElementId> elementIds() const
    {
        return elementIds(_graph);
    }

public:
    void findOverrideAlerts(VisualisationInfosMap& infos)
    {
        if(_numAppliedVisualisations < 2)
            return;

        for(size_t c = 0; c < NumChannels; c++)
        {
            for(size_t i = 0; i < _numAppliedVisualisations - 1; i++)
            {
                const auto& iv = _applications.at(c).at(i);

                for(size_t j = i + 1; j < _numAppliedVisualisations; j++)
                {
                    const auto& jv = _applications.at(c).at(j);
                    int bothSet = 0, sourceSet = 0;

                    for(auto elementId : elementIds())
                    {
                        if(iv._array.get(elementId))
                        {
                            sourceSet++;

                            if(jv._array.get(elementId))
                                bothSet++;
                        }
                    }

                    if(bothSet > 0)
                    {
                        if(bothSet != sourceSet)
                        {
                            infos[iv._index].addAlert(AlertType::Warning,
                                QObject::tr("Partially overriden by subsequent visualisations"));
                        }
                        else
                        {
                            infos[iv._index].addAlert(AlertType::Error,
                                QObject::tr("Overriden by subsequent visualisations"));
                        }
                    }
                }
            }
        }
    }

    void build(const Attribute& attribute,
               const VisualisationChannel& channel,
               const VisualisationConfig& config,
               int index, VisualisationInfo& visualisationInfo)
    {
        for(size_t c = 0; c < NumChannels; c++)
            _applications.at(c).emplace_back(index, *_graph);

        if(elementIds().empty())
        {
            visualisationInfo.addAlert(AlertType::Error, QObject::tr("No elements to visualise"));
            return;
        }

        switch(attribute.valueType())
        {
        case ValueType::Int:
        case ValueType::Float:
        {
            const bool invert = config.isFlagSet(QStringLiteral("invert"));
            const bool perComponent = config.isFlagSet(QStringLiteral("component"));

            // This is only here to avoid an internal compiler error
            const auto& mappingValue = QStringLiteral("mapping");

            size_t numApplications = 0;

            auto applyTo = [&](const auto& graph, const u::Statistics& statistics)
            {
                if(channel.requiresRange() && statistics._range == 0.0)
                {
                    visualisationInfo.addAlert(AlertType::Warning,
                        QObject::tr("No numeric range in one or more components"));
                    return;
                }

                const VisualisationMapping mapping(statistics, config.parameterValue(mappingValue));

                visualisationInfo.setMappedMinimum(mapping.min());
                visualisationInfo.setMappedMaximum(mapping.max());

                for(auto elementId : elementIds(graph))
                {
                    double value = attribute.numericValueOf(elementId);

                    if(channel.allowsMapping())
                    {
                        if(invert)
                            value = statistics.inverse(value);

                        value = mapping.map(value);
                    }

                    apply(value, channel, elementId, _numAppliedVisualisations);
                }

                numApplications++;
            };

            auto statistics = attribute.findStatisticsforElements(elementIds(), true);

            if(perComponent)
            {
                for(auto componentId : _graph->componentIds())
                {
                    const auto* component = _graph->componentById(componentId);
                    auto componentStatistics = attribute.findStatisticsforElements(elementIds(component));
                    applyTo(component, componentStatistics);
                }
            }
            else
                applyTo(_graph, statistics);

            visualisationInfo.setStatistics(statistics);
            visualisationInfo.setNumApplications(numApplications);

            if(numApplications > 0)
                _numAppliedVisualisations++;

            break;
        }

        case ValueType::String:
        {
            for(auto elementId : elementIds())
            {
                auto stringValue = attribute.stringValueOf(elementId);
                apply(stringValue, channel, elementId, _numAppliedVisualisations);
            }

            _numAppliedVisualisations++;
            break;
        }

        default:
            break;
        }
    }
};

#endif // VISUALISATIONBUILDER_H
