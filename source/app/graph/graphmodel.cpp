#include "graphmodel.h"
#include "componentmanager.h"

#include "ui/selectionmanager.h"
#include "ui/searchmanager.h"

#include "attributes/attribute.h"

#include "transform/transforms/filtertransform.h"
#include "transform/transforms/edgecontractiontransform.h"
#include "transform/transforms/mcltransform.h"
#include "transform/transforms/pageranktransform.h"
#include "transform/transforms/eccentricitytransform.h"
#include "transform/graphtransformconfigparser.h"

#include "ui/visualisations/colorvisualisationchannel.h"
#include "ui/visualisations/sizevisualisationchannel.h"
#include "ui/visualisations/textvisualisationchannel.h"
#include "ui/visualisations/visualisationconfigparser.h"
#include "ui/visualisations/visualisationbuilder.h"

#include "shared/commands/icommand.h"

#include "shared/utils/enumreflection.h"
#include "shared/utils/preferences.h"
#include "shared/utils/utils.h"
#include "shared/utils/pair_iterator.h"
#include "shared/utils/flags.h"
#include "shared/utils/utils.h"

#include <QRegularExpression>

#include <utility>

GraphModel::GraphModel(QString name, IPlugin* plugin) :
    _graph(),
    _transformedGraph(*this, _graph),
    _nodePositions(_graph),
    _nodeVisuals(_graph),
    _edgeVisuals(_graph),
    _mappedNodeVisuals(_graph),
    _mappedEdgeVisuals(_graph),
    _transformedGraphIsChanging(false),
    _nodeNames(_graph),
    _name(std::move(name)),
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

    connect(&_transformedGraph, &Graph::graphWillChange, this, &GraphModel::onTransformedGraphWillChange, Qt::DirectConnection);
    connect(&_transformedGraph, &Graph::graphChanged, this, &GraphModel::onTransformedGraphChanged, Qt::DirectConnection);

    connect(S(Preferences), &Preferences::preferenceChanged, this, &GraphModel::onPreferenceChanged);

    GraphModel::createAttribute(tr("Node Degree"))
        .setIntValueFn([this](NodeId nodeId) { return _transformedGraph.nodeById(nodeId).degree(); })
        .intRange().setMin(0)
        .setDescription(tr("A node's degree is its number of incident edges."));

    GraphModel::createAttribute(tr("Node Multiplicity"))
        .setIntValueFn([this](NodeId nodeId)
        {
            if(_transformedGraph.typeOf(nodeId) != MultiElementType::Head)
                return 1;

            return _transformedGraph.mergedNodeIdsForNodeId(nodeId).size();
        })
        .intRange().setMin(0)
        .setFlag(AttributeFlag::IgnoreTails)
        .setDescription(tr("A node's multiplicity is how many nodes it represents."));

    GraphModel::createAttribute(tr("Edge Multiplicity"))
        .setIntValueFn([this](EdgeId edgeId)
        {
            if(_transformedGraph.typeOf(edgeId) != MultiElementType::Head)
                return 1;

            return _transformedGraph.mergedEdgeIdsForEdgeId(edgeId).size();
        })
        .intRange().setMin(0)
        .setFlag(AttributeFlag::IgnoreTails)
        .setDescription(tr("An edge's multiplicity is how many edges it represents."));

    GraphModel::createAttribute(tr("Component Size"))
        .setIntValueFn([this](const IGraphComponent& component) { return component.numNodes(); })
        .intRange().setMin(1)
        .setDescription(tr("Component Size refers to the number of nodes the component contains."));

    _graphTransformFactories.emplace(tr("Remove Nodes"),      std::make_unique<FilterTransformFactory>(this, ElementType::Node, false));
    _graphTransformFactories.emplace(tr("Remove Edges"),      std::make_unique<FilterTransformFactory>(this, ElementType::Edge, false));
    _graphTransformFactories.emplace(tr("Remove Components"), std::make_unique<FilterTransformFactory>(this, ElementType::Component, false));
    _graphTransformFactories.emplace(tr("Keep Nodes"),        std::make_unique<FilterTransformFactory>(this, ElementType::Node, true));
    _graphTransformFactories.emplace(tr("Keep Edges"),        std::make_unique<FilterTransformFactory>(this, ElementType::Edge, true));
    _graphTransformFactories.emplace(tr("Keep Components"),   std::make_unique<FilterTransformFactory>(this, ElementType::Component, true));
    _graphTransformFactories.emplace(tr("Contract Edges"),    std::make_unique<EdgeContractionTransformFactory>(this));
    _graphTransformFactories.emplace(tr("MCL Cluster"),       std::make_unique<MCLTransformFactory>(this));
    _graphTransformFactories.emplace(tr("PageRank"),          std::make_unique<PageRankTransformFactory>(this));
    _graphTransformFactories.emplace(tr("Eccentricity"),      std::make_unique<EccentricityTransformFactory>(this));

    _visualisationChannels.emplace(tr("Colour"), std::make_unique<ColorVisualisationChannel>());
    _visualisationChannels.emplace(tr("Size"), std::make_unique<SizeVisualisationChannel>());
    _visualisationChannels.emplace(tr("Text"), std::make_unique<TextVisualisationChannel>());
}

