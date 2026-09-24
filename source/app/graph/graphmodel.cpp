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

#include "graphmodel.h"

#include "shared/loading/userelementdata.h"
#include "app/preferences.h"

#include "limitconstants.h"

#include "app/graph/mutablegraph.h"

#include "app/layout/nodepositions.h"

#include "app/ui/document.h"
#include "app/ui/selectionmanager.h"
#include "app/ui/searchmanager.h"

#include "app/transform/transformedgraph.h"
#include "app/transform/transforminfo.h"
#include "app/transform/transforms/filtertransform.h"
#include "app/transform/transforms/edgecontractiontransform.h"
#include "app/transform/transforms/mcltransform.h"
#include "app/transform/transforms/louvaintransform.h"
#include "app/transform/transforms/pageranktransform.h"
#include "app/transform/transforms/eccentricitytransform.h"
#include "app/transform/transforms/betweennesstransform.h"
#include "app/transform/transforms/contractbyattributetransform.h"
#include "app/transform/transforms/separatebyattributetransform.h"
#include "app/transform/transforms/knntransform.h"
#include "app/transform/transforms/percentnntransform.h"
#include "app/transform/transforms/edgereductiontransform.h"
#include "app/transform/transforms/forwardmultielementattributetransform.h"
#include "app/transform/transforms/spanningtreetransform.h"
#include "app/transform/transforms/attributesynthesistransform.h"
#include "app/transform/transforms/combineattributestransform.h"
#include "app/transform/transforms/conditionalattributetransform.h"
#include "app/transform/transforms/averageattributetransform.h"
#include "app/transform/transforms/typecasttransform.h"
#include "app/transform/transforms/removeleavestransform.h"
#include "app/transform/graphtransformconfigparser.h"

#include "app/ui/visualisations/colorvisualisationchannel.h"
#include "app/ui/visualisations/sizevisualisationchannel.h"
#include "app/ui/visualisations/sharedtextvisualisationchannel.h"
#include "app/ui/visualisations/textvisualisationchannel.h"
#include "app/ui/visualisations/textcolorvisualisationchannel.h"
#include "app/ui/visualisations/textsizevisualisationchannel.h"
#include "app/ui/visualisations/visualisationconfigparser.h"
#include "app/ui/visualisations/visualisationbuilder.h"
#include "app/ui/visualisations/visualisationinfo.h"
#include "app/ui/visualisations/elementvisual.h"
#include "app/ui/visualisations/textvisual.h"

#include "shared/plugins/iplugin.h"

#include "shared/utils/container_combine.h"
#include "shared/utils/utils.h"
#include "shared/utils/pair_iterator.h"
#include "shared/utils/flags.h"
#include "shared/utils/string.h"
#include "shared/utils/scopetimer.h"

#include <QColor>
#include <QDebug>
#include <QList>
#include <QRegularExpression>
#include <QMetaType>
#include <QtGlobal>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <mutex>
#include <optional>
#include <set>
#include <shared_mutex>
#include <utility>
#include <vector>

using namespace Qt::Literals::StringLiterals;

using NodeVisuals = NodeArray<ElementVisual>;
using EdgeVisuals = EdgeArray<ElementVisual>;

static void updateTextVisualPositions(TextVisuals& textVisuals, const NodePositions& nodePositions)
{
    // Hold the positions still so that every text visual is placed
    // from the same iteration of the layout
    const std::unique_lock<const NodePositions> lock(nodePositions);

    for(auto& [componentId, textVisuals_] : textVisuals)
    {
        for(auto& textVisual : textVisuals_)
            textVisual.updatePositions(nodePositions);
    }
}

struct PendingVisualUpdate
{
    bool _full = false; // Something that every element's visual depends on has changed
    bool _force = false;

    std::optional<NodeIdSet> _selectedNodeIds;
    std::optional<bool> _nodesMaskActive;
    std::optional<NodeIdSet> _foundNodeIds;
    std::optional<NodeIdSet> _highlightedNodeIds;
};

class GraphModelImpl
{
    friend class GraphModel;
    friend class AttributeChangesTracker;

public:
    explicit GraphModelImpl(GraphModel& graphModel) :
        _transformedGraph(graphModel, _graph),
        _nodePositions(_graph),
        _nodeVisuals(_graph),
        _edgeVisuals(_graph),
        _mappedNodeVisuals(_graph),
        _mappedEdgeVisuals(_graph),
        _nodeNames(_graph)
    {
        _userNodeData.initialise(_graph);
        _userEdgeData.initialise(_graph);
    }

private:
    MutableGraph _graph;
    TransformedGraph _transformedGraph;
    TransformInfosMap _transformInfos;
    NodePositions _nodePositions;

    std::atomic<float> _nodeSize = u::pref(u"visuals/defaultNormalNodeSize"_s).toFloat();
    std::atomic<float> _edgeSize = u::pref(u"visuals/defaultNormalEdgeSize"_s).toFloat();
    std::atomic<float> _textSize = LimitConstants::defaultTextSize();

    NodeVisuals _nodeVisuals;
    EdgeVisuals _edgeVisuals;

    // Held exclusively while anything writes to the visuals, and shared by plugins that
    // read them; the renderer relies on the scene update being disabled instead
    mutable std::shared_mutex _visualsMutex;
    NodeVisuals _mappedNodeVisuals;
    EdgeVisuals _mappedEdgeVisuals;
    QStringList _visualisedAttributeNames;
    VisualisationInfosMap _visualisationInfos;

    TextVisuals _newTextVisuals;
    TextVisuals _textVisuals;
    mutable std::mutex _textVisualsMutex;

    NodeArray<QString> _nodeNames;

    UserNodeData _userNodeData;
    UserEdgeData _userEdgeData;

    PreferencesWatcher _preferencesWatcher;

    std::map<QString, Attribute> _attributes;

    struct AttributeIdentity
    {
        QString _name;
        ValueType _valueType;
        ElementType _elementType;

        AttributeIdentity(const QString& name,
            ValueType valueType, ElementType elementType) :
            _name(name), _valueType(valueType), _elementType(elementType)
        {}

        bool operator==(const AttributeIdentity& other) const
        {
            return _name == other._name &&
                _valueType == other._valueType &&
                _elementType == other._elementType;
        }
    };

    std::vector<AttributeIdentity> _previousAttributeIdentities;

    auto currentAttributeIdentities() const
    {
        std::vector<AttributeIdentity> attributeIdentities;
        attributeIdentities.reserve(_attributes.size());

        for(const auto& attribute : _attributes)
        {
            attributeIdentities.emplace_back(
                attribute.first,
                attribute.second.valueType(),
                attribute.second.elementType());
        }

        return attributeIdentities;
    }

    QStringList _removedDynamicAttributeNames;
    QStringList _changedDynamicAttributeNames;

    void updateSharedAttributeValues(Attribute& attribute) const
    {
        if(!attribute.testFlag(AttributeFlag::FindShared))
            return;

        if(attribute.elementType() == ElementType::Node)
            attribute.updateSharedValuesForElements(_transformedGraph.nodeIds());
        else if(attribute.elementType() == ElementType::Edge)
            attribute.updateSharedValuesForElements(_transformedGraph.edgeIds());
        else if(attribute.elementType() == ElementType::Component)
            qDebug() << "updateSharedAttributeValues called on component attribute";
    }

    std::map<QString, std::unique_ptr<GraphTransformFactory>> _graphTransformFactories;

    std::map<QString, std::unique_ptr<VisualisationChannel>> _visualisationChannels;

    // Slightly hacky state variable that tracks whether or not a edge text
    // visualisation is present. This is required so that we can show a warning
    // to the user, if they have edge text enabled, but do not have a
    // corresponding visualisation
    bool _hasValidEdgeTextVisualisation = false;

    std::atomic_bool _transformRebuildRequired = false;
    std::atomic_bool _visualisationRebuildRequired = false;

    NodeIdSet _selectedNodeIds;
    NodeIdSet _foundNodeIds;
    NodeIdSet _highlightedNodeIds;
    bool _nodesMaskActive = false;

    std::mutex _pendingVisualUpdateMutex;
    PendingVisualUpdate _pendingVisualUpdate;

