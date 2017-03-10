#include "graphmodel.h"
#include "componentmanager.h"

#include "ui/selectionmanager.h"
#include "ui/searchmanager.h"

#include "transform/compoundtransform.h"
#include "transform/filtertransform.h"
#include "transform/edgecontractiontransform.h"
#include "transform/graphtransformconfigparser.h"

#include "ui/visualisations/colorvisualisationchannel.h"
#include "ui/visualisations/sizevisualisationchannel.h"
#include "ui/visualisations/textvisualisationchannel.h"
#include "ui/visualisations/visualisationconfigparser.h"
#include "ui/visualisations/visualisationbuilder.h"

#include "shared/utils/enumreflection.h"
#include "shared/utils/preferences.h"
#include "shared/utils/utils.h"
#include "shared/utils/pair_iterator.h"

#include <QRegularExpression>

#include <utility>

GraphModel::GraphModel(const QString &name, IPlugin* plugin) :
    _graph(),
    _transformedGraph(_graph),
    _nodePositions(_graph),
    _nodeVisuals(_graph),
    _edgeVisuals(_graph),
    _mappedNodeVisuals(_graph),
    _mappedEdgeVisuals(_graph),
    _nodeNames(_graph),
    _name(name),
    _plugin(plugin)
{
    connect(&_transformedGraph, &Graph::nodeRemoved, [this](const Graph*, NodeId nodeId)
    {
       _nodeVisuals[nodeId]._state = VisualFlags::None;
    });
    connect(&_transformedGraph, &Graph::edgeRemoved, [this](const Graph*, EdgeId edgeId)
    {
       _edgeVisuals[edgeId]._state = VisualFlags::None;
    });

    connect(&_graph, &Graph::graphChanged, this, &GraphModel::onMutableGraphChanged, Qt::DirectConnection);

    connect(S(Preferences), &Preferences::preferenceChanged, this, &GraphModel::onPreferenceChanged);

    attribute(tr("Node Degree"))
        .setIntValueFn([this](NodeId nodeId) { return _transformedGraph.nodeById(nodeId).degree(); })
        .setIntMin(0)
        .setDescription(tr("A node's degree is its number of incident edges."));

    attribute(tr("Component Size"))
        .setIntValueFn([this](const IGraphComponent& component) { return component.numNodes(); })
        .setIntMin(1)
        .setDescription(tr("Component Size refers to the number of nodes the component contains."));

    _graphTransformFactories.emplace(tr("Remove Nodes"),      std::make_unique<FilterTransformFactory>(ElementType::Node, false));
    _graphTransformFactories.emplace(tr("Remove Edges"),      std::make_unique<FilterTransformFactory>(ElementType::Edge, false));
    _graphTransformFactories.emplace(tr("Remove Components"), std::make_unique<FilterTransformFactory>(ElementType::Component, false));
    _graphTransformFactories.emplace(tr("Keep Nodes"),        std::make_unique<FilterTransformFactory>(ElementType::Node, true));
    _graphTransformFactories.emplace(tr("Keep Edges"),        std::make_unique<FilterTransformFactory>(ElementType::Edge, true));
    _graphTransformFactories.emplace(tr("Keep Components"),   std::make_unique<FilterTransformFactory>(ElementType::Component, true));
    _graphTransformFactories.emplace(tr("Contract Edges"),    std::make_unique<EdgeContractionTransformFactory>());

    _visualisationChannels.emplace(tr("Colour"), std::make_unique<ColorVisualisationChannel>());
    _visualisationChannels.emplace(tr("Size"), std::make_unique<SizeVisualisationChannel>());
    _visualisationChannels.emplace(tr("Text"), std::make_unique<TextVisualisationChannel>());
}

void GraphModel::setNodeName(NodeId nodeId, const QString& name)
{
    _nodeNames[nodeId] = name;
    updateVisuals();
}

bool GraphModel::graphTransformIsValid(const QString& transform) const
{
    GraphTransformConfigParser p;
    bool parsed = p.parse(transform);

    if(parsed)
    {
        const auto& graphTransformConfig = p.result();

        if(!u::contains(_graphTransformFactories, graphTransformConfig._action))
            return false;

        auto& factory = _graphTransformFactories.at(graphTransformConfig._action);
        auto graphTransform = factory->create(graphTransformConfig, _attributes);

        return graphTransform != nullptr;
    }

    return false;
}

