#include "transformcache.h"

#include "graph/graphmodel.h"
#include "graph/mutablegraph.h"
#include "transform/transformedgraph.h"

#include "shared/utils/iterator_range.h"
#include "shared/utils/container.h"

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

bool TransformCache::lastResultChangesGraph() const
{
    return std::any_of(_cache.back().begin(), _cache.back().end(), [](const auto& result)
    {
        return result._graph != nullptr;
    });
}

bool TransformCache::lastResultCreatedAnyOf(const std::vector<QString>& attributeNames) const
{
    return !u::setIntersection(attributesCreatedByLastResult(), attributeNames).empty();
}

std::vector<QString> TransformCache::attributesCreatedByLastResult() const
{
    std::vector<QString> attributeNames;

    for(const auto& result : _cache.back())
    {
        auto resultAttributeNames = u::keysFor(result._newAttributes);
        attributeNames.insert(attributeNames.end(), resultAttributeNames.begin(), resultAttributeNames.end());
    }

    return attributeNames;
}

void TransformCache::add(TransformCache::Result&& result)
{
    if(_cache.empty() || lastResultChangesGraph() || lastResultCreatedAnyOf(result.referencedAttributeNames()))
        _cache.emplace_back();

    _cache.back().emplace_back(std::move(result));
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
            if(u::contains(resultIt->_config.referencedAttributeNames(), attributeName))
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

    auto it = std::find_if(resultSet.begin(), resultSet.end(),
    [&](const auto& cachedResult)
    {
        return cachedResult._config == config;
    });

    if(it != resultSet.end())
    {
        auto& cachedResult = *it;

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
    }

    return result;
}

const MutableGraph* TransformCache::graph() const
{
    // Return the last graph in the cache
    for(const auto& resultSet : make_iterator_range(_cache.rbegin(), _cache.rend()))
    {
        const auto& cachedResults = make_iterator_range(resultSet.rbegin(), resultSet.rend());
        auto it = std::find_if(cachedResults.begin(), cachedResults.end(),
        [](const auto& cachedResult)
        {
            return cachedResult._graph != nullptr;
        });

        if(it != cachedResults.end())
            return it->_graph.get();
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