    template<typename Fn>
    void requestVisualUpdate(Fn&& fn)
    {
        const std::unique_lock<std::mutex> lock(_pendingVisualUpdateMutex);
        fn(_pendingVisualUpdate);
    }

    void requestFullVisualUpdate()
    {
        requestVisualUpdate([](auto& pending) { pending._full = true; });
    }

    std::set<AttributeChangesTracker*> _attributeChangesTrackers;
};

GraphModel::GraphModel(const QString& name, IPlugin* plugin) :
    _(std::make_unique<GraphModelImpl>(*this)),
    _graphTransformsAreChanging(false),
    _name(name),
    _plugin(plugin)
{
    qRegisterMetaType<VisualChangeFlags>("VisualChangeFlags");

    connect(&_->_transformedGraph, &Graph::nodeRemoved, [this](const Graph*, NodeId nodeId)
    {
        const std::unique_lock<std::shared_mutex> lock(_->_visualsMutex);
        _->_nodeVisuals[nodeId]._state = VisualFlags::None;
    });
    connect(&_->_transformedGraph, &Graph::edgeRemoved, [this](const Graph*, EdgeId edgeId)
    {
        const std::unique_lock<std::shared_mutex> lock(_->_visualsMutex);
        _->_edgeVisuals[edgeId]._state = VisualFlags::None;
    });

    connect(&_->_graph, &Graph::graphChanged, this, &GraphModel::onMutableGraphChanged, Qt::DirectConnection);

    connect(&_->_transformedGraph, &Graph::graphWillChange, this, &GraphModel::onTransformedGraphWillChange, Qt::DirectConnection);
    connect(&_->_transformedGraph, &Graph::graphChanged, this, &GraphModel::onTransformedGraphChanged, Qt::DirectConnection);
    connect(&_->_transformedGraph, &TransformedGraph::attributeValuesChanged,
    [this](const QStringList& attributeNames)
    {
        _->_changedDynamicAttributeNames = attributeNames;
    });

    connect(&_->_userNodeData, &UserData::vectorValuesChanged, [this](const QString& vectorName)
    {
        auto attributeName = _->_userNodeData.exposedAttributeName(vectorName);
        if(attributeName.isEmpty())
            return;

        for(auto* tracker : _->_attributeChangesTrackers)
            tracker->change(attributeName);
    });

    connect(&_->_userEdgeData, &UserData::vectorValuesChanged, [this](const QString& vectorName)
    {
        auto attributeName = _->_userEdgeData.exposedAttributeName(vectorName);
        if(attributeName.isEmpty())
            return;

        for(auto* tracker : _->_attributeChangesTrackers)
            tracker->change(attributeName);
    });

    connect(this, &GraphModel::attributesChanged, this, &GraphModel::onAttributesChanged, Qt::DirectConnection);

    connect(&_->_preferencesWatcher, &PreferencesWatcher::preferenceChanged,
        this, &GraphModel::onPreferenceChanged);

    // NOLINTNEXTLINE clang-analyzer-optin.cplusplus.VirtualCall
    createAttribute(tr("Node Degree"))
        .setIntValueFn([this](NodeId nodeId) { return _->_transformedGraph.nodeById(nodeId).degree(); })
        .intRange().setMin(0)
        .setDescription(tr("A node's degree is its number of incident edges."));

    if(directed())
    {
        // NOLINTNEXTLINE clang-analyzer-optin.cplusplus.VirtualCall
        createAttribute(tr("Node In Degree"))
            .setIntValueFn([this](NodeId nodeId) { return _->_transformedGraph.nodeById(nodeId).inDegree(); })
            .intRange().setMin(0)
            .setDescription(tr("A node's in degree is its number of inbound edges."));

        // NOLINTNEXTLINE clang-analyzer-optin.cplusplus.VirtualCall
        createAttribute(tr("Node Out Degree"))
            .setIntValueFn([this](NodeId nodeId) { return _->_transformedGraph.nodeById(nodeId).outDegree(); })
            .intRange().setMin(0)
            .setDescription(tr("A node's out degree is its number of outbound edges."));
    }

    // NOLINTNEXTLINE clang-analyzer-optin.cplusplus.VirtualCall
    createAttribute(tr("Node Multiplicity"))
        .setIntValueFn([this](NodeId nodeId) { return _->_transformedGraph.multiplicityOf(nodeId); })
        .intRange().setMin(0)
        .setDescription(tr("A node's multiplicity is how many nodes it represents."));

    // NOLINTNEXTLINE clang-analyzer-optin.cplusplus.VirtualCall
    createAttribute(tr("Edge Multiplicity"))
        .setIntValueFn([this](EdgeId edgeId) { return _->_transformedGraph.multiplicityOf(edgeId); })
        .intRange().setMin(0)
        .setDescription(tr("An edge's multiplicity is how many edges it represents."));

    // NOLINTNEXTLINE clang-analyzer-optin.cplusplus.VirtualCall
    createAttribute(tr("Component Size"))
        .setIntValueFn([](const IGraphComponent& component) { return static_cast<int>(component.numNodes()); })
        .intRange().setMin(1)
        .setDescription(tr("Component Size refers to the number of nodes the component contains."));

    // NOLINTNEXTLINE clang-analyzer-optin.cplusplus.VirtualCall
    createAttribute(tr("Node Component Identifier"))
        .setStringValueFn([this](NodeId nodeId)
        {
            return u"Component %1"_s.arg(static_cast<int>(_->_transformedGraph.componentIdOfNode(nodeId) + 1));
        })
        .setDescription(tr("A node's component identifier indicates which component it is part of."))
        .setFlag(AttributeFlag::DisableDuringTransform);

    // NOLINTNEXTLINE clang-analyzer-optin.cplusplus.VirtualCall
    createAttribute(tr("Edge Component Identifier"))
        .setStringValueFn([this](EdgeId edgeId)
        {
            return u"Component %1"_s.arg(static_cast<int>(_->_transformedGraph.componentIdOfEdge(edgeId) + 1));
        })
        .setDescription(tr("An edge's component identifier indicates which component it is part of."))
        .setFlag(AttributeFlag::DisableDuringTransform);

    _->_graphTransformFactories.emplace(tr("Remove Nodes"),             std::make_unique<FilterTransformFactory>(this, ElementType::Node, false));
    _->_graphTransformFactories.emplace(tr("Remove Edges"),             std::make_unique<FilterTransformFactory>(this, ElementType::Edge, false));
    _->_graphTransformFactories.emplace(tr("Remove Components"),        std::make_unique<FilterTransformFactory>(this, ElementType::Component, false));
    _->_graphTransformFactories.emplace(tr("Keep Nodes"),               std::make_unique<FilterTransformFactory>(this, ElementType::Node, true));
    _->_graphTransformFactories.emplace(tr("Keep Edges"),               std::make_unique<FilterTransformFactory>(this, ElementType::Edge, true));
    _->_graphTransformFactories.emplace(tr("Keep Components"),          std::make_unique<FilterTransformFactory>(this, ElementType::Component, true));
    _->_graphTransformFactories.emplace(tr("Contract Edges"),           std::make_unique<EdgeContractionTransformFactory>(this));
    _->_graphTransformFactories.emplace(tr("MCL Cluster"),              std::make_unique<MCLTransformFactory>(this));
    _->_graphTransformFactories.emplace(tr("Louvain Cluster"),          std::make_unique<LouvainTransformFactory>(this));
    _->_graphTransformFactories.emplace(tr("Weighted Louvain Cluster"), std::make_unique<WeightedLouvainTransformFactory>(this));
    _->_graphTransformFactories.emplace(tr("PageRank"),                 std::make_unique<PageRankTransformFactory>(this));
    _->_graphTransformFactories.emplace(tr("Eccentricity"),             std::make_unique<EccentricityTransformFactory>(this));
    _->_graphTransformFactories.emplace(tr("Betweenness"),              std::make_unique<BetweennessTransformFactory>(this));
    _->_graphTransformFactories.emplace(tr("Contract By Attribute"),    std::make_unique<ContractByAttributeTransformFactory>(this));
    _->_graphTransformFactories.emplace(tr("Separate By Attribute"),    std::make_unique<SeparateByAttributeTransformFactory>(this));
    _->_graphTransformFactories.emplace(tr("Boolean Node Attribute"),   std::make_unique<ConditionalAttributeTransformFactory>(this, ElementType::Node));
    _->_graphTransformFactories.emplace(tr("Boolean Edge Attribute"),   std::make_unique<ConditionalAttributeTransformFactory>(this, ElementType::Edge));
    _->_graphTransformFactories.emplace(tr("k-NN"),                     std::make_unique<KNNTransformFactory>(this));
    _->_graphTransformFactories.emplace(tr("%-NN"),                     std::make_unique<PercentNNTransformFactory>(this));
    _->_graphTransformFactories.emplace(tr("Edge Reduction"),           std::make_unique<EdgeReductionTransformFactory>(this));
    _->_graphTransformFactories.emplace(tr("Spanning Forest"),          std::make_unique<SpanningTreeTransformFactory>(this));
    _->_graphTransformFactories.emplace(tr("Attribute Synthesis"),      std::make_unique<AttributeSynthesisTransformFactory>(this));
    _->_graphTransformFactories.emplace(tr("Combine Attributes"),       std::make_unique<CombineAttributesTransformFactory>(this));
    _->_graphTransformFactories.emplace(tr("Forward Attribute"),        std::make_unique<ForwardMultiElementAttributeTransformFactory>(this));
    _->_graphTransformFactories.emplace(tr("Average Attribute"),        std::make_unique<AverageAttributeTransformFactory>(this));
    _->_graphTransformFactories.emplace(tr("Type Cast"),                std::make_unique<TypeCastTransformFactory>(this));
    _->_graphTransformFactories.emplace(tr("Remove Leaves"),            std::make_unique<RemoveLeavesTransformFactory>(this));
    _->_graphTransformFactories.emplace(tr("Remove Branches"),          std::make_unique<RemoveBranchesTransformFactory>(this));

    _->_visualisationChannels.emplace(tr("Colour"), std::make_unique<ColorVisualisationChannel>());
    _->_visualisationChannels.emplace(tr("Size"), std::make_unique<SizeVisualisationChannel>());
    _->_visualisationChannels.emplace(tr("Shared Text"), std::make_unique<SharedTextVisualisationChannel>());
    _->_visualisationChannels.emplace(tr("Text"), std::make_unique<TextVisualisationChannel>());
    _->_visualisationChannels.emplace(tr("Text Colour"), std::make_unique<TextColorVisualisationChannel>());
    _->_visualisationChannels.emplace(tr("Text Size"), std::make_unique<TextSizeVisualisationChannel>());
}