void GraphModel::buildTransforms(const QStringList& transforms)
{
    auto compoundTransform = std::make_unique<CompoundTransform>();

    for(const auto& transform : transforms)
    {
        GraphTransformConfigParser graphTransformConfigParser;

        if(!graphTransformConfigParser.parse(transform))
            continue;

        const auto& graphTransformConfig = graphTransformConfigParser.result();
        const auto& action = graphTransformConfig._action;

        if(graphTransformConfig.isFlagSet("disabled"))
            continue;

        if(!u::contains(_graphTransformFactories, action))
            continue;

        auto& factory = _graphTransformFactories.at(action);
        auto graphTransform = factory->create(graphTransformConfig, _attributes);

        if(graphTransform)
        {
            graphTransform->setRepeating(graphTransformConfig.isFlagSet("repeating"));
            compoundTransform->addTransform(std::move(graphTransform));
        }
    }

    _transformedGraph.enableAutoRebuild();
    _transformedGraph.setTransform(std::move(compoundTransform));
}

QStringList GraphModel::availableTransformNames() const
{
    QStringList stringList;

    for(auto& t : _graphTransformFactories)
    {
        if(!availableAttributesFor(t.first).isEmpty())
            stringList.append(t.first);
    }

    return stringList;
}

QStringList GraphModel::availableAttributes(ElementType elementTypes, ValueType valueTypes) const
{
    QStringList stringList;
    auto requestedElementTypes = static_cast<int>(elementTypes);
    auto requestedValueTypes = static_cast<int>(valueTypes);

    for(auto& f : _attributes)
    {
        auto attributeElementType = static_cast<int>(f.second.elementType());
        auto attributeValueType = static_cast<int>(f.second.valueType());

        if(!(attributeElementType & requestedElementTypes))
            continue;

        if(!(attributeValueType & requestedValueTypes))
            continue;

        stringList.append(f.first);
    }

    return stringList;
}

QStringList GraphModel::availableAttributesFor(const QString& transformName) const
{
    if(!transformName.isEmpty())
    {
        auto elementType = _graphTransformFactories.at(transformName)->elementType();
        return availableAttributes(elementType, ValueType::All);
    }

    return {};
}

QStringList GraphModel::avaliableConditionFnOps(const QString& attributeName) const
{
    if(attributeName.isEmpty() || !u::contains(_attributes, attributeName))
        return {};

    return GraphTransformConfigParser::ops(_attributes.at(attributeName).valueType());
}

bool GraphModel::visualisationIsValid(const QString& visualisation) const
{
    VisualisationConfigParser p;
    bool parsed = p.parse(visualisation);

    if(parsed)
    {
        const auto& visualisationConfig = p.result();

        if(!u::contains(_attributes, visualisationConfig._attributeName))
            return false;

        if(!u::contains(_visualisationChannels, visualisationConfig._channelName))
            return false;

        return true;
    }

    return false;
}

void GraphModel::buildVisualisations(const QStringList& visualisations)
{
    _mappedNodeVisuals.resetElements();
    _mappedEdgeVisuals.resetElements();
    _visualisationAlertsMap.clear();

    VisualisationsBuilder<NodeId> nodeVisualisationsBuilder(_graph, graph().nodeIds(), _mappedNodeVisuals);
    VisualisationsBuilder<EdgeId> edgeVisualisationsBuilder(_graph, graph().edgeIds(), _mappedEdgeVisuals);

    for(int index = 0; index < visualisations.size(); index++)
    {
        auto& visualisation = visualisations.at(index);

        VisualisationConfigParser visualisationConfigParser;

        if(!visualisationConfigParser.parse(visualisation))
            continue;

        const auto& visualisationConfig = visualisationConfigParser.result();

        if(visualisationConfig.isFlagSet("disabled"))
            continue;

        const auto& attributeName = visualisationConfig._attributeName;
        const auto& channelName = visualisationConfig._channelName;
        bool invert = visualisationConfig.isFlagSet("invert");

        if(!u::contains(_attributes, attributeName))
        {
            _visualisationAlertsMap[index].emplace_back(VisualisationAlertType::Error,
                tr("Attribute doesn't exist"));
            continue;
        }

        if(!u::contains(_visualisationChannels, channelName))
            continue;

        auto& attribute = attributeByName(attributeName);
        auto& channel = _visualisationChannels.at(channelName);

        if(!channel->supports(attribute.valueType()))
        {
            _visualisationAlertsMap[index].emplace_back(VisualisationAlertType::Error,
                tr("Visualisation doesn't support attribute type"));
            continue;
        }

        switch(attribute.elementType())
        {
        case ElementType::Node:
            nodeVisualisationsBuilder.build(attribute, *channel.get(), invert, index);
            break;

        case ElementType::Edge:
            edgeVisualisationsBuilder.build(attribute, *channel.get(), invert, index);
            break;

        default:
            break;
        }
    }

    nodeVisualisationsBuilder.findOverrideAlerts(_visualisationAlertsMap);
    edgeVisualisationsBuilder.findOverrideAlerts(_visualisationAlertsMap);

    updateVisuals();
}