void GraphModel::removeDynamicAttributes()
{
    QStringList dynamicAttributeNames;

    for(const auto& attributePair : _attributes)
    {
        if(attributePair.second.testFlag(AttributeFlag::Dynamic))
            dynamicAttributeNames.append(attributePair.first);
    }

    for(const auto& attributeName : dynamicAttributeNames)
        _attributes.erase(attributeName);
}

QString GraphModel::normalisedAttributeName(QString attribute) const
{
    while(u::contains(_attributes, attribute))
    {
        int number = 1;

        // The attribute name is already used, so generate a new one
        QRegularExpression re(R"(^(.*)\((\d+)\)$)");
        auto match = re.match(attribute);
        if(match.hasMatch())
        {
            attribute = match.captured(1);
            number = match.captured(2).toInt() + 1;
        }

        attribute = QString("%1(%2)").arg(attribute).arg(number);
    }

    return attribute;
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

        if(factory->requiresCondition() && !graphTransformConfig.hasCondition())
            return false;

        auto graphTransform = factory->create(graphTransformConfig);

        return graphTransform != nullptr;
    }

    return false;
}

void GraphModel::buildTransforms(const QStringList& transforms, ICommand* command)
{
    _transformedGraph.clearTransforms();
    _transformedGraph.setCommand(command);
    _transformInfos.clear();
    for(int index = 0; index < transforms.size(); index++)
    {
        const auto& transform = transforms.at(index);
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
        auto graphTransform = factory->create(graphTransformConfig);

        Q_ASSERT(graphTransform != nullptr);
        if(graphTransform != nullptr)
        {
            graphTransform->setConfig(graphTransformConfig);
            graphTransform->setRepeating(graphTransformConfig.isFlagSet("repeating"));
            graphTransform->setInfo(&_transformInfos[index]);
            _transformedGraph.addTransform(std::move(graphTransform));
        }
    }

    _transformedGraph.enableAutoRebuild();
    _transformedGraph.setCommand(nullptr);
}

void GraphModel::cancelTransformBuild()
{
    _transformedGraph.cancelRebuild();
}

QStringList GraphModel::availableTransformNames() const
{
    QStringList stringList;
    stringList.reserve(static_cast<int>(_graphTransformFactories.size()));

    for(auto& t : _graphTransformFactories)
    {
        auto elementType = _graphTransformFactories.at(t.first)->elementType();
        bool attributesAvailable = !availableAttributes(elementType, ValueType::All).isEmpty();

        if(elementType == ElementType::None || attributesAvailable)
            stringList.append(t.first);
    }

    return stringList;
}

const GraphTransformFactory* GraphModel::transformFactory(const QString& transformName) const
{
    if(!transformName.isEmpty() && u::contains(_graphTransformFactories, transformName))
        return _graphTransformFactories.at(transformName).get();

    return nullptr;
}

