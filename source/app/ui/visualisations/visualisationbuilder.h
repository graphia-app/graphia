#ifndef VISUALISATIONBUILDER_H
#define VISUALISATIONBUILDER_H

#include "visualisationchannel.h"
#include "visualisationalert.h"

#include "shared/graph/grapharray.h"
#include "attributes/attribute.h"

#include <vector>
#include <array>

#include <QtGlobal>

template<typename ElementId>
class VisualisationsBuilder
{
public:
    VisualisationsBuilder(const IGraphArrayClient& graph,
                          const std::vector<ElementId>& elementIds,
                          ElementIdArray<ElementId, ElementVisual>& visuals) :
        _graph(&graph), _elementIds(&elementIds), _visuals(&visuals)
    {}

private:
    const IGraphArrayClient* _graph;
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
        _applications[1].at(index)._array.set(elementId, oldVisual._color != visual._color);
        _applications[2].at(index)._array.set(elementId, oldVisual._text  != visual._text);
    }

public:
    void findOverrideAlerts(VisualisationAlertsMap& alerts)
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
                            alerts[iv._index].emplace_back(VisualisationAlertType::Warning,
                                QObject::tr("Partially overriden by subsequent visualisations"));
                        }
                        else
                        {
                            alerts[iv._index].emplace_back(VisualisationAlertType::Error,
                                QObject::tr("Overriden by subsequent visualisations"));
                        }
                    }
                }
            }
        }
    }

    void build(const Attribute& attribute,
               const VisualisationChannel& channel,
               bool invert, int index)
    {
        for(int c = 0; c < NumChannels; c++)
            _applications[c].emplace_back(index, *_graph);

        switch(attribute.valueType())
        {
        case FieldType::Int:
        case FieldType::Float:
        {
            double min, max;
            std::tie(min, max) = attribute.findNumericRange(*_elementIds);

            for(auto elementId : *_elementIds)
            {
                double value = attribute.numericValueOf(elementId);

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

        case FieldType::String:
            for(auto elementId : *_elementIds)
                apply(attribute.stringValueOf(elementId), channel, elementId, _numAppliedVisualisations);
            break;

        default:
            break;
        }

        _numAppliedVisualisations++;
    }
};

#endif // VISUALISATIONBUILDER_H