GraphModel::~GraphModel() // NOLINT modernize-use-equals-default
{
    // Only here so that we can have a unique_ptr to GraphModelImpl
}

void GraphModel::removeDynamicAttributes()
{
    QStringList dynamicAttributeNames;

    for(const auto& attributePair : _->_attributes)
    {
        if(attributePair.second.testFlag(AttributeFlag::Dynamic))
            dynamicAttributeNames.append(attributePair.first);
    }

    for(const auto& attributeName : dynamicAttributeNames)
    {
        _->_removedDynamicAttributeNames.append(attributeName);
        _->_attributes.erase(attributeName);
    }
}

QString GraphModel::normalisedAttributeName(QString attribute) const
{
    // Dots in attribute names are disallowed as they conflict with
    // the dot syntax for parameterised attributes
    attribute.replace('.', '_');

    attribute = u::findUniqueName(_->_attributes, attribute);

    return attribute;
}

IMutableGraph& GraphModel::mutableGraphImpl() { return mutableGraph(); }
const IMutableGraph& GraphModel::mutableGraphImpl() const { return mutableGraph(); }
const IGraph& GraphModel::graphImpl() const { return graph(); }


MutableGraph& GraphModel::mutableGraph() { return _->_graph; }
const MutableGraph& GraphModel::mutableGraph() const { return _->_graph; }
const Graph& GraphModel::graph() const { return _->_transformedGraph; }

void GraphModel::setNodeSize(float nodeSize)
{
    _->_nodeSize = nodeSize;
    _->requestFullVisualUpdate();
    scheduleVisualUpdate();
}

void GraphModel::setEdgeSize(float edgeSize)
{
    _->_edgeSize = edgeSize;
    _->requestFullVisualUpdate();
    scheduleVisualUpdate();
}

void GraphModel::setTextSize(float textSize)
{
    _->_textSize = textSize;
    _->requestFullVisualUpdate();
    scheduleVisualUpdate();
}

float GraphModel::nodeSize() const { return _->_nodeSize; }
float GraphModel::edgeSize() const { return _->_edgeSize; }
float GraphModel::textSize() const { return _->_textSize; }

const ElementVisual& GraphModel::nodeVisual(NodeId nodeId) const { return _->_nodeVisuals.at(nodeId); }
const ElementVisual& GraphModel::edgeVisual(EdgeId edgeId) const { return _->_edgeVisuals.at(edgeId); }

void GraphModel::readNodeVisuals(const std::vector<NodeId>& nodeIds,
    const std::function<void(NodeId, const IElementVisual&)>& fn) const
{
    const std::shared_lock<std::shared_mutex> lock(_->_visualsMutex);

    for(auto nodeId : nodeIds)
    {
        if(!nodeId.isNull())
            fn(nodeId, _->_nodeVisuals.at(nodeId));
    }
}

void GraphModel::readEdgeVisuals(const std::vector<EdgeId>& edgeIds,
    const std::function<void(EdgeId, const IElementVisual&)>& fn) const
{
    const std::shared_lock<std::shared_mutex> lock(_->_visualsMutex);

    for(auto edgeId : edgeIds)
    {
        if(!edgeId.isNull())
            fn(edgeId, _->_edgeVisuals.at(edgeId));
    }
}

std::vector<ElementVisual> GraphModel::nodeVisuals(const std::vector<NodeId>& nodeIds) const
{
    std::vector<ElementVisual> visuals;
    visuals.reserve(nodeIds.size());

    for(auto nodeId : nodeIds)
        visuals.emplace_back(nodeVisual(nodeId));

    return visuals;
}

std::vector<ElementVisual> GraphModel::edgeVisuals(const std::vector<EdgeId>& edgeIds) const
{
    std::vector<ElementVisual> visuals;
    visuals.reserve(edgeIds.size());

    for(auto edgeId : edgeIds)
        visuals.emplace_back(edgeVisual(edgeId));

    return visuals;
}

void GraphModel::forEachTextVisual(const std::function<void(const TextVisual&)>& fn) const
{
    const std::unique_lock<std::mutex> lock(_->_textVisualsMutex);

    for(const auto& [componentId, textVisuals] : _->_textVisuals)
    {
        for(const auto& textVisual : textVisuals)
            fn(textVisual);
    }
}

void GraphModel::forEachTextVisual(ComponentId componentId,
    const std::function<void(const TextVisual&)>& fn) const
{
    const std::unique_lock<std::mutex> lock(_->_textVisualsMutex);

    auto it = _->_textVisuals.find(componentId);
    if(it == _->_textVisuals.end())
        return;

    for(const auto& textVisual : it->second)
        fn(textVisual);
}

NodePositions& GraphModel::nodePositions() { return _->_nodePositions; }
const NodePositions& GraphModel::nodePositions() const { return _->_nodePositions; }

const NodeArray<QString>& GraphModel::nodeNames() const { return _->_nodeNames; }
QString GraphModel::nodeName(NodeId nodeId) const { return _->_nodeNames[nodeId]; }
void GraphModel::setNodeName(NodeId nodeId, const QString& name)
{
    _->_nodeNames[nodeId] = name;
    _->requestFullVisualUpdate();
    scheduleVisualUpdate();
}

bool GraphModel::editable() const { return _plugin->editable(); }
bool GraphModel::directed() const { return _plugin->directed(); }

QString GraphModel::pluginName() const { return _plugin->name(); }
int GraphModel::pluginDataVersion() const { return _plugin->dataVersion(); }
QString GraphModel::pluginQmlModule() const { return _plugin->qmlModule(); }