QStringList GraphModel::availableVisualisationChannelNames(const QString& attributeName) const
{
    QStringList stringList;

    auto& attribute = attributeByName(attributeName);

    for(auto& t : _visualisationChannels)
    {
        if(t.second->supports(attribute.valueType()))
            stringList.append(t.first);
    }

    return stringList;
}

QString GraphModel::visualisationDescription(const QString& attributeName, const QString& channelName) const
{
    if(!u::contains(_attributes, attributeName) || !u::contains(_visualisationChannels, channelName))
        return {};

    auto& attribute = attributeByName(attributeName);
    auto& channel = _visualisationChannels.at(channelName);

    if(!channel->supports(attribute.valueType()))
        return tr("This visualisation channel is not supported for the attribute type.");

    return channel->description(attribute.elementType(), attribute.valueType());
}

std::vector<VisualisationAlert> GraphModel::visualisationAlertsAtIndex(int index) const
{
    if(u::contains(_visualisationAlertsMap, index))
        return _visualisationAlertsMap.at(index);

    return {};
}

QStringList GraphModel::attributeNames(ElementType elementType) const
{
    QStringList attributeNames;

    for(auto& attribute : _attributes)
    {
        if(attribute.second.elementType() == elementType)
            attributeNames.append(attribute.first);
    }

    return attributeNames;
}

IAttribute& GraphModel::attribute(const QString& name)
{
    return _attributes[name];
}

const Attribute& GraphModel::attributeByName(const QString& name) const
{
    static Attribute nullAttribute;
    return u::contains(_attributes, name) ? _attributes.at(name) : nullAttribute;
}

void GraphModel::enableVisualUpdates()
{
    _visualUpdatesEnabled = true;
    updateVisuals();
}

static float mappedSize(float min, float max, float user, float mapped)
{
    // The fraction of the mapped value that contributes to the final value
    const float mappedRange = 0.5f;

    auto normalised = u::normalise(min, max, user);
    auto out = (mapped * mappedRange) + (normalised * (1.0f - mappedRange));

    return min + (out * (max - min));
}

