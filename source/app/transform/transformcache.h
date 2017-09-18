#ifndef TRANSFORMCACHE_H
#define TRANSFORMCACHE_H

#include "graphtransformconfig.h"
#include "attributes/attribute.h"

#include <vector>

class MutableGraph;
class TransformedGraph;
class GraphModel;

class TransformCache
{
public:
    struct Result
    {
        Result() = default;

        Result(const Result& other) :
            _config(other._config),
            _graph(other._graph ? std::make_unique<MutableGraph>(*other._graph) : nullptr),
            _newAttributes(other._newAttributes)
        {}

        Result(Result&& other) :
            _config(std::move(other._config)),
            _graph(std::move(other._graph)),
            _newAttributes(std::move(other._newAttributes))
        {}

        Result& operator=(Result&& other)
        {
            _config = std::move(other._config);
            _graph = std::move(other._graph);
            _newAttributes = std::move(other._newAttributes);

            return *this;
        }

        bool changesGraph() const { return _graph != nullptr; }
        bool isApplicable() const { return changesGraph() || !_newAttributes.empty(); }

        GraphTransformConfig _config;
        std::unique_ptr<MutableGraph> _graph;
        std::map<QString, Attribute> _newAttributes;
    };

    using ResultSet = std::vector<Result>;

private:
    GraphModel* _graphModel;
    std::vector<ResultSet> _cache;

public:
    TransformCache(GraphModel& graphModel);
    TransformCache(const TransformCache& other) = default;
    TransformCache(TransformCache&& other) = default;
    TransformCache& operator=(TransformCache&& other) noexcept;

    bool empty() const { return _cache.empty(); }
    void clear() { _cache.clear(); }
    void add(Result&& result);
    void attributeAdded(const QString& attributeName);
    Result apply(const GraphTransformConfig& config, TransformedGraph& graph);

    const MutableGraph* graph() const;
    std::map<QString, Attribute> attributes() const;
};

#endif // TRANSFORMCACHE_H