bool GraphModel::graphTransformIsValid(const QString& transform) const
{
    GraphTransformConfigParser p;
    const bool parsed = p.parse(transform, false);

    if(parsed)
    {
        const auto& graphTransformConfig = p.result();

        if(!u::contains(_->_graphTransformFactories, graphTransformConfig._action))
            return false;

        auto& factory = _->_graphTransformFactories.at(graphTransformConfig._action);

        if(factory->requiresCondition() && !graphTransformConfig.hasCondition())
            return false;

        return factory->configIsValid(graphTransformConfig);
    }

    return false;
}

QStringList GraphModel::transformsWithMissingParametersSetToDefault(const QStringList& transforms) const
{
    QStringList transformsWithDefaults;

    for(const auto& transform : transforms)
    {
        GraphTransformConfigParser graphTransformConfigParser;

        if(!graphTransformConfigParser.parse(transform))
            continue;

        auto graphTransformConfig = graphTransformConfigParser.result();

        // This can happen if the file has transforms that the app doesn't
        if(!u::contains(_->_graphTransformFactories, graphTransformConfig._action))
            continue;

        const auto& factory = _->_graphTransformFactories.at(graphTransformConfig._action);

        factory->setMissingParametersToDefault(graphTransformConfig);
        transformsWithDefaults.append(graphTransformConfig.asString());
    }

    return transformsWithDefaults;
}

void GraphModel::buildTransforms(const QStringList& transforms, Progressable* progressable)
{
    _->_transformedGraph.clearTransforms();
    _->_transformInfos.clear();
    for(int index = 0; index < transforms.size(); index++)
    {
        const auto& transform = transforms.at(index);
        GraphTransformConfigParser graphTransformConfigParser;

        if(!graphTransformConfigParser.parse(transform))
            continue;

        const auto& graphTransformConfig = graphTransformConfigParser.result();
        const auto& action = graphTransformConfig._action;

        if(graphTransformConfig.isFlagSet(u"disabled"_s))
            continue;

        if(!u::contains(_->_graphTransformFactories, action))
            continue;

        const auto& factory = _->_graphTransformFactories.at(action);
        auto graphTransform = factory->create(graphTransformConfig);

        Q_ASSERT(graphTransform != nullptr);
        if(graphTransform != nullptr)
        {
            graphTransform->setIndex(index);
            graphTransform->setConfig(graphTransformConfig);
            graphTransform->setRepeating(graphTransformConfig.isFlagSet(u"repeating"_s));
            graphTransform->setInfo(&_->_transformInfos[index]);
            graphTransform->setProgressable(progressable);
            _->_transformedGraph.addTransform(std::move(graphTransform));
        }
    }

    _->_transformedGraph.enableAutoRebuild();
}

void GraphModel::cancelTransformBuild()
{
    _->_transformedGraph.cancelRebuild();
}

QStringList GraphModel::availableTransformNames() const
{
    QStringList stringList;
    stringList.reserve(static_cast<int>(_->_graphTransformFactories.size()));

    for(auto& t : _->_graphTransformFactories)
    {
        auto elementType = _->_graphTransformFactories.at(t.first)->elementType();
        const bool attributesAvailable = !availableAttributeNames(elementType, ValueType::All).isEmpty();

        if(elementType == ElementType::None || attributesAvailable)
            stringList.append(t.first);
    }

    return stringList;
}

const GraphTransformFactory* GraphModel::transformFactory(const QString& transformName) const
{
    if(!transformName.isEmpty() && u::contains(_->_graphTransformFactories, transformName))
        return _->_graphTransformFactories.at(transformName).get();

    return nullptr;
}

QStringList GraphModel::availableAttributeNames(ElementType elementTypes,
    ValueType valueTypes, AttributeFlag skipFlags,
    const QStringList& skipAttributeNames) const
{
    QStringList stringList;

    for(auto& attribute : _->_attributes)
    {
        if(!Flags<ElementType>(elementTypes).test(attribute.second.elementType()))
            continue;

        if(!Flags<ValueType>(valueTypes).test(attribute.second.valueType()))
            continue;

        if(attribute.second.testFlag(skipFlags))
            continue;

        if(skipAttributeNames.contains(attribute.first))
            continue;

        stringList.append(attribute.first);
    }

    return stringList;
}

QStringList GraphModel::avaliableConditionFnOps(const QString& attributeName) const
{
    QStringList ops;

    if(attributeName.isEmpty() || !u::contains(_->_attributes, attributeName))
        return ops;

    const auto& attribute = _->_attributes.at(attributeName);
    ops.append(GraphTransformConfigParser::ops(attribute.valueType()));

    if(attribute.hasMissingValues())
        ops.append(GraphTransformConfigParser::opToString(ConditionFnOp::Unary::HasValue));

    return ops;
}

bool GraphModel::hasTransformInfo() const
{
    return !_->_transformInfos.empty();
}

const TransformInfo& GraphModel::transformInfoAtIndex(int index) const
{
    static const TransformInfo nullTransformInfo;

    if(u::contains(_->_transformInfos, index))
        return _->_transformInfos.at(index);

    return nullTransformInfo;
}

std::vector<QString> GraphModel::addedOrChangedAttributeNamesAtTransformIndex(int index) const
{
    return _->_transformedGraph.addedOrChangedAttributeNamesAtTransformIndex(index);
}

std::vector<QString> GraphModel::addedOrChangedAttributeNamesAtTransformIndexOrLater(int firstIndex) const
{
    std::vector<QString> attributeNames;

    for(int index = firstIndex; index < static_cast<int>(_->_transformedGraph.numTransforms()); index++)
    {
        auto addedOrChangedAttributeNames = addedOrChangedAttributeNamesAtTransformIndex(index);
        attributeNames.insert(attributeNames.end(),
            addedOrChangedAttributeNames.begin(), addedOrChangedAttributeNames.end());
    }

    return attributeNames;
}

ValueType GraphModel::valueTypeOfTransformAttributeName(const QString& attributeName) const
{
    for(auto& t : _->_graphTransformFactories)
    {
        const auto& defaultVisualisations = _->_graphTransformFactories.at(t.first)->defaultVisualisations();

        for(const auto& v : defaultVisualisations)
        {
            if(v._attributeName == attributeName)
                return v._attributeValueType;
        }
    }

    return ValueType::Unknown;
}

bool GraphModel::opIsUnary(const QString& op)
{
    return GraphTransformConfigParser::opIsUnary(op);
}

bool GraphModel::visualisationIsValid(const QString& visualisation) const
{
    VisualisationConfigParser p;
    const bool parsed = p.parse(visualisation, false);

    if(parsed)
    {
        const auto& visualisationConfig = p.result();

        if(!u::contains(_->_visualisationChannels, visualisationConfig._channelName))
            return false;

        return true;
    }

    return false;
}

