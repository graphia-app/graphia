#include "transformcache.h"

#include "graph/graphmodel.h"
#include "graph/mutablegraph.h"
#include "transform/transformedgraph.h"

#include <algorithm>

TransformCache::TransformCache(GraphModel& graphModel) :
    _graphModel(&graphModel)
{}

TransformCache::TransformCache(TransformCache&& other) :
    _graphModel(other._graphModel),
    _cache(std::move(other._cache))
{}

TransformCache& TransformCache::operator=(TransformCache&& other)
{
    _graphModel = other._graphModel;
    _cache = std::move(other._cache);
    return *this;
}

static bool setChangesGraph(const TransformCache::ResultSet& entrySet)
{
    return std::any_of(entrySet.begin(), entrySet.end(), [](const auto& entry)
    {
        return entry._graph != nullptr;
    });
}

TransformCache::Result& TransformCache::createEntry()
{
    if(_cache.empty() || setChangesGraph(_cache.back()))
        _cache.emplace_back();

    TransformCache::ResultSet& entrySet = _cache.back();
    entrySet.emplace_back();

    return entrySet.back();
}

TransformCache::Result TransformCache::apply(const GraphTransformConfig& config, TransformedGraph& graph)
{
    TransformCache::Result result;
    result._config = config;

    if(_cache.empty())
        return result;

    auto& entrySet = _cache.front();

    for(auto it = entrySet.begin(); it != entrySet.end(); it++)
    {
        if(it->_config == config)
        {
            // Apply the cached result
            _graphModel->addAttributes(it->_newAttributes);
            if(it->_graph != nullptr)
                graph = *(it->_graph);

            // Assign the result to the new cache
            result = std::move(*it);

            if(result._graph != nullptr)
            {
                // If the graph was changed, remove the entire set...
                _cache.erase(_cache.begin());
            }
            else
            {
                // ...otherwise just remove the specific entry
                entrySet.erase(it);
            }

            break;
        }
    }

    return result;
}
