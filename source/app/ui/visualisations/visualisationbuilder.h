#ifndef VISUALISATIONBUILDER_H
#define VISUALISATIONBUILDER_H

#include "visualisationchannel.h"
#include "visualisationinfo.h"

#include "graph/graph.h"
#include "shared/graph/grapharray.h"
#include "attributes/attribute.h"

#include <vector>
#include <array>

#include <QtGlobal>

template<typename ElementId>
class VisualisationsBuilder
{
public:
    VisualisationsBuilder(const Graph& graph,
                          const std::vector<ElementId>& elementIds,
                          ElementIdArray<ElementId, ElementVisual>& visuals) :
        _graph(&graph), _elementIds(&elementIds), _visuals(&visuals)
    {}

private:
    const Graph* _graph;
    const std::vector<ElementId>* _elementIds;
    ElementIdArray<ElementId, ElementVisual>* _visuals;
    int _numAppliedVisualisations = 0;

    static const int NumChannels = 3;

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
               ElementId elementId, int index)
    {
        auto& visual = (*_visuals)[elementId];
        auto oldVisual = visual;
        channel.apply(value, visual);

        Q_ASSERT(index < static_cast<int>(_applications[0].size()));

        // Must not exceed NumChannels
        _applications[0].at(index)._array.set(elementId, oldVisual._size  != visual._size);
        _applications[1].at(index)._array.set(elementId, oldVisual._innerColor != visual._innerColor ||
                oldVisual._outerColor != visual._outerColor);
        _applications[2].at(index)._array.set(elementId, oldVisual._text  != visual._text);
    }

public:
    void findOverrideAlerts(VisualisationInfosMap& infos)
    {
        for(int c = 0; c < NumChannels; c++)
        {
            for(int i = 0; i < _numAppliedVisualisations - 1; i++)
            {
                auto& iv = _applications[c].at(i);

                for(int j = i + 1; j < _numAppliedVisualisations; j++)
                {
                    auto& jv = _applications[c].at(j);
                    int bothSet = 0, sourceSet = 0;

                    for(auto elementId : *_elementIds)
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

    void build(const Attribute* attribute,
               const VisualisationChannel& channel,
               bool invert, int index, VisualisationInfo& visualisationInfo)
    {
        for(int c = 0; c < NumChannels; c++)
            _applications[c].emplace_back(index, *_graph);

        switch(attribute->valueType())
        {
        case ValueType::Int:
        case ValueType::Float:
        {
            double min, max;
            std::tie(min, max) = attribute->findRangeforElements(*_elementIds,
            [this](const Attribute& _attribute, ElementId elementId)
            {
                return _attribute.testFlag(AttributeFlag::IgnoreTails) &&
                    _graph->typeOf(elementId) == MultiElementType::Tail;
            });

            visualisationInfo.setMin(min);
            visualisationInfo.setMax(max);

            if(min == max)
            {
                visualisationInfo.addAlert(AlertType::Warning, QObject::tr("No Numeric Range To Map To"));
                return;
            }

            for(auto elementId : *_elementIds)
            {
                if(attribute->testFlag(AttributeFlag::IgnoreTails) &&
                   _graph->typeOf(elementId) == MultiElementType::Tail)
                {
                    continue;
                }

                double value = attribute->numericValueOf(elementId);

                if(channel.requiresNormalisedValue())
                {
                    value = u::normalise(min, max, value);

                    if(invert)
                        value = 1.0 - value;
                }
                else
                {
                    if(invert)
                        value = ((max - min) - (value - min)) + min;
                }

                apply(value, channel, elementId, _numAppliedVisualisations);
            }
            break;
        }

        case ValueType::String:
            for(auto elementId : *_elementIds)
            {
                if(attribute->testFlag(AttributeFlag::IgnoreTails) &&
                   _graph->typeOf(elementId) == MultiElementType::Tail)
                {
                    continue;
                }

                apply(attribute->stringValueOf(elementId), channel, elementId, _numAppliedVisualisations);
            }
            break;

        default:
            break;
        }

        _numAppliedVisualisations++;
    }
};

#endif // VISUALISATIONBUILDER_H
