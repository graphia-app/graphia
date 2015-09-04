#include "filtertransform.h"
#include "transformedgraph.h"

#include "../graph/componentmanager.h"

#include "../utils/utils.h"

#include <algorithm>

template<typename ElementId, typename FilterFn, typename AddFn, typename RemoveFn>
void filter(const std::vector<ElementId>& source, const std::vector<ElementId>& target,
            FilterFn filtered, AddFn add, RemoveFn remove)
{
    auto s = source.cbegin();
    auto sLast = source.cend();
    auto t = target.cbegin();
    auto tLast = target.cend();

    while(s != sLast)
    {
        if(t == tLast)
            break;

        if(*s < *t)
        {
            if(!filtered(*s))
                add(*s);

            s++;
        }
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
    {
        if(!filtered(*s))
            add(*s);

        s++;
    }

    while(t != tLast)
        remove(*t++);
}

template<typename ElementId> bool elementIdFiltered(std::vector<ElementFilterFn<ElementId>> filters, ElementId elementId)
{
    return std::any_of(filters.cbegin(), filters.cend(), [elementId](ElementFilterFn<ElementId> f) { return f(elementId); });
}

bool componentFiltered(std::vector<ComponentFilterFn> filters, const Graph& component)
{
    return std::any_of(filters.cbegin(), filters.cend(), [&component](ComponentFilterFn f) { return f(component); });
}

void FilterTransform::filterComponents(TransformedGraph& target) const
{
    if(!_componentFilters.empty())
    {
        ComponentManager componentManager(target);

        for(auto componentId : componentManager.componentIds())
        {
            auto component = componentManager.componentById(componentId);
            if(componentFiltered(_componentFilters, *component))
                target.removeNodes(component->nodeIds());
        }
    }
}

void FilterTransform::apply(const Graph& source, TransformedGraph& target) const
{
    target.reserve(source);

    filter(source.nodeIds(), target.nodeIds(),
        [this](NodeId nodeId) { return elementIdFiltered(_nodeFilters, nodeId); },
        [&target](NodeId nodeId) { u::checkEqual(target.addNode(nodeId), nodeId); },
        [&target](NodeId nodeId) { target.removeNode(nodeId); });

    filter(source.edgeIds(), target.edgeIds(),
        [this, &source, &target](EdgeId edgeId)
        {
            auto edge = source.edgeById(edgeId);
            // Check the nodes this edge connected haven't already been filtered
            return !target.containsNodeId(edge.sourceId()) ||
                   !target.containsNodeId(edge.targetId()) ||
                   elementIdFiltered(_edgeFilters, edgeId);
        },
        [&source, &target](EdgeId edgeId) { u::checkEqual(target.addEdge(source.edgeById(edgeId)), edgeId); },
        [&target](EdgeId edgeId) { target.removeEdge(edgeId); });

    filterComponents(target);
}

void FilterTransform::apply(TransformedGraph& target) const
{
    for(auto edgeId : target.edgeIds())
    {
        if(elementIdFiltered(_edgeFilters, edgeId))
            target.removeEdge(edgeId);
    }

    for(auto nodeId : target.nodeIds())
    {
        if(elementIdFiltered(_nodeFilters, nodeId))
            target.removeNode(nodeId);
    }

    filterComponents(target);
}
