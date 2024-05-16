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

#ifndef TRANSFORMCACHE_H
#define TRANSFORMCACHE_H

#include "graphtransformconfig.h"
#include "app/graph/mutablegraph.h"
#include "app/attributes/attribute.h"

#include <vector>

class TransformedGraph;
class GraphModel;

class TransformCache
{
public:
    struct Result
    {
        Result() = default;

        Result(const Result& other) :
            _index(other._index),
            _config(other._config),
            _graph(other._graph ? std::make_unique<MutableGraph>(*other._graph) : nullptr),
            _addedOrChangedAttributes(other._addedOrChangedAttributes)
        {}

        Result(Result&& other) noexcept :
            _index(other._index),
            _config(std::move(other._config)),
            _graph(std::move(other._graph)),
            _addedOrChangedAttributes(std::move(other._addedOrChangedAttributes))
        {}

        Result& operator=(Result&& other) noexcept
        {
            _index = other._index;
            _config = std::move(other._config);
            _graph = std::move(other._graph);
            _addedOrChangedAttributes = std::move(other._addedOrChangedAttributes);

            return *this;
        }

        bool changesGraph() const { return _graph != nullptr; }
        bool wasApplied() const { return changesGraph() || !_addedOrChangedAttributes.empty(); }

        std::vector<QString> referencedAttributeNames() const
        {
            return _config.referencedAttributeNames();
        }

        int _index = -1;
        GraphTransformConfig _config;
        std::unique_ptr<MutableGraph> _graph;
        std::map<QString, Attribute> _addedOrChangedAttributes;
    };

    using ResultSet = std::vector<Result>;

private:
    bool lastResultChangesGraph() const;
    bool lastResultChangedAnyOf(const std::vector<QString>& attributeNames) const;
    std::vector<QString> attributesChangedByLastResult() const;

    GraphModel* _graphModel;
    std::vector<ResultSet> _cache;

public:
    explicit TransformCache(GraphModel& graphModel);
    TransformCache(const TransformCache& other) = default;
    TransformCache(TransformCache&& other) noexcept = default;
    TransformCache& operator=(TransformCache&& other) noexcept;

    bool empty() const { return _cache.empty(); }
    void clear() { _cache.clear(); }
    void add(Result&& result);
    void attributeAddedOrChanged(const QString& attributeName);
    Result apply(int index, const GraphTransformConfig& config, TransformedGraph& graph);

    const MutableGraph* graph() const;
    std::map<QString, Attribute> attributes() const;
};

#endif // TRANSFORMCACHE_H