QStringList GraphModel::availableAttributes(ElementType elementTypes, ValueType valueTypes) const
{
    QStringList stringList;

    for(auto& attribute : _attributes)
    {
        if(!Flags<ElementType>(elementTypes).test(attribute.second.elementType()))
            continue;

        if(!Flags<ValueType>(valueTypes).test(attribute.second.valueType()))
            continue;

        stringList.append(attribute.first);
    }

    return stringList;
}

QStringList GraphModel::avaliableConditionFnOps(const QString& attributeName) const
{
    QStringList ops;

    if(attributeName.isEmpty() || !u::contains(_attributes, attributeName))
        return ops;

    const auto& attribute = _attributes.at(attributeName);
    ops.append(GraphTransformConfigParser::ops(attribute.valueType()));

    if(attribute.hasMissingValues())
        ops.append(GraphTransformConfigParser::opToString(ConditionFnOp::Unary::HasValue));

    return ops;
}

bool GraphModel::hasTransformInfo() const
{
    return !_transformInfos.empty();
}

const TransformInfo& GraphModel::transformInfoAtIndex(int index) const
{
    static TransformInfo nullTransformInfo;

    if(u::contains(_transformInfos, index))
        return _transformInfos.at(index);

    return nullTransformInfo;
}

bool GraphModel::opIsUnary(const QString& op) const
{
    return GraphTransformConfigParser::opIsUnary(op);
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
    clearVisualisationInfos();

    VisualisationsBuilder<NodeId> nodeVisualisationsBuilder(graph(), graph().nodeIds(), _mappedNodeVisuals);
    VisualisationsBuilder<EdgeId> edgeVisualisationsBuilder(graph(), graph().edgeIds(), _mappedEdgeVisuals);

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
            _visualisationInfos[index].addAlert(AlertType::Error,
                tr("Attribute doesn't exist"));
            continue;
        }

        if(!u::contains(_visualisationChannels, channelName))
            continue;

        auto attribute = attributeValueByName(attributeName);
        auto& channel = _visualisationChannels.at(channelName);

        if(!channel->supports(attribute.valueType()))
        {
            _visualisationInfos[index].addAlert(AlertType::Error,
                tr("Visualisation doesn't support attribute type"));
            continue;
        }

        for(const auto& parameter : visualisationConfig._parameters)
            channel->setParameter(parameter._name, parameter.valueAsString());

        switch(attribute.elementType())
        {
        case ElementType::Node:
            nodeVisualisationsBuilder.build(attribute, *channel.get(), invert, index, _visualisationInfos[index]);
            break;

        case ElementType::Edge:
            edgeVisualisationsBuilder.build(attribute, *channel.get(), invert, index, _visualisationInfos[index]);
            break;

        default:
            break;
        }
    }

    nodeVisualisationsBuilder.findOverrideAlerts(_visualisationInfos);
    edgeVisualisationsBuilder.findOverrideAlerts(_visualisationInfos);

    updateVisuals();
}

QStringList GraphModel::availableVisualisationChannelNames(ValueType valueType) const
{
    QStringList stringList;

    for(auto& t : _visualisationChannels)
    {
        if(t.second->supports(valueType))
            stringList.append(t.first);
    }

    return stringList;
}

QString GraphModel::visualisationDescription(const QString& attributeName, const QString& channelName) const
{
    if(!u::contains(_attributes, attributeName) || !u::contains(_visualisationChannels, channelName))
        return {};

    auto attribute = attributeValueByName(attributeName);
    auto& channel = _visualisationChannels.at(channelName);

    if(!channel->supports(attribute.valueType()))
        return tr("This visualisation channel is not supported for the attribute type.");

    return channel->description(attribute.elementType(), attribute.valueType());
}

void GraphModel::clearVisualisationInfos()
{
    _visualisationInfos.clear();
}

bool GraphModel::hasVisualisationInfo() const
{
    return !_visualisationInfos.empty();
}

const VisualisationInfo& GraphModel::visualisationInfoAtIndex(int index) const
{
    static VisualisationInfo nullVisualisationInfo;

    if(u::contains(_visualisationInfos, index))
        return _visualisationInfos.at(index);

    return nullVisualisationInfo;
}