void GraphModel::buildVisualisations(const QStringList& visualisations)
{
    SCOPE_TIMER_MULTISAMPLES(50)

    _->_visualisationRebuildRequired = false;

    _->_mappedNodeVisuals.resetElements();
    _->_mappedEdgeVisuals.resetElements();
    _->_newTextVisuals.clear();
    clearVisualisationInfos();

    _->_visualisedAttributeNames.clear();
    _->_hasValidEdgeTextVisualisation = false;

    VisualisationsBuilder<NodeId> nodeVisualisationsBuilder(graph(), _->_mappedNodeVisuals);
    VisualisationsBuilder<EdgeId> edgeVisualisationsBuilder(graph(), _->_mappedEdgeVisuals);

    for(int index = 0; index < visualisations.size(); index++)
    {
        const auto& visualisation = visualisations.at(index);

        VisualisationConfigParser visualisationConfigParser;

        if(!visualisationConfigParser.parse(visualisation))
            continue;

        const auto& visualisationConfig = visualisationConfigParser.result();

        if(visualisationConfig.isFlagSet(u"disabled"_s))
            continue;

        const auto& attributeName = visualisationConfig._attributeName;
        const auto& channelName = visualisationConfig._channelName;
        auto& info = _->_visualisationInfos[index];

        // Track referenced attribute names, even if the attributes in question don't exist
        _->_visualisedAttributeNames.append(attributeName);

        if(!attributeExists(attributeName))
        {
            info.addAlert(AlertType::Error, tr("Attribute '%1' doesn't exist").arg(attributeName));
            continue;
        }

        if(!u::contains(_->_visualisationChannels, channelName))
        {
            info.addAlert(AlertType::Error, tr("Visualisation channel '%1' doesn't exist").arg(channelName));
            continue;
        }

        auto attribute = attributeValueByName(attributeName);
        auto& channel = _->_visualisationChannels.at(channelName);

        channel->findErrors(attribute.elementType(), info);

        if(!channel->supports(attribute.valueType()))
        {
            info.addAlert(AlertType::Error, tr("Visualisation doesn't support attribute type"));
            continue;
        }

        channel->reset();

        for(const auto& parameter : visualisationConfig._parameters)
            channel->setParameter(parameter._name, parameter.valueAsString());

        if(attribute.elementType() == ElementType::Edge && channelName == u"Text"_s)
            _->_hasValidEdgeTextVisualisation = true;

        if(attribute.valueType() == ValueType::String)
        {
            auto sharedValues = attribute.sharedValues();
            if(sharedValues.empty())
            {
                if(attribute.elementType() == ElementType::Node)
                    sharedValues = attribute.findSharedValuesForElements(graph().nodeIds());
                else if(attribute.elementType() == ElementType::Edge)
                    sharedValues = attribute.findSharedValuesForElements(graph().edgeIds());
            }

            if(!visualisationConfig.isFlagSet(u"assignByQuantity"_s))
            {
                // Sort in natural order so that e.g. "Thing 1" is always
                // assigned a visualisation before "Thing 2"
                std::ranges::sort(sharedValues,
                [](const auto& a, const auto& b)
                {
                    return u::numericCompare(a._value, b._value) < 0;
                });
            }
            else
            {
                // Shared values should already be sorted at this point,
                // but resort anyway as in that case it'll be cheap and
                // if it's not sorted (for whatever reason), we need it
                // to be sorted
                std::ranges::sort(sharedValues,
                [](const auto& a, const auto& b)
                {
                    if(a._count == b._count)
                        return u::numericCompare(a._value, b._value) < 0;

                    return a._count > b._count;
                });
            }

            for(const auto& sharedValue : sharedValues)
            {
                channel->addValue(sharedValue._value);
                info.addStringValue(sharedValue._value);
            }
        }

        switch(attribute.elementType())
        {
        case ElementType::Node:
            nodeVisualisationsBuilder.build(attribute, *channel, visualisationConfig, index, info);
            break;

        case ElementType::Edge:
            edgeVisualisationsBuilder.build(attribute, *channel, visualisationConfig, index, info);
            break;

        default:
            break;
        }

        auto textVisuals = channel->textVisuals(attributeName, *this, _->_transformedGraph);

        for(const auto& [componentId, textVisual] : textVisuals)
        {
            auto& existingTextVisuals = _->_newTextVisuals[componentId];
            existingTextVisuals.insert(existingTextVisuals.end(), textVisual.begin(), textVisual.end());
        }
    }

    nodeVisualisationsBuilder.findOverrideAlerts(_->_visualisationInfos);
    edgeVisualisationsBuilder.findOverrideAlerts(_->_visualisationInfos);

    _->requestFullVisualUpdate();
    scheduleVisualUpdate();
}

bool GraphModel::transformRebuildRequired() const { return _->_transformRebuildRequired; }
bool GraphModel::visualisationRebuildRequired() const { return _->_visualisationRebuildRequired; }

bool GraphModel::hasValidEdgeTextVisualisation() const
{
    return _->_hasValidEdgeTextVisualisation;
}

QStringList GraphModel::availableVisualisationChannelNames(ElementType elementType, ValueType valueType) const
{
    QStringList stringList;

    for(const auto& [channelName, channel] : _->_visualisationChannels)
    {
        if(channel->appliesTo(elementType) && channel->supports(valueType))
            stringList.append(channelName);
    }

    return stringList;
}

bool GraphModel::visualisationChannelAllowsMapping(const QString& channelName) const
{
    if(channelName.isEmpty())
        return false;

    if(!u::contains(_->_visualisationChannels, channelName))
        return false;

    return _->_visualisationChannels.at(channelName)->allowsMapping();
}

QStringList GraphModel::visualisationDescription(const QString& attributeName, const QStringList& channelNames) const
{
    QStringList descriptions;

    if(!attributeExists(attributeName))
        return descriptions;

    for(const auto& channelName : channelNames)
    {
        if(!u::contains(_->_visualisationChannels, channelName))
            return descriptions;

        auto attribute = attributeValueByName(attributeName);
        auto& channel = _->_visualisationChannels.at(channelName);

        if(!channel->supports(attribute.valueType()))
            descriptions.append(tr("This visualisation channel is not supported for the attribute type."));
        else
            descriptions.append(channel->description(attribute.elementType(), attribute.valueType()));
    }

    return descriptions;
}

void GraphModel::clearVisualisationInfos()
{
    _->_visualisationInfos.clear();
}

bool GraphModel::hasVisualisationInfo() const
{
    return !_->_visualisationInfos.empty();
}

const VisualisationInfo& GraphModel::visualisationInfoAtIndex(int index) const
{
    static const VisualisationInfo nullVisualisationInfo;

    if(u::contains(_->_visualisationInfos, index))
        return _->_visualisationInfos.at(index);

    return nullVisualisationInfo;
}

QVariantMap GraphModel::visualisationDefaultParameters(ValueType valueType, const QString& channelName) const
{
    if(!u::contains(_->_visualisationChannels, channelName))
        return {};

    auto& channel = _->_visualisationChannels.at(channelName);

    return channel->defaultParameters(valueType);
}

std::vector<QString> GraphModel::attributeNames(ElementType elementType) const
{
    std::vector<QString> attributeNames;

    for(auto& attribute : _->_attributes)
    {
        if(Flags<ElementType>(elementType).test(attribute.second.elementType()))
            attributeNames.emplace_back(attribute.first);
    }

    return attributeNames;
}

Attribute& GraphModel::createAttribute(QString name, QString* assignedName)
{
    name = normalisedAttributeName(name);

    if(assignedName != nullptr)
        *assignedName = name;

    Attribute& attribute = _->_attributes[name];

    // If we're creating an attribute during the graph transform, it's
    // a dynamically created attribute rather than a persistent one,
    // so mark it as such
    if(_graphTransformsAreChanging)
        attribute.setFlag(AttributeFlag::Dynamic);

    for(auto* tracker : _->_attributeChangesTrackers)
    {
        if(_graphTransformsAreChanging && _->_removedDynamicAttributeNames.contains(name))
            tracker->change(name);
        else
            tracker->add(name);
    }

    return attribute;
}

void GraphModel::addAttributes(const std::map<QString, Attribute>& attributes)
{
    _->_attributes.insert(attributes.begin(), attributes.end());
}

void GraphModel::removeAttribute(const QString& name)
{
    if(!u::contains(_->_attributes, name))
        return;

    _->_attributes.erase(name);

    for(auto* tracker : _->_attributeChangesTrackers)
        tracker->remove(name);
}

const Attribute* GraphModel::attributeByName(const QString& name) const
{
    auto attributeName = Attribute::parseAttributeName(name);

    if(!u::contains(_->_attributes, attributeName._name))
    {
        qDebug() << "WARNING: attribute unknown in attributeByName" << attributeName._name;
        return nullptr;
    }

    return &_->_attributes.at(attributeName._name);
}

Attribute* GraphModel::attributeByName(const QString& name)
{
    return const_cast<Attribute*>(const_cast<const GraphModel*>(this)->attributeByName(name));
}

bool GraphModel::attributeExists(const QString& name) const
{
    auto attributeName = Attribute::parseAttributeName(name);
    return u::contains(_->_attributes, attributeName._name);
}

bool GraphModel::attributeIsValid(const QString& name) const
{
    if(!attributeExists(name))
        return false;

    auto attributeName = Attribute::parseAttributeName(name);
    const auto* attribute = &_->_attributes.at(attributeName._name);

    const bool attributeDisabled = _graphTransformsAreChanging &&
        attribute->testFlag(AttributeFlag::DisableDuringTransform);

    return !attributeDisabled;
}

