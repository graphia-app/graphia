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

#include "visualisationbuilder.h"

#include "visualisationchannel.h"
#include "visualisationconfig.h"
#include "visualisationmapping.h"

#include "app/attributes/attribute.h"
#include "app/graph/graph.h"
#include "app/ui/visualisations/elementvisual.h"

#include "shared/graph/igraphcomponent.h"

#include "shared/utils/container.h"
#include "shared/utils/statistics.h"

#include <QObject>
#include <QString>
#include <QtGlobal>

#include <type_traits>

using namespace Qt::Literals::StringLiterals;

template<typename ElementId>
template<typename T>
void VisualisationsBuilder<ElementId>::apply(T value, const VisualisationChannel& channel,
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

template<typename ElementId>
template<typename G>
std::vector<ElementId> VisualisationsBuilder<ElementId>::elementIds(const G* graph) const
{
    if constexpr(std::is_same_v<ElementId, NodeId>)
        return u::vectorFrom(_graph->mergedNodeIdsForNodeIds(graph->nodeIds()));

    if constexpr(std::is_same_v<ElementId, EdgeId>)
        return u::vectorFrom(_graph->mergedEdgeIdsForEdgeIds(graph->edgeIds()));

    return {};
}

template<typename ElementId>
std::vector<ElementId> VisualisationsBuilder<ElementId>::elementIds() const
{
    return elementIds(_graph);
}

template<typename ElementId>
void VisualisationsBuilder<ElementId>::findOverrideAlerts(VisualisationInfosMap& infos)
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

template<typename ElementId>
void VisualisationsBuilder<ElementId>::build(const Attribute& attribute,
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
        const bool invert = config.isFlagSet(u"invert"_s);
        const bool perComponent = config.isFlagSet(u"component"_s);

        // This is only here to avoid an internal compiler error
        const auto& mappingValue = u"mapping"_s;

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

template class VisualisationsBuilder<NodeId>;
template class VisualisationsBuilder<EdgeId>;