QVariantMap GraphModel::visualisationDefaultParameters(ValueType valueType, const QString& channelName) const
{
    if(!u::contains(_visualisationChannels, channelName))
        return {};

    auto& channel = _visualisationChannels.at(channelName);

    return channel->defaultParameters(valueType);
}

std::vector<QString> GraphModel::attributeNames(ElementType elementType) const
{
    std::vector<QString> attributeNames;

    for(auto& attribute : _attributes)
    {
        if(Flags<ElementType>(elementType).test(attribute.second.elementType()))
            attributeNames.emplace_back(attribute.first);
    }

    return attributeNames;
}

void GraphModel::patchAttributeNames(QStringList& transforms, QStringList& visualisations) const
{
    if(transforms.empty() || visualisations.empty())
        return;

    std::map<QString, QString> substitutions;

    for(const auto& transform : transforms)
    {
        GraphTransformConfigParser p;

        if(!p.parse(transform))
            continue;

        const auto& graphTransformConfig = p.result();

        if(!u::contains(_graphTransformFactories, graphTransformConfig._action))
            continue;

        auto& factory = _graphTransformFactories.at(graphTransformConfig._action);

        auto newAttributes = u::keysFor(factory->declaredAttributes());

        for(const auto& name : newAttributes)
        {
            auto normalisedName = normalisedAttributeName(name);
            if(normalisedName != name)
                substitutions.emplace(name, normalisedName);
        }
    }

    QStringList patchedVisualisations;
    for(const auto& visualisation : visualisations)
    {
        VisualisationConfigParser p;

        if(!p.parse(visualisation))
            continue;

        auto visualisationConfig = p.result();

        if(u::contains(substitutions, visualisationConfig._attributeName))
            visualisationConfig._attributeName = substitutions[visualisationConfig._attributeName];

        patchedVisualisations.append(visualisationConfig.asString());
    }

    visualisations = patchedVisualisations;
}

Attribute& GraphModel::createAttribute(QString name)
{
    name = normalisedAttributeName(name);
    Attribute& attribute = _attributes[name];

    // If we're creating an attribute during the graph transform, it's
    // a dynamically created attribute rather than a persistent one,
    // so mark it as such
    if(_transformedGraphIsChanging)
        attribute.setFlag(AttributeFlag::Dynamic);

    return attribute;
}

void GraphModel::addAttributes(const std::map<QString, Attribute>& attributes)
{
    _attributes.insert(attributes.begin(), attributes.end());
}

void GraphModel::removeAttribute(const QString& name)
{
    if(u::contains(_attributes, name))
        _attributes.erase(name);
}

const Attribute* GraphModel::attributeByName(const QString& name) const
{
    auto attributeName = Attribute::parseAttributeName(name);

    if(!u::contains(_attributes, attributeName._name))
        return nullptr;

    return &_attributes.at(attributeName._name);
}

