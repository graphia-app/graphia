#include "transformcache.h"

#include "graph/graphmodel.h"
#include "graph/mutablegraph.h"
#include "transform/transformedgraph.h"

#include "shared/utils/iterator_range.h"

#include <algorithm>

TransformCache::TransformCache(GraphModel& graphModel) :
    _graphModel(&graphModel)
{}

TransformCache& TransformCache::operator=(TransformCache&& other) noexcept
{
    _graphModel = other._graphModel;
    _cache = std::move(other._cache);
    return *this;
}

static bool setChangesGraph(const TransformCache::ResultSet& resultSet)
{
    return std::any_of(resultSet.begin(), resultSet.end(), [](const auto& result)
    {
        return result._graph != nullptr;
    });
}

void TransformCache::add(TransformCache::Result&& result)
{
    if(_cache.empty() || setChangesGraph(_cache.back()))
        _cache.emplace_back();

    TransformCache::ResultSet* resultSet = &_cache.back();

    // If this result references attributes that were created in the last result set,
    // then move all said attribute creating results to their own set
    const auto& referencedAttributeNames = result._config.attributeNames();
    TransformCache::ResultSet attributeCreatingResultSet;
    for(auto& cachedResult : *resultSet)
    {
        if(!u::setIntersection(u::keysFor(cachedResult._newAttributes), referencedAttributeNames).empty())
            attributeCreatingResultSet.emplace_back(std::move(cachedResult));
    }

    if(!attributeCreatingResultSet.empty())
    {
        // Insert the attribute creating set before the last
        _cache.insert(_cache.end() - 1, std::move(attributeCreatingResultSet));
        resultSet = &_cache.back();

        // Get rid of any results that have been moved from and won't do anything when applied
        auto it = resultSet->begin();
        while(it != resultSet->end())
        {
              if(!it->isApplicable())
                  it = resultSet->erase(it);
              else
                  ++it;
        }
    }

    resultSet->emplace_back(std::move(result));
}

void TransformCache::attributeAdded(const QString& attributeName)
{
    // When an attribute is added its values may differ to previous incarnations, so
    // invalidate any subsequent entries that depend on it
    // Furthermore, if any entries are /creating/ the same attribute that we're adding,
    // invalidate those too as their names will need to be regenerated
    auto resultSetIt = _cache.begin();
    auto resultSetEnd = _cache.end();
    for(; resultSetIt != resultSetEnd; ++resultSetIt)
    {
        auto resultIt = resultSetIt->begin();
        auto resultEnd = resultSetIt->end();
        for(; resultIt != resultEnd; ++resultIt)
        {
            // Depends on attributeName
            if(u::contains(resultIt->_config.attributeNames(), attributeName))
            {
                resultSetIt->erase(resultIt, resultEnd);
                break;
            }

            // Creates attributeName
            if(u::contains(resultIt->_newAttributes, attributeName))
            {
                resultSetIt->erase(resultIt, resultEnd);
                break;
            }
        }

        // All subsequent results are now also invalid
        if(resultIt != resultEnd)
        {
            _cache.erase(resultSetIt, resultSetEnd);
            break;
        }
    }
}

TransformCache::Result TransformCache::apply(const GraphTransformConfig& config, TransformedGraph& graph)
{
    TransformCache::Result result;
    result._config = config;

    if(_cache.empty())
        return result;

    auto& resultSet = _cache.front();

    auto it = resultSet.begin();
    for(auto& cachedResult : resultSet)
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
                // ...otherwise just remove the specific result
                resultSet.erase(it);

                // If that was the last result, remove the set as well
                if(resultSet.empty())
                    _cache.erase(_cache.begin());
            }

            break;
        }

        ++it;
    }

    return result;
}

const MutableGraph* TransformCache::graph() const
{
    // Return the last graph in the cache
    for(const auto& resultSet : make_iterator_range(_cache.rbegin(), _cache.rend()))
    {
        for(const auto& cachedResult : make_iterator_range(resultSet.rbegin(), resultSet.rend()))
        {
            if(cachedResult._graph != nullptr)
                return cachedResult._graph.get();
        }
    }

    return nullptr;
}

std::map<QString, Attribute> TransformCache::attributes() const
{
    std::map<QString, Attribute> map;

    for(const auto& resultSet : _cache)
    {
        for(const auto& cachedResult : resultSet)
        {
            const auto& newAttributes = cachedResult._newAttributes;
            map.insert(newAttributes.begin(), newAttributes.end());
        }
    }

    return map;
}
