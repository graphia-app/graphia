#include "graphmodel.h"
#include "componentmanager.h"

#include "../ui/selectionmanager.h"
#include "../ui/searchmanager.h"

#include "../transform/compoundtransform.h"
#include "../transform/filtertransform.h"
#include "../transform/edgecontractiontransform.h"

#include "../utils/enumreflection.h"
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
       _nodeVisuals[nodeId]._state = 0;
    });
    connect(&_transformedGraph, &Graph::edgeRemoved, [this](const Graph*, EdgeId edgeId)
    {
       _edgeVisuals[edgeId]._state = 0;
    });

    connect(S(Preferences), &Preferences::preferenceChanged, this, &GraphModel::onPreferenceChanged);

    dataField(tr("Node Degree"))
        .setIntValueFn([this](NodeId nodeId) { return _transformedGraph.nodeById(nodeId).degree(); })
        .setIntMin(0);

    dataField(tr("Node Name"))
        .setStringValueFn([this](NodeId nodeId) { return _nodeNames[nodeId]; })
        .setSearchable(true);

    dataField(tr("Component Size"))
        .setIntValueFn([this](const GraphComponent& component) { return component.numNodes(); })
        .setIntMin(1);

    _graphTransformFactories.emplace(tr("Filter Nodes"),
        std::make_pair(DataFieldElementType::Node, std::make_unique<FilterTransformFactory>()));

    _graphTransformFactories.emplace(tr("Filter Edges"),
        std::make_pair(DataFieldElementType::Edge, std::make_unique<FilterTransformFactory>()));
    _graphTransformFactories.emplace(tr("Contract Edges"),
        std::make_pair(DataFieldElementType::Edge, std::make_unique<EdgeContractionTransformFactory>()));

    _graphTransformFactories.emplace(tr("Filter Components"),
        std::make_pair(DataFieldElementType::Component, std::make_unique<FilterTransformFactory>()));
}

void GraphModel::setNodeName(NodeId nodeId, const QString& name)
{
    _nodeNames[nodeId] = name;
    updateVisuals();
}

static std::unique_ptr<GraphTransform> createGraphTransform(const GraphTransformConfiguration& config,
                                                            const GraphTransformFactory& factory,
                                                            const DataField& field)
{
    switch(field.type())
    {
    case DataFieldType::IntNode:            return factory.create(field.createNodeConditionFn(config.opType(),      config.value().toInt()));
    case DataFieldType::IntEdge:            return factory.create(field.createEdgeConditionFn(config.opType(),      config.value().toInt()));
    case DataFieldType::IntComponent:       return factory.create(field.createComponentConditionFn(config.opType(), config.value().toInt()));

    case DataFieldType::FloatNode:          return factory.create(field.createNodeConditionFn(config.opType(),      config.value().toFloat()));
    case DataFieldType::FloatEdge:          return factory.create(field.createEdgeConditionFn(config.opType(),      config.value().toFloat()));
    case DataFieldType::FloatComponent:     return factory.create(field.createComponentConditionFn(config.opType(), config.value().toFloat()));

    case DataFieldType::StringNode:         return factory.create(field.createNodeConditionFn(config.opType(),      config.value()));
    case DataFieldType::StringEdge:         return factory.create(field.createEdgeConditionFn(config.opType(),      config.value()));
    case DataFieldType::StringComponent:    return factory.create(field.createComponentConditionFn(config.opType(), config.value()));

    default: return nullptr;
    }
}

void GraphModel::buildTransforms(const std::vector<GraphTransformConfiguration>& graphTransformConfigurations)
{
    auto compoundTransform = std::make_unique<CompoundTransform>();

    for(auto& graphTransformConfiguration : graphTransformConfigurations)
    {
        if(!graphTransformConfiguration.enabled() || !graphTransformConfiguration.valid())
            continue;

        if(!u::contains(_graphTransformFactories, graphTransformConfiguration.name()))
            continue;

        if(!u::contains(_dataFields, graphTransformConfiguration.fieldName()))
            continue;

        auto& factory = _graphTransformFactories.at(graphTransformConfiguration.name()).second;
        auto& field = _dataFields.at(graphTransformConfiguration.fieldName());
        std::unique_ptr<GraphTransform> graphTransform = createGraphTransform(graphTransformConfiguration, *factory, field);

        if(graphTransform)
            compoundTransform->addTransform(std::move(graphTransform));
    }

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

template<typename Factories, typename Fields>
static QStringList availableDataFieldsInFactories(const Factories& factories,
                                                  const Fields& fields,
                                                  const QString& transformName)
{
    QStringList stringList;

    if(u::contains(factories, transformName))
    {
        for(auto& f : fields)
            stringList.append(f.first);
    }

    return stringList;
}

QStringList GraphModel::availableDataFields(const QString& transformName) const
{
    QStringList stringList;

    if(!transformName.isEmpty())
    {
        auto elementType = _graphTransformFactories.at(transformName).first;

        for(auto& f : _dataFields)
        {
            if(f.second.elementType() == elementType)
                stringList.append(f.first);
        }
    }

    return stringList;
}

DataFieldType GraphModel::typeOfDataField(const QString& dataFieldName) const
{
    return _dataFields.at(dataFieldName).type();
}

QStringList GraphModel::avaliableConditionFnOps(const QString& dataFieldName) const
{
    QStringList stringList;

    if(!dataFieldName.isEmpty())
    {
        auto validConditionFnOps = _dataFields.at(dataFieldName).validConditionFnOps();

        for(auto& conditionFnOp : validConditionFnOps)
            stringList.append(enumToString(conditionFnOp));
    }

    return stringList;
}

QStringList GraphModel::dataFieldNames(DataFieldElementType elementType) const
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

