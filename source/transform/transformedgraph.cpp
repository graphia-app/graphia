#include "transformedgraph.h"

#include "identitytransform.h"

#include "../utils/cpp1x_hacks.h"

#include <functional>

TransformedGraph::TransformedGraph(const Graph& source) :
    Graph(true),
    _source(&source)
{
    connect(_source, &Graph::graphChanged, this, &TransformedGraph::onGraphChanged, Qt::DirectConnection);

    // Forward the signals, but with the graph argument replaced with this
    connect(&_target, &Graph::graphWillChange,   [this](const Graph*)                   { emit graphWillChange(this); });
    connect(&_target, &Graph::nodeWillBeRemoved, [this](const Graph*, const Node* node) { emit nodeWillBeRemoved(this, node); });
    connect(&_target, &Graph::nodeAdded,         [this](const Graph*, const Node* node) { emit nodeAdded(this, node); });
    connect(&_target, &Graph::edgeWillBeRemoved, [this](const Graph*, const Edge* edge) { emit edgeWillBeRemoved(this, edge); });
    connect(&_target, &Graph::edgeAdded,         [this](const Graph*, const Edge* edge) { emit edgeAdded(this, edge); });
    connect(&_target, &Graph::graphChanged,      [this](const Graph*)                   { emit graphChanged(this); });

    setTransform(std::make_unique<IdentityTransform>());
}

void TransformedGraph::setTransform(std::unique_ptr<GraphTransform> graphTransform)
{
    _graphTransform = std::move(graphTransform);
    rebuild();
}

template<typename T, typename FilterFunction, typename AddFunction, typename RemoveFunction>
void synchronise(const std::vector<T>& source, const std::vector<T>& target,
                 FilterFunction filtered, AddFunction add, RemoveFunction remove)
{
    auto s = source.cbegin();
    auto sLast = source.cend();
    auto t = target.cbegin();
    auto tLast = target.cend();

    while(s != sLast)
    {
        if(t == tLast)
            break;

        if(filtered(*s))
            s++;

        if(*s < *t)
            add(*s++);
        else
        {
            if(*t < *s)
                remove(*t);
            else
                s++;

            t++;
        }
    }

    while(s != sLast)
        add(*s++);

    while(t != tLast)
        remove(*t++);
}

void TransformedGraph::rebuild()
{
    MutableGraph::ScopedTransaction transaction(_target);

    synchronise(_source->nodeIds(), _target.nodeIds(),
                [this](NodeId nodeId) { return _graphTransform->nodeIsFiltered(_source->nodeById(nodeId)); },
                [this](NodeId nodeId) { _target.addNode(nodeId); },
                [this](NodeId nodeId) { _target.removeNode(nodeId); });

    synchronise(_source->edgeIds(), _target.edgeIds(),
                [this](EdgeId edgeId) { return _graphTransform->edgeIsFiltered(_source->edgeById(edgeId)); },
                [this](EdgeId edgeId) { _target.addEdge(_source->edgeById(edgeId)); },
                [this](EdgeId edgeId) { _target.removeEdge(edgeId); });

    _graphTransform->transform(*_source, _target);
}

void TransformedGraph::onGraphChanged(const Graph*)
{
    rebuild();
}
