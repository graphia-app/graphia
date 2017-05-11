#include "transformcache.h"

#include "graph/graphmodel.h"
#include "graph/mutablegraph.h"
#include "transform/transformedgraph.h"

#include "shared/utils/iterator_range.h"

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

void TransformCache::add(TransformCache::Result&& result)
{
    if(_cache.empty() || setChangesGraph(_cache.back()))
        _cache.emplace_back();

    TransformCache::ResultSet& resultSet = _cache.back();
    resultSet.emplace_back(std::move(result));
}

TransformCache::Result TransformCache::apply(const GraphTransformConfig& config, TransformedGraph& graph)
{
    TransformCache::Result result;
    result._config = config;

    if(_cache.empty())
        return result;

    auto& resultSet = _cache.front();

    auto it = resultSet.begin();
    for(auto& cachedResult : make_iterator_range(it, resultSet.end()))
    {
        if(cachedResult._config == config)
        {
            // Apply the cached result
            _graphModel->addAttributes(cachedResult._newAttributes);
            if(cachedResult._graph != nullptr)
                graph = *(cachedResult._graph);

            result = std::move(cachedResult);

            if(result._graph != nullptr)
            {
                // If the graph was changed, remove the entire set...
                _cache.erase(_cache.begin());
            }
            else
            {
                // ...otherwise just remove the specific entry
                resultSet.erase(it);
            }

            break;
        }
    }

    return result;
}
