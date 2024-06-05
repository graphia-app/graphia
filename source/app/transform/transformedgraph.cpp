/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include "transformedgraph.h"

#include "app/graph/componentmanager.h"
#include "app/graph/graphmodel.h"

#include "shared/commands/icommand.h"
#include "shared/utils/container.h"
#include "shared/utils/container_combine.h"
#include "shared/utils/string.h"

#include <functional>

TransformedGraph::TransformedGraph(GraphModel& graphModel, const MutableGraph& source) :
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
    const std::unique_lock<std::mutex> lock(_currentTransformMutex);
    _cancelled = true;

    if(_currentTransform != nullptr)
        _currentTransform->cancel();
}

bool TransformedGraph::onAttributeValuesChangedExternally(const QStringList& changedAttributeNames)
{
    std::vector<QString> referencedAttributeNames;

    for(const auto& transform : _transforms)
    {
        const auto& names = transform->config().referencedAttributeNames();
        referencedAttributeNames.insert(referencedAttributeNames.end(),
            names.begin(), names.end());
    }

    u::removeDuplicates(referencedAttributeNames);

    auto affectedAttributeNames = u::setIntersection(referencedAttributeNames,
        u::toQStringVector(changedAttributeNames));

    if(affectedAttributeNames.empty())
        return false;

    // Invalidate any cache entries affected by the changing attributes
    for(const auto& attributeName : affectedAttributeNames)
        _cache.attributeAddedOrChanged(attributeName);

    return true;
}

void TransformedGraph::reserve(const Graph& other)
{
    _target.reserve(other);
    Graph::reserve(other);
}

TransformedGraph& TransformedGraph::operator=(const MutableGraph& other)
{
    _target = other;
    Graph::reserve(other);

    return *this;
}

bool TransformedGraph::update()
{
    _graphChangeOccurred = _target.update() || _graphChangeOccurred;
    return _graphChangeOccurred;
}

std::vector<QString> TransformedGraph::addedOrChangedAttributeNamesAtTransformIndex(int index) const
{
    if(u::contains(_addedOrChangedAttributeNames, index))
        return _addedOrChangedAttributeNames.at(index);

    return {};
}

void TransformedGraph::rebuild()
{
    if(!_autoRebuild)
        return;

    _cancelled = false;

    emit graphWillChange(this);

    // Disable the CompomentManager for the duration of the transform
    // as its data will be out of date anyway; if transforms need to
    // work on components, they will need to use their own
    // ComponentManager instance
    disableComponentManagement();

    QStringList changedAttributeNames;

    _target.performTransaction([this, &changedAttributeNames](IMutableGraph&)
    {
        _changeSignalsEmitted = false;

        TransformCache newCache(*_graphModel);
        AddedOrChangedAttributeNamesMap newAddedOrChangedAttributeNames;
        *this = *_source;
        _target.update();

        // Save previous state in case we get cancelled
        auto oldCache = _cache;
        auto oldAddedOrChangedAttributeNames = _addedOrChangedAttributeNames;

        // Save attributes of current graph so we can remove ones added if cancelled
        auto fixedAttributeNames = _graphModel->attributeNames();

        for(auto& transform : _transforms)
        {
            transform->setProgress(-1); // Indeterminate by default

            TransformCache::Result result;
            result._config = transform->config();

            result = _cache.apply(transform->index(), result._config, *this);
            if(result.wasApplied())
            {
                newAddedOrChangedAttributeNames[transform->index()] = u::keysFor(result._addedOrChangedAttributes);
                newCache.add(std::move(result));
                continue;
            }

            const AttributeChangesTracker tracker(_graphModel, "emitOnDestruct"_no);

            setCurrentTransform(transform.get());
            transform->uncancel();

            if(transform->applyAndUpdate(*this, *_graphModel))
            {
                result._graph = std::make_unique<MutableGraph>(_target);

                // Graph has changed, so the cache is now invalid
                _cache.clear();
            }

            setCurrentTransform(nullptr);

            if(_cancelled)
                break;

            for(const auto& attributeName : tracker.changed()) // clazy:exclude=range-loop-detach
                changedAttributeNames.append(attributeName);

            for(const auto& attributeName : tracker.addedOrChanged()) // clazy:exclude=range-loop-detach
            {
                result._addedOrChangedAttributes.emplace(attributeName, _graphModel->attributeValueByName(attributeName));
                _cache.attributeAddedOrChanged(attributeName);
            }

            result._index = transform->index();

            newAddedOrChangedAttributeNames[transform->index()] = u::toQStringVector(tracker.addedOrChanged());
            newCache.add(std::move(result));

            // Revert to indeterminate in case any more long running work occurs subsequently
            transform->setProgress(-1);
            transform->clearPhase();
            transform->setProgressable(nullptr);
        }

        if(_cancelled)
        {
            // We've been cancelled so rollback to our previous state
            _cache = std::move(oldCache);
            _addedOrChangedAttributeNames = std::move(oldAddedOrChangedAttributeNames);
            changedAttributeNames.clear();
            const auto* cachedGraph = _cache.graph();
            *this = (cachedGraph != nullptr ? *cachedGraph : *_source);

            // Remove any attributes that were added before the cancel occurred
            for(const auto& attributeName : u::setDifference(_graphModel->attributeNames(), fixedAttributeNames))
                _graphModel->removeAttribute(attributeName);

            _graphModel->addAttributes(_cache.attributes());

            _nodesState = _previousNodesState;
            _edgesState = _previousEdgesState;
        }
        else
        {
            _cache = std::move(newCache);
            _addedOrChangedAttributeNames = std::move(newAddedOrChangedAttributeNames);
        }
    });

    if(!changedAttributeNames.empty())
        emit attributeValuesChanged(changedAttributeNames);

    enableComponentManagement();

    emit graphChanged(this, _changeSignalsEmitted);
}

void TransformedGraph::setCurrentTransform(GraphTransform* currentTransform)
{
    const std::unique_lock<std::mutex> lock(_currentTransformMutex);
    _currentTransform = currentTransform;
}

void TransformedGraph::onTargetGraphChanged(const Graph*)
{
    // Let everything know what changed; note the signals won't necessarily happen in the order
    // in which the changes originally occurred, but adding nodes and edges, then removing edges
    // and nodes ensures that the receivers get a sane view at all times
    for(NodeId nodeId(0); nodeId < static_cast<int>(_nodesState.size()); ++nodeId)
    {
        if(!_previousNodesState[nodeId].added() && _nodesState[nodeId].added())
        {
            emit nodeAdded(this, nodeId);
            _changeSignalsEmitted = true;
        }
    }

    for(EdgeId edgeId(0); edgeId < static_cast<int>(_edgesState.size()); ++edgeId)
    {
        if(!_previousEdgesState[edgeId].added() && _edgesState[edgeId].added())
        {
            emit edgeAdded(this, edgeId);
            _changeSignalsEmitted = true;
        }
        else if(!_previousEdgesState[edgeId].removed() && _edgesState[edgeId].removed())
        {
            emit edgeRemoved(this, edgeId);
            _changeSignalsEmitted = true;
        }
    }

    for(NodeId nodeId(0); nodeId < static_cast<int>(_nodesState.size()); ++nodeId)
    {
        if(!_previousNodesState[nodeId].removed() && _nodesState[nodeId].removed())
        {
            emit nodeRemoved(this, nodeId);
            _changeSignalsEmitted = true;
        }
    }

    _previousNodesState = _nodesState;
    _previousEdgesState = _edgesState;

    _nodesState.resetElements();
    _edgesState.resetElements();
}
