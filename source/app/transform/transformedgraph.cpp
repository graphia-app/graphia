#include "transformedgraph.h"

#include "graph/componentmanager.h"
#include "graph/graphmodel.h"

#include "shared/commands/icommand.h"
#include "shared/utils/container.h"

#include <functional>

TransformedGraph::TransformedGraph(GraphModel& graphModel, const MutableGraph& source) :
    Graph(),
    _graphModel(&graphModel),
    _source(&source),
    _cache(graphModel),
    _cancelled(false),
    _nodesState(source),
    _edgesState(source),
    _previousNodesState(source),
    _previousEdgesState(source)
{
    connect(_source, &Graph::graphChanged, [this]
    {
        // If the source graph changes at all, our cache is invalid
        _cache.clear();
        rebuild();
    });

    connect(&_target, &Graph::graphChanged, this, &TransformedGraph::onTargetGraphChanged, Qt::DirectConnection);
    enableComponentManagement();

    // These connections allow us to track what changes, so we can then
    // re-emit a canonical set of signals once the transform is complete
    connect(_source, &Graph::nodeRemoved,  [this](const Graph*, NodeId nodeId) { _nodesState[nodeId].remove(); });
    connect(_source, &Graph::nodeAdded,    [this](const Graph*, NodeId nodeId) { _nodesState[nodeId].add(); });
    connect(_source, &Graph::edgeRemoved,  [this](const Graph*, EdgeId edgeId) { _edgesState[edgeId].remove(); });
    connect(_source, &Graph::edgeAdded,    [this](const Graph*, EdgeId edgeId) { _edgesState[edgeId].add(); });

    connect(&_target, &Graph::nodeRemoved, [this](const Graph*, NodeId nodeId) { _nodesState[nodeId].remove(); });
    connect(&_target, &Graph::nodeAdded,   [this](const Graph*, NodeId nodeId) { _nodesState[nodeId].add(); });
    connect(&_target, &Graph::edgeRemoved, [this](const Graph*, EdgeId edgeId) { _edgesState[edgeId].remove(); });
    connect(&_target, &Graph::edgeAdded,   [this](const Graph*, EdgeId edgeId) { _edgesState[edgeId].add(); });

    addTransform(std::make_unique<IdentityTransform>());
}

void TransformedGraph::cancelRebuild()
{
    std::unique_lock<std::mutex> lock(_currentTransformMutex);
    _cancelled = true;

    if(_currentTransform != nullptr)
        _currentTransform->cancel();
}

void TransformedGraph::setProgress(int progress)
{
    if(_command != nullptr)
        _command->setProgress(progress);
}

void TransformedGraph::reserve(const Graph& other)
{
    _target.reserve(other);
    Graph::reserve(other);
}

MutableGraph& TransformedGraph::operator=(const MutableGraph& other)
{
    _target = other;
    Graph::reserve(other);

    return _target;
}

void TransformedGraph::rebuild()
{
    if(!_autoRebuild)
        return;

    _cancelled = false;

    emit graphWillChange(this);

    QStringList updatedAttributeNames;

    _target.performTransaction([this, &updatedAttributeNames](IMutableGraph&)
    {
        _graphChangeOccurred = false;

        TransformCache newCache(*_graphModel);
        *this = *_source;

        // Save previous state in case we get cancelled
        auto oldCache = _cache;

        // Save attributes of current graph so we can remove ones added if cancelled
        auto fixedAttributeNames = _graphModel->attributeNames();

        for(auto& transform : _transforms)
        {
            setProgress(-1); // Indetermindate by default

            TransformCache::Result result;
            result._config = transform->config();

            result = _cache.apply(result._config, *this);
            if(result.isApplicable())
            {
                newCache.add(std::move(result));
                continue;
            }

            // Save the attribute names before the transform application
            // so we can see which attributes are created
            auto attributeNames = _graphModel->attributeNames();

            setCurrentTransform(transform.get());
            transform->uncancel();

            if(transform->applyAndUpdate(*this))
            {
                result._graph = std::make_unique<MutableGraph>(_target);

                // Graph has changed, so the cache is now invalid
                _cache.clear();
            }

            for(const auto& attribute : transform->attributes())
                updatedAttributeNames.append(attribute);

            setCurrentTransform(nullptr);

            if(_cancelled)
                break;

            for(const auto& attributeName : u::setDifference(_graphModel->attributeNames(), attributeNames))
            {
                result._newAttributes.emplace(attributeName, _graphModel->attributeValueByName(attributeName));
                _cache.attributeAdded(attributeName);
            }

            newCache.add(std::move(result));
        }

        if(_cancelled)
        {
            // We've been cancelled so rollback to our previous state
            _cache = std::move(oldCache);
            auto* cachedGraph = _cache.graph();
            *this = (cachedGraph != nullptr ? *cachedGraph : *_source);

            // Remove any attributes that were added before the cancel occurred
            for(const auto& attributeName : u::setDifference(_graphModel->attributeNames(), fixedAttributeNames))
                _graphModel->removeAttribute(attributeName);

            _graphModel->addAttributes(_cache.attributes());
        }

        _cache = std::move(newCache);
    });

    emit attributeValuesChanged(updatedAttributeNames);

    emit graphChanged(this, _graphChangeOccurred);
    clearPhase();
}

void TransformedGraph::setCurrentTransform(GraphTransform* currentTransform)
{
    std::unique_lock<std::mutex> lock(_currentTransformMutex);
    _currentTransform = currentTransform;
}

void TransformedGraph::onTargetGraphChanged(const Graph*)
{
    // Let everything know what changed; note the signals won't necessarily happen in the order
    // in which the changes originally occurred, but adding nodes and edges, then removing edges
    // and nodes ensures that the receivers get a sane view at all times
    for(NodeId nodeId(0); nodeId < _nodesState.size(); ++nodeId)
    {
        if(!_previousNodesState[nodeId].added() && _nodesState[nodeId].added())
        {
            emit nodeAdded(this, nodeId);
            _graphChangeOccurred = true;
        }
    }

    for(EdgeId edgeId(0); edgeId < _edgesState.size(); ++edgeId)
    {
        if(!_previousEdgesState[edgeId].added() && _edgesState[edgeId].added())
        {
            emit edgeAdded(this, edgeId);
            _graphChangeOccurred = true;
        }
        else if(!_previousEdgesState[edgeId].removed() && _edgesState[edgeId].removed())
        {
            emit edgeRemoved(this, edgeId);
            _graphChangeOccurred = true;
        }
    }

    for(NodeId nodeId(0); nodeId < _nodesState.size(); ++nodeId)
    {
        if(!_previousNodesState[nodeId].removed() && _nodesState[nodeId].removed())
        {
            emit nodeRemoved(this, nodeId);
            _graphChangeOccurred = true;
        }
    }

    _previousNodesState = _nodesState;
    _previousEdgesState = _edgesState;

    _nodesState.resetElements();
    _edgesState.resetElements();
}