void GraphModel::updateVisuals(const SelectionManager* selectionManager, const SearchManager* searchManager)
{
    if(!_visualUpdatesEnabled)
        return;

    emit visualsWillChange();

    auto nodeColor      = u::pref("visuals/defaultNodeColor").value<QColor>();
    auto edgeColor      = u::pref("visuals/defaultEdgeColor").value<QColor>();
    auto multiColor     = u::pref("visuals/multiElementColor").value<QColor>();
    auto nodeSize       = u::pref("visuals/defaultNodeSize").toFloat();
    auto minNodeSize    = u::minPref("visuals/defaultNodeSize").toFloat();
    auto maxNodeSize    = u::maxPref("visuals/defaultNodeSize").toFloat();
    auto edgeSize       = u::pref("visuals/defaultEdgeSize").toFloat();
    auto minEdgeSize    = u::minPref("visuals/defaultEdgeSize").toFloat();
    auto maxEdgeSize    = u::maxPref("visuals/defaultEdgeSize").toFloat();

    if(searchManager != nullptr)
    {
        // Clear all edge NotFound flags as we can't know what to change unless
        // we have the previous search state to hand
        for(auto edgeId : graph().edgeIds())
            _edgeVisuals[edgeId]._state.setFlag(VisualFlags::NotFound, false);
    }

    for(auto nodeId : graph().nodeIds())
    {
        // Size
        if(_mappedNodeVisuals[nodeId]._size >= 0.0f)
        {
            _nodeVisuals[nodeId]._size = mappedSize(minNodeSize, maxNodeSize, nodeSize,
                                         _mappedNodeVisuals[nodeId]._size);
        }
        else
            _nodeVisuals[nodeId]._size = nodeSize;

        // Color
        if(!_mappedNodeVisuals[nodeId]._color.isValid())
        {
            _nodeVisuals[nodeId]._color = graph().typeOf(nodeId) == NodeIdDistinctSetCollection::Type::Not ?
                nodeColor : multiColor;
        }
        else
            _nodeVisuals[nodeId]._color = _mappedNodeVisuals[nodeId]._color;

        // Text
        if(!_mappedNodeVisuals[nodeId]._text.isEmpty())
            _nodeVisuals[nodeId]._text = _mappedNodeVisuals[nodeId]._text;
        else
            _nodeVisuals[nodeId]._text = nodeName(nodeId);

        if(selectionManager != nullptr)
        {
            _nodeVisuals[nodeId]._state.setFlag(VisualFlags::Selected,
                                                selectionManager->nodeIsSelected(nodeId));
        }

        if(searchManager != nullptr)
        {
            if(!searchManager->foundNodeIds().empty() && !searchManager->nodeWasFound(nodeId))
            {
                _nodeVisuals[nodeId]._state.setFlag(VisualFlags::NotFound, true);

                for(auto edgeId : graph().edgeIdsForNodeId(nodeId))
                    _edgeVisuals[edgeId]._state.setFlag(VisualFlags::NotFound, true);
            }
            else
                _nodeVisuals[nodeId]._state.setFlag(VisualFlags::NotFound, false);
        }
    }

    for(auto edgeId : graph().edgeIds())
    {
        // Size
        if(_mappedEdgeVisuals[edgeId]._size >= 0.0f)
        {
            _edgeVisuals[edgeId]._size = mappedSize(minEdgeSize, maxEdgeSize, edgeSize,
                                         _mappedEdgeVisuals[edgeId]._size);
        }
        else
            _edgeVisuals[edgeId]._size = edgeSize;

        // Restrict edgeSize to be no larger than the source or target size
        auto& edge = graph().edgeById(edgeId);
        auto minEdgeNodesSize = std::min(_nodeVisuals[edge.sourceId()]._size,
                                         _nodeVisuals[edge.targetId()]._size);
        _edgeVisuals[edgeId]._size = std::min(_edgeVisuals[edgeId]._size, minEdgeNodesSize);

        // Color
        if(!_mappedEdgeVisuals[edgeId]._color.isValid())
        {
            _edgeVisuals[edgeId]._color = graph().typeOf(edgeId) == EdgeIdDistinctSetCollection::Type::Not ?
                edgeColor : multiColor;
        }
        else
            _edgeVisuals[edgeId]._color = _mappedEdgeVisuals[edgeId]._color;

        // Text
        if(!_mappedEdgeVisuals[edgeId]._text.isEmpty())
            _edgeVisuals[edgeId]._text = _mappedEdgeVisuals[edgeId]._text;
        else
            _edgeVisuals[edgeId]._text.clear();
    }

    emit visualsChanged();
}

void GraphModel::onSelectionChanged(const SelectionManager* selectionManager)
{
    updateVisuals(selectionManager, nullptr);
}

void GraphModel::onFoundNodeIdsChanged(const SearchManager* searchManager)
{
    updateVisuals(nullptr, searchManager);
}

void GraphModel::onPreferenceChanged(const QString&, const QVariant&)
{
    updateVisuals();
}

void GraphModel::onMutableGraphChanged(const IGraph* graph)
{
    for(auto& attribute : make_value_wrapper(_attributes))
    {
        if(attribute.elementType() == ElementType::Node)
            attribute.autoSetRange(graph->nodeIds());
        else if(attribute.elementType() == ElementType::Edge)
            attribute.autoSetRange(graph->edgeIds());
    }
}
