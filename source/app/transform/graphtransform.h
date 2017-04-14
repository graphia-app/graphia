#ifndef GRAPHTRANSFORM_H
#define GRAPHTRANSFORM_H

#include "shared/graph/elementid.h"
#include "graph/elementtype.h"
#include "transforminfo.h"
#include "graphtransformconfig.h"

#include <memory>

class Graph;
class GraphComponent;
class GraphModel;
class TransformedGraph;
class Attribute;

class GraphTransform
{
public:
    GraphTransform() = default;
    virtual ~GraphTransform() = default;

    // In some circumstances it may be a performance win to reimplement this instead of going
    // for the inplace transform version
    virtual bool applyFromSource(const Graph& source, TransformedGraph& target) const;

    virtual bool apply(TransformedGraph&) const { return false; }
    bool applyAndUpdate(TransformedGraph& target) const;

    bool repeating() const { return _repeating; }
    void setRepeating(bool repeating) { _repeating = repeating; }

    template<typename... Args>
    void addAlert(Args&&... args) const
    {
        if(_info != nullptr)
            _info->addAlert(std::forward<Args>(args)...);
    }

    const TransformInfo* info() const { return _info; }
    void setInfo(TransformInfo* info) { _info = info; }

protected:
    bool hasUnknownAttributes(const std::vector<QString>& referencedAttributes,
                               const std::vector<QString>& availableAttributes) const;

private:
    mutable TransformInfo* _info = nullptr;
    bool _repeating = false;
};

class GraphTransformFactory
{
private:
    GraphModel* _graphModel = nullptr;

public:
    explicit GraphTransformFactory(GraphModel* graphModel) :
        _graphModel(graphModel)
    {}

    virtual ~GraphTransformFactory() = default;

    virtual QString description() const = 0;

    virtual ElementType elementType() const { return ElementType::None; }
    virtual std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const = 0;

    GraphModel* graphModel() const { return _graphModel; }
};

using IdentityTransform = GraphTransform;

#endif // GRAPHTRANSFORM_H