Attribute GraphModel::attributeValueByName(const QString& name) const
{
    auto attributeName = Attribute::parseAttributeName(name);

    if(!u::contains(_->_attributes, attributeName._name))
    {
        qDebug() << "WARNING: attribute unknown in attributeValueByName" << attributeName._name;
        return {};
    }

    auto attribute = _->_attributes.at(attributeName._name);

    if(!attributeName._parameter.isEmpty())
        attribute.setParameterValue(attributeName._parameter);

    if(attributeName._type != Attribute::EdgeNodeType::None)
    {
        return Attribute::edgeNodesAttribute(_->_graph,
            attribute, attributeName._type);
    }

    return attribute;
}

void GraphModel::calculateAttributeRange(const IGraph* graph, Attribute& attribute)
{
    if(!attribute.testFlag(AttributeFlag::AutoRange))
        return;

    if(attribute.elementType() == ElementType::Node)
        attribute.autoSetRangeForElements(graph->nodeIds());
    else if(attribute.elementType() == ElementType::Edge)
        attribute.autoSetRangeForElements(graph->edgeIds());
    else if(attribute.elementType() == ElementType::Component)
        qDebug() << "calculateAttributeRange called on component attribute";
}

void GraphModel::calculateAttributeRange(Attribute& attribute)
{
    calculateAttributeRange(&mutableGraph(), attribute);
}

static void calculateAttributeRanges(const Graph* graph,
    std::map<QString, Attribute>& attributes)
{
    for(auto& attribute : make_value_wrapper(attributes)) // clazy:exclude=compare-member-check
        GraphModel::calculateAttributeRange(graph, attribute);
}

void GraphModel::initialiseAttributeRanges()
{
    calculateAttributeRanges(&mutableGraph(), _->_attributes);
}

void GraphModel::initialiseSharedAttributeValues()
{
    for(auto& attribute : make_value_wrapper(_->_attributes)) // clazy:exclude=compare-member-check
        _->updateSharedAttributeValues(attribute);
}

bool GraphModel::attributeNameIsValid(const QString& attributeName)
{
    if(attributeName.isEmpty())
        return false;

    auto attributeNameRegex = QRegularExpression(IAttribute::ValidNameRegex);
    return attributeNameRegex.match(attributeName).hasMatch();
}

UserNodeData& GraphModel::userNodeData() { return _->_userNodeData; }
UserEdgeData& GraphModel::userEdgeData() { return _->_userEdgeData; }

void GraphModel::clearHighlightedNodes()
{
    highlightNodes({});
}

void GraphModel::highlightNodes(const NodeIdSet& nodeIds)
{
    bool requested = false;

    _->requestVisualUpdate([this, &nodeIds, &requested](auto& pending)
    {
        const auto& latestHighlightedNodeIds = pending._highlightedNodeIds ?
            *pending._highlightedNodeIds : _->_highlightedNodeIds;

        if(latestHighlightedNodeIds.empty() && nodeIds.empty())
            return;

        pending._highlightedNodeIds = nodeIds;
        pending._full = true;
        requested = true;
    });

    if(requested)
        scheduleVisualUpdate();
}

void GraphModel::enableVisualUpdates()
{
    // Loading has only just built the transforms and visualisations from scratch
    _->_transformRebuildRequired = false;
    _->_visualisationRebuildRequired = false;

    _visualUpdatesEnabled = true;
    _->requestFullVisualUpdate();

    // This is the one update made on the main thread, and it is made here deliberately.
    // Loading has finished, so there is no command to overlap, and the renderer is only
    // ever created while the main thread is blocked, so it cannot appear part way through
    // this update, and then read the visuals while they are being written, or see the
    // end of the change without having seen its beginning
    applyPendingVisualUpdates();
}

static float mappedSize(float min, float max, float user, float mapped = -1.0f)
{
    if(mapped < 0.0f)
        return user;

    // The fraction of the mapped value that contributes to the final value
    const float mappedRange = 0.75f;

    auto normalised = u::normalise(min, max, user);
    auto out = (mapped * mappedRange) + (normalised * (1.0f - mappedRange));

    return min + (out * (max - min));
}

static float mappedTextSize(float user, float mapped = -1.0f)
{
    auto value = mapped >= 0.0f ?
        ((1.0f * user) + (2.0f * mapped)) / 3.0f :
        user;

    return value < 0.5f ?
        u::interpolate(LimitConstants::minimumTextSize(), 1.0f, value / 0.5f) :
        u::interpolate(1.0f, LimitConstants::maximumTextSize(), (value - 0.5f) / 0.5f);
}

bool GraphModel::nodeIsUnhighlighted(NodeId nodeId, bool nodeIsSelected) const
{
    auto isNotFound = !_->_foundNodeIds.empty() && !u::contains(_->_foundNodeIds, nodeId);
    auto isNotHighlighted = !_->_highlightedNodeIds.empty() && nodeIsSelected &&
        !u::contains(_->_highlightedNodeIds, nodeId);

    return (isNotFound && _->_nodesMaskActive) || isNotHighlighted;
}

void GraphModel::scheduleVisualUpdate()
{
    // Before visual updates are enabled requests only accumulate, and
    // enabling them schedules the update that takes account of them all
    if(_visualUpdatesEnabled)
        emit visualUpdateRequested();
}

void GraphModel::applyPendingVisualUpdates()
{
    if(!_visualUpdatesEnabled)
        return;

    // Nothing locks the graph while it is read here: after loading it only ever changes
    // in commands, and this only ever runs between them, or at the end of one

    PendingVisualUpdate pending;
    NodeIdSet previousSelectedNodeIds;
    bool maskChanged = false;

    {
        const std::unique_lock<std::mutex> pendingLock(_->_pendingVisualUpdateMutex);
        pending = std::exchange(_->_pendingVisualUpdate, {});

        if(pending._selectedNodeIds)
        {
            previousSelectedNodeIds = std::exchange(_->_selectedNodeIds,
                std::move(*pending._selectedNodeIds));
        }

        if(pending._nodesMaskActive)
        {
            maskChanged = std::exchange(_->_nodesMaskActive,
                *pending._nodesMaskActive) != *pending._nodesMaskActive;
        }

        if(pending._foundNodeIds)
            _->_foundNodeIds = std::move(*pending._foundNodeIds);

        if(pending._highlightedNodeIds)
            _->_highlightedNodeIds = std::move(*pending._highlightedNodeIds);
    }

    // Whether a mask is in place affects the highlight state of every node, not
    // just the ones whose selected state changed, so there is no shortcut here
    if(pending._full || maskChanged)
        updateVisuals(pending._force);
    else if(pending._selectedNodeIds)
        updateSelectionVisuals(previousSelectedNodeIds);
}

