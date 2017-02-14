#include "graphmodel.h"
#include "componentmanager.h"

#include "ui/selectionmanager.h"
#include "ui/searchmanager.h"

#include "transform/compoundtransform.h"
#include "transform/filtertransform.h"
#include "transform/edgecontractiontransform.h"
#include "transform/graphtransformconfigparser.h"

#include "shared/utils/enumreflection.h"
#include "shared/utils/preferences.h"
#include "shared/utils/utils.h"

#include <QRegularExpression>

#include <utility>

GraphModel::GraphModel(const QString &name, IPlugin* plugin) :
    _graph(),
    _transformedGraph(_graph),
    _nodePositions(_graph),
    _nodeVisuals(_graph),
    _edgeVisuals(_graph),
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

    connect(S(Preferences), &Preferences::preferenceChanged, this, &GraphModel::onPreferenceChanged);

    dataField(tr("Node Degree"))
        .setIntValueFn([this](NodeId nodeId) { return _transformedGraph.nodeById(nodeId).degree(); })
        .setIntMin(0)
        .setDescription(tr("A node's degree is its number of incident edges."));

    dataField(tr("Component Size"))
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
        auto graphTransform = factory->create(graphTransformConfig, _dataFields);

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

        if(graphTransformConfig.isMetaAttributeSet("disabled"))
            continue;

        if(!u::contains(_graphTransformFactories, action))
            continue;

        auto& factory = _graphTransformFactories.at(action);
        auto graphTransform = factory->create(graphTransformConfig, _dataFields);

        if(graphTransform)
        {
            graphTransform->setRepeating(graphTransformConfig.isMetaAttributeSet("repeating"));
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
        if(!availableDataFields(t.first).isEmpty())
            stringList.append(t.first);
    }

    return stringList;
}

QStringList GraphModel::availableDataFields(const QString& transformName) const
{
    QStringList stringList;

    if(!transformName.isEmpty())
    {
        auto elementType = _graphTransformFactories.at(transformName)->elementType();

        for(auto& f : _dataFields)
        {
            if(f.second.elementType() == elementType)
                stringList.append(f.first);
        }
    }

    return stringList;
}

QStringList GraphModel::avaliableConditionFnOps(const QString& dataFieldName) const
{
    if(dataFieldName.isEmpty() || !u::contains(_dataFields, dataFieldName))
        return {};

    return GraphTransformConfigParser::ops(_dataFields.at(dataFieldName).valueType());
}

QStringList GraphModel::dataFieldNames(ElementType elementType) const
{
    QStringList dataFieldNames;

    for(auto& dataField : _dataFields)
    {
        if(dataField.second.elementType() == elementType)
            dataFieldNames.append(dataField.first);
    }

    return dataFieldNames;
}

IDataField& GraphModel::dataField(const QString& name)
{
    return _dataFields[name];
}

const DataField& GraphModel::dataFieldByName(const QString& name) const
{
    static DataField nullDataField;
    return u::contains(_dataFields, name) ? _dataFields.at(name) : nullDataField;
}

void GraphModel::enableVisualUpdates()
{
    _visualUpdatesEnabled = true;
    updateVisuals();
}

void GraphModel::updateVisuals(const SelectionManager* selectionManager, const SearchManager* searchManager)
{
    if(!_visualUpdatesEnabled)
        return;

    emit visualsWillChange();

    auto nodeColor  = u::pref("visuals/defaultNodeColor").value<QColor>();
    auto edgeColor  = u::pref("visuals/defaultEdgeColor").value<QColor>();
    auto multiColor = u::pref("visuals/multiElementColor").value<QColor>();
    auto nodeSize   = u::pref("visuals/defaultNodeSize").toFloat();
    auto edgeSize   = u::pref("visuals/defaultEdgeSize").toFloat();

    if(searchManager != nullptr)
    {
        // Clear all edge NotFound flags as we can't know what to change unless
        // we have the previous search state to hand
        for(auto edgeId : graph().edgeIds())
            _edgeVisuals[edgeId]._state.setFlag(VisualFlags::NotFound, false);
    }

    for(auto nodeId : graph().nodeIds())
    {
        _nodeVisuals[nodeId]._size = nodeSize;
        _nodeVisuals[nodeId]._color = graph().typeOf(nodeId) == NodeIdDistinctSetCollection::Type::Not ?
                    nodeColor : multiColor;
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
        // Restrict edgeSize to be no larger than the source or target size
        auto& edge = graph().edgeById(edgeId);
        auto minNodeSize = std::min(_nodeVisuals[edge.sourceId()]._size,
                                    _nodeVisuals[edge.targetId()]._size);
        _edgeVisuals[edgeId]._size = std::min(edgeSize, minNodeSize);

        _edgeVisuals[edgeId]._color = graph().typeOf(edgeId) == EdgeIdDistinctSetCollection::Type::Not ?
                    edgeColor : multiColor;
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