Attribute GraphModel::attributeValueByName(const QString& name) const
{
    auto attributeName = Attribute::parseAttributeName(name);

    if(!u::contains(_attributes, attributeName._name))
        return {};

    auto attribute = _attributes.at(attributeName._name);

    if(attributeName._type != Attribute::EdgeNodeType::None)
    {
        return Attribute::edgeNodesAttribute(_transformedGraph,
            attribute, attributeName._type);
    }

    return attribute;
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
    auto meIndicators   = u::pref("visuals/showMultiElementIndicators").toBool();

    if(selectionManager != nullptr)
    {
        // Clear all edge Selected flags as we can't know what to change unless
        // we have the previous selection state to hand
        for(auto edgeId : graph().edgeIds())
            _edgeVisuals[edgeId]._state.reset(VisualFlags::Selected);
    }

    if(searchManager != nullptr)
    {
        // Clear all edge NotFound flags as we can't know what to change unless
        // we have the previous search state to hand
        for(auto edgeId : graph().edgeIds())
            _edgeVisuals[edgeId]._state.reset(VisualFlags::NotFound);
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
        if(!_mappedNodeVisuals[nodeId]._outerColor.isValid())
            _nodeVisuals[nodeId]._outerColor = nodeColor;
        else
            _nodeVisuals[nodeId]._outerColor = _mappedNodeVisuals[nodeId]._outerColor;

        _nodeVisuals[nodeId]._innerColor = !meIndicators || graph().typeOf(nodeId) == MultiElementType::Not ?
                    _nodeVisuals[nodeId]._outerColor : multiColor;

        // Text
        if(!_mappedNodeVisuals[nodeId]._text.isEmpty())
            _nodeVisuals[nodeId]._text = _mappedNodeVisuals[nodeId]._text;
        else
            _nodeVisuals[nodeId]._text = nodeName(nodeId);

        if(selectionManager != nullptr)
        {
            auto nodeIsSelected = selectionManager->nodeIsSelected(nodeId);

            _nodeVisuals[nodeId]._state.setState(VisualFlags::Selected, nodeIsSelected);

            if(nodeIsSelected)
            {
                for(auto edgeId : graph().edgeIdsForNodeId(nodeId))
                    _edgeVisuals[edgeId]._state.setState(VisualFlags::Selected, nodeIsSelected);
            }
        }

        if(searchManager != nullptr)
        {
            auto nodeWasFound = !searchManager->foundNodeIds().empty() &&
                    !searchManager->nodeWasFound(nodeId);

            _nodeVisuals[nodeId]._state.setState(VisualFlags::NotFound, nodeWasFound);

            if(nodeWasFound)
            {
                for(auto edgeId : graph().edgeIdsForNodeId(nodeId))
                    _edgeVisuals[edgeId]._state.set(VisualFlags::NotFound);
            }
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
        if(!_mappedEdgeVisuals[edgeId]._outerColor.isValid())
            _edgeVisuals[edgeId]._outerColor = edgeColor;
        else
            _edgeVisuals[edgeId]._outerColor = _mappedEdgeVisuals[edgeId]._outerColor;

        _edgeVisuals[edgeId]._innerColor = !meIndicators || graph().typeOf(edgeId) == MultiElementType::Not ?
            _edgeVisuals[edgeId]._outerColor : multiColor;

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

void GraphModel::onMutableGraphChanged(const Graph* graph)
{
    for(auto& attribute : make_value_wrapper(_attributes))
    {
        if(!attribute.testFlag(AttributeFlag::AutoRangeMutable))
            continue;

        if(attribute.elementType() == ElementType::Node)
            attribute.autoSetRangeForElements(graph->nodeIds());
        else if(attribute.elementType() == ElementType::Edge)
            attribute.autoSetRangeForElements(graph->edgeIds());
    }
}

void GraphModel::onTransformedGraphWillChange(const Graph*)
{
    // Store previous dynamic attributes for comparison
    _previousDynamicAttributeNames.clear();

    for(auto& name : u::keysFor(_attributes))
        _previousDynamicAttributeNames.emplace_back(name);

    removeDynamicAttributes();

    _transformedGraphIsChanging = true;
}

void GraphModel::onTransformedGraphChanged(const Graph* graph)
{
    _transformedGraphIsChanging = false;

    for(auto& attribute : make_value_wrapper(_attributes))
    {
        if(!attribute.testFlag(AttributeFlag::AutoRangeTransformed))
            continue;

        if(attribute.elementType() == ElementType::Node)
            attribute.autoSetRangeForElements(graph->nodeIds());
        else if(attribute.elementType() == ElementType::Edge)
            attribute.autoSetRangeForElements(graph->edgeIds());
    }

    // Compare with previous Dynamic attributes
    // Check for added
    auto addedAttributeNames = u::setDifference(u::keysFor(_attributes), _previousDynamicAttributeNames);
    for(auto& name : addedAttributeNames)
        emit attributeAdded(name);

    // Check for removed
    auto removedAttributeNames = u::setDifference(_previousDynamicAttributeNames, u::keysFor(_attributes));
    for(auto& name : removedAttributeNames)
        emit attributeRemoved(name);
}