void GraphModel::updateVisuals(bool force)
{
    std::unique_lock<std::shared_mutex> visualsLock(_->_visualsMutex);

    const float userNodeSize = _->_nodeSize;
    const float userEdgeSize = _->_edgeSize;
    const float userTextSize = _->_textSize;

    auto nodeColor      = u::pref(u"visuals/defaultNodeColor"_s).value<QColor>();
    auto edgeColor      = u::pref(u"visuals/defaultEdgeColor"_s).value<QColor>();
    auto multiColor     = u::pref(u"visuals/multiElementColor"_s).value<QColor>();
    auto nodeSize       = u::interpolate(LimitConstants::minimumNodeSize(), LimitConstants::maximumNodeSize(), userNodeSize);
    auto edgeSize       = u::interpolate(LimitConstants::minimumEdgeSize(), LimitConstants::maximumEdgeSize(), userEdgeSize);
    auto textSize       = mappedTextSize(userTextSize);
    auto textColor      = Document::contrastingColorForBackground();
    auto meIndicators   = u::pref(u"visuals/showMultiElementIndicators"_s).toBool();

    auto& nodeVisuals = _->_nodeVisuals;
    auto& edgeVisuals = _->_edgeVisuals;

    Flags<VisualChangeFlags> nodeChanges;
    Flags<VisualChangeFlags> edgeChanges;

    bool changing = false;
    auto beginChanging = [this, &changing]
    {
        if(changing)
            return;

        changing = true;
        emit visualsWillChange();
    };

    auto setIfChanged = [&beginChanging](auto& destination, const auto& value,
        Flags<VisualChangeFlags>& changes, VisualChangeFlags change)
    {
        if(destination != value) // clazy:exclude=compare-member-check
        {
            beginChanging();
            destination = value;
            changes.set(change);
        }
    };

    if(force)
        beginChanging();

    // An edge's state is accumulated from the nodes it joins, so it is staged here and
    // written back once complete, rather than being cleared and rebuilt where it lives
    EdgeArray<Flags<VisualFlags>> newEdgeStates(graph());

    for(auto edgeId : graph().edgeIds())
    {
        newEdgeStates[edgeId] = edgeVisuals[edgeId]._state;
        newEdgeStates[edgeId].reset(VisualFlags::Selected, VisualFlags::Unhighlighted);
    }

    auto setElementVisual = [&](auto elementId, auto& visual, const auto& mapped,
        float size, const QColor& color, const QString& text,
        Flags<VisualChangeFlags>& changes)
    {
        setIfChanged(visual._size, size, changes, VisualChangeFlags::Size);

        setIfChanged(visual._outerColor, mapped._outerColor.isValid() ?
            mapped._outerColor : color, changes, VisualChangeFlags::Color);

        setIfChanged(visual._innerColor,
            !meIndicators || graph().typeOf(elementId) == MultiElementType::Not ?
            visual._outerColor : multiColor, changes, VisualChangeFlags::Color);

        setIfChanged(visual._text, !mapped._text.isEmpty() ?
            mapped._text : text, changes, VisualChangeFlags::Text);

        setIfChanged(visual._textSize, mappedTextSize(userTextSize, mapped._textSize),
            changes, VisualChangeFlags::TextSize);

        setIfChanged(visual._textColor, mapped._textColor.isValid() ?
            mapped._textColor : textColor, changes, VisualChangeFlags::TextColor);
    };

    for(auto nodeId : graph().nodeIds())
    {
        auto& visual = nodeVisuals[nodeId];
        const auto& mapped = _->_mappedNodeVisuals[nodeId];

        setElementVisual(nodeId, visual, mapped,
            mappedSize(LimitConstants::minimumNodeSize(), LimitConstants::maximumNodeSize(),
            nodeSize, mapped._size), nodeColor, nodeName(nodeId), nodeChanges);

        auto nodeIsSelected = u::contains(_->_selectedNodeIds, nodeId);
        auto nodeUnhighlighted = nodeIsUnhighlighted(nodeId, nodeIsSelected);

        auto state = visual._state;
        state.setState(VisualFlags::Selected, nodeIsSelected);
        state.setState(VisualFlags::Unhighlighted, nodeUnhighlighted);
        setIfChanged(visual._state, state, nodeChanges, VisualChangeFlags::State);

        if(nodeIsSelected)
        {
            for(auto edgeId : graph().edgeIdsForNodeId(nodeId))
                newEdgeStates[edgeId].set(VisualFlags::Selected);
        }

        if(nodeUnhighlighted)
        {
            for(auto edgeId : graph().edgeIdsForNodeId(nodeId))
                newEdgeStates[edgeId].set(VisualFlags::Unhighlighted);
        }
    }

    for(auto edgeId : graph().edgeIds())
    {
        auto& visual = edgeVisuals[edgeId];
        const auto& mapped = _->_mappedEdgeVisuals[edgeId];

        // An edge is never drawn thicker than either of the nodes it joins
        const auto& edge = graph().edgeById(edgeId);
        auto size = std::min({mappedSize(LimitConstants::minimumEdgeSize(),
            LimitConstants::maximumEdgeSize(), edgeSize, mapped._size),
            nodeVisuals[edge.sourceId()]._size, nodeVisuals[edge.targetId()]._size});

        setElementVisual(edgeId, visual, mapped, size, edgeColor, {}, edgeChanges);

        setIfChanged(visual._state, newEdgeStates[edgeId], edgeChanges, VisualChangeFlags::State);
    }

    for(auto& [componentId, textVisuals] : _->_newTextVisuals)
    {
        for(auto& textVisual : textVisuals)
        {
            // This is a reasonable approximation for fixing the text size to something that
            // is close to the average radius of the node island being annotated
            textVisual._size = textSize * std::cbrt(static_cast<float>(textVisual._nodeIds.size()));
            textVisual._color = textColor;
        }
    }

    auto findTextVisualsChange = [](const TextVisuals& previous, const TextVisuals& current)
    {
        Flags<VisualChangeFlags> change;

        if(previous.size() != current.size())
            return VisualChangeFlags::State;

        auto previousComponentIds = u::keysFor(previous);
        auto currentComponentIds = u::keysFor(current);

        if(previousComponentIds != currentComponentIds)
            return VisualChangeFlags::State;

        for(const ComponentId componentId : previousComponentIds)
        {
            const auto& previousTextVisuals = previous.at(componentId);
            const auto& currentTextVisuals = current.at(componentId);

            if(previousTextVisuals.size() != currentTextVisuals.size())
                return VisualChangeFlags::State;

            for(size_t i = 0; i < previousTextVisuals.size(); i++)
            {
                const auto& previousTextVisual = previousTextVisuals.at(i);
                const auto& currentTextVisual = currentTextVisuals.at(i);

                if(previousTextVisual._nodeIds != currentTextVisual._nodeIds)
                    return VisualChangeFlags::State;

                if(previousTextVisual._color != currentTextVisual._color)
                    change.set(VisualChangeFlags::TextColor);

                if(previousTextVisual._size != currentTextVisual._size)
                    change.set(VisualChangeFlags::TextSize);
            }
        }

        return *change;
    };

    const VisualChangeFlags textChange = findTextVisualsChange(_->_textVisuals, _->_newTextVisuals);

    if(textChange != VisualChangeFlags::None)
        beginChanging();

    if(!changing)
        return;

    {
        const std::unique_lock<std::mutex> textVisualsLock(_->_textVisualsMutex);

        _->_textVisuals = _->_newTextVisuals;
        updateTextVisualPositions(_->_textVisuals, _->_nodePositions);
    }

    visualsLock.unlock();
    emit visualsChanged(*nodeChanges, *edgeChanges, textChange);
}

// When the selection changes, only update the selection visuals
void GraphModel::updateSelectionVisuals(const NodeIdSet& previousSelectedNodeIds)
{
    std::vector<NodeId> changedNodeIds;

    for(auto nodeId : _->_selectedNodeIds)
    {
        if(!u::contains(previousSelectedNodeIds, nodeId))
            changedNodeIds.emplace_back(nodeId);
    }

    for(auto nodeId : previousSelectedNodeIds)
    {
        if(!u::contains(_->_selectedNodeIds, nodeId))
            changedNodeIds.emplace_back(nodeId);
    }

    if(changedNodeIds.empty())
        return;

    std::unique_lock<std::shared_mutex> visualsLock(_->_visualsMutex);

    emit visualsWillChange();

    Flags<VisualChangeFlags> nodeChange;
    Flags<VisualChangeFlags> edgeChange;

    for(auto nodeId : changedNodeIds)
    {
        auto& nodeVisual = _->_nodeVisuals[nodeId];
        const auto previousState = nodeVisual._state;

        auto nodeIsSelected = u::contains(_->_selectedNodeIds, nodeId);

        nodeVisual._state.setState(VisualFlags::Selected, nodeIsSelected);
        nodeVisual._state.setState(VisualFlags::Unhighlighted,
            nodeIsUnhighlighted(nodeId, nodeIsSelected));

        if(nodeVisual._state != previousState) // clazy:exclude=compare-member-check
            nodeChange.set(VisualChangeFlags::State);
    }

    for(auto nodeId : changedNodeIds)
    {
        for(auto edgeId : graph().edgeIdsForNodeId(nodeId))
        {
            auto& edgeVisual = _->_edgeVisuals[edgeId];
            const auto previousState = edgeVisual._state;

            // An edge is selected or unhighlighted if either of the nodes it joins is;
            // those are precisely the nodes whose edgeIdsForNodeId contains it
            const auto& edge = graph().edgeById(edgeId);
            const auto sourceState = _->_nodeVisuals[edge.sourceId()]._state;
            const auto targetState = _->_nodeVisuals[edge.targetId()]._state;

            edgeVisual._state.setState(VisualFlags::Selected,
                sourceState.test(VisualFlags::Selected) ||
                targetState.test(VisualFlags::Selected));

            edgeVisual._state.setState(VisualFlags::Unhighlighted,
                sourceState.test(VisualFlags::Unhighlighted) ||
                targetState.test(VisualFlags::Unhighlighted));

            if(edgeVisual._state != previousState) // clazy:exclude=compare-member-check
                edgeChange.set(VisualChangeFlags::State);
        }
    }

    visualsLock.unlock();
    emit visualsChanged(*nodeChange, *edgeChange, VisualChangeFlags::None);
}

void GraphModel::onSelectionChanged(const SelectionManager* selectionManager)
{
    auto selectedNodeIds = selectionManager->selectedNodes();
    auto nodesMaskActive = selectionManager->nodesMaskActive();

    _->requestVisualUpdate([&selectedNodeIds, nodesMaskActive](auto& pending)
    {
        pending._selectedNodeIds = std::move(selectedNodeIds);
        pending._nodesMaskActive = nodesMaskActive;
    });

    scheduleVisualUpdate();
}

void GraphModel::onFoundNodeIdsChanged(const SearchManager* searchManager)
{
    auto foundNodeIds = searchManager->foundNodeIds();

    _->requestVisualUpdate([&foundNodeIds](auto& pending)
    {
        pending._foundNodeIds = std::move(foundNodeIds);
        pending._full = true;
    });

    scheduleVisualUpdate();
}

void GraphModel::onPreferenceChanged(const QString& name, const QVariant&)
{
    if(!name.startsWith(u"visuals"_s))
        return;

    const bool force = name.endsWith(u"backgroundColor"_s) ||
        name.endsWith(u"showNodeText"_s) ||
        name.endsWith(u"showEdgeText"_s) ||
        name.endsWith(u"showEdges"_s) ||
        name.endsWith(u"edgeVisualType"_s) ||
        name.endsWith(u"highlightColor"_s) ||
        name.endsWith(u"textFont"_s) ||
        name.endsWith(u"textSize"_s) ||
        name.endsWith(u"textAlignment"_s);

    _->requestVisualUpdate([force](auto& pending)
    {
        pending._full = true;
        pending._force = pending._force || force;
    });

    scheduleVisualUpdate();
}

void GraphModel::onLayoutChanged()
{
    // This occurs on the layout thread
    const std::unique_lock<std::mutex> lock(_->_textVisualsMutex);
    updateTextVisualPositions(_->_textVisuals, _->_nodePositions);
}

void GraphModel::onMutableGraphChanged(const Graph* graph)
{
    calculateAttributeRanges(graph, _->_attributes);
}

void GraphModel::onTransformedGraphWillChange(const Graph*)
{
    // The graph may gain elements that have never been through a full visual
    // update, so whatever next asks for an update must get a full one
    _->requestFullVisualUpdate();

    // Rebuilding applies the transforms to the attribute values as they now are
    _->_transformRebuildRequired = false;

    // Store previous attributes for comparison
    _->_previousAttributeIdentities = _->currentAttributeIdentities();

    removeDynamicAttributes();

    _graphTransformsAreChanging = true;
}

void GraphModel::onTransformedGraphChanged(const Graph*, bool changeOccurred)
{
    _->_removedDynamicAttributeNames.clear();

    auto attributeIdentities = _->currentAttributeIdentities();

    // Compare with previous attributes
    auto removedAttributeIdentities = u::setDifference(_->_previousAttributeIdentities, attributeIdentities);
    auto addedAttributeIdentities = u::setDifference(attributeIdentities, _->_previousAttributeIdentities);

    QStringList removedAttributeNames;
    QStringList addedAttributeNames;

    auto identityToName = [](const auto& identity) { return identity._name; };

    std::ranges::transform(removedAttributeIdentities,
        std::back_inserter(removedAttributeNames), identityToName);
    std::ranges::transform(addedAttributeIdentities,
        std::back_inserter(addedAttributeNames), identityToName);

    QStringList changedAttributeNames(std::move(_->_changedDynamicAttributeNames));

    changedAttributeNames.erase(std::ranges::remove_if(changedAttributeNames, // clazy:exclude=strict-iterators
    [&removedAttributeNames, &addedAttributeNames](const auto& dynamicAttributeName)
    {
        return u::contains(removedAttributeNames, dynamicAttributeName) ||
            u::contains(addedAttributeNames, dynamicAttributeName);
    }).begin(), changedAttributeNames.end()); // clazy:exclude=strict-iterators

    // When the graph changes, every attribute's range and shared values also potentially change
    for(auto& [name, attribute] : _->_attributes)
    {
        // These ones will be taken care of in onAttributesChanged, below...
        if(u::contains(addedAttributeNames, name) || u::contains(changedAttributeNames, name))
            continue;

        calculateAttributeRange(attribute);
        _->updateSharedAttributeValues(attribute);
    }

    emit attributesChanged(addedAttributeNames, removedAttributeNames,
        changedAttributeNames, changeOccurred);

    _graphTransformsAreChanging = false;
}

// This will also be called when the transformed graph changes, even if there are no actual attribute changes
void GraphModel::onAttributesChanged(const QStringList& addedNames, const QStringList& removedNames,
    const QStringList& changedValuesNames, bool graphChangeOccurred)
{
    for(const auto& attributeName : u::combine(addedNames, changedValuesNames))
    {
        auto& attribute = _->_attributes.at(Attribute::parseAttributeName(attributeName)._name);

        if(attribute.testFlag(AttributeFlag::Dynamic) && attribute.userDefined())
        {
            qDebug() << "WARNING: Dynamic attribute" << attributeName <<
                "has userDefined set, this is not allowed, resetting...";
            attribute.setUserDefined(false);
        }

        calculateAttributeRange(attribute);
        _->updateSharedAttributeValues(attribute);
    }

    // Attribute changes may require a graph transform or visualisation to be rebuilt
    QStringList changed;
    changed << addedNames << removedNames << changedValuesNames;
    u::removeDuplicates(changed);

    const bool transformRebuildRequired = !_graphTransformsAreChanging &&
        _->_transformedGraph.onAttributeValuesChangedExternally(changed);

    const bool visualisationRebuildRequired = graphChangeOccurred || !u::setIntersection(
        static_cast<const QList<QString>&>(_->_visualisedAttributeNames),
        static_cast<const QList<QString>&>(changed)).empty();

    if(transformRebuildRequired)
        _->_transformRebuildRequired = true;

    if(visualisationRebuildRequired)
        _->_visualisationRebuildRequired = true;

    // The rebuilds happen alongside the next visual update
    if(transformRebuildRequired || visualisationRebuildRequired)
        scheduleVisualUpdate();
}

AttributeChangesTracker::AttributeChangesTracker(GraphModel* graphModel,
    NamedBool<"emitOnDestruct"> emitOnDestruct) :
    _graphModel(graphModel), _emitOnDestruct(emitOnDestruct)
{
    _graphModel->_->_attributeChangesTrackers.insert(this);
}

AttributeChangesTracker::~AttributeChangesTracker()
{
    if(_emitOnDestruct)
        emitAttributesChanged();

    _graphModel->_->_attributeChangesTrackers.erase(this);
}

void AttributeChangesTracker::add(const QString& name)
{
    if(u::contains(_changed, name))
    {
        qDebug() << "AttributeChangesTracker::add called on attribute marked as changed" << name;
        _changed.remove(name);
    }

    if(u::contains(_removed, name))
    {
        // If it's been removed and added, count that as changed (potentially)
        _removed.remove(name);
        _changed.insert(name);
    }
    else
       _added.insert(name);
}

void AttributeChangesTracker::remove(const QString& name)
{
    if(u::contains(_added, name))
    {
        // If it's been added and removed, count that as net zero
        _added.remove(name);
    }
    else
        _removed.insert(name);
}

void AttributeChangesTracker::change(const QString& name)
{
    // Only count an attribute as changed if it's not new
    if(!u::contains(_added, name))
        _changed.insert(name);
}

QStringList AttributeChangesTracker::addedOrChanged() const
{
    auto combined = u::combine(_added, _changed);
    return {combined.begin(), combined.end()};
}

void AttributeChangesTracker::emitAttributesChanged()
{
    if(_added.empty() && _removed.empty() && _changed.empty())
        return;

    emit _graphModel->attributesChanged(added(), removed(), changed(), false);
}
