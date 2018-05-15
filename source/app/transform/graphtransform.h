#ifndef GRAPHTRANSFORM_H
#define GRAPHTRANSFORM_H

#include "shared/graph/elementid.h"
#include "shared/graph/elementtype.h"

#include "shared/utils/cancellable.h"

#include "transforminfo.h"
#include "graphtransformattributeparameter.h"
#include "graphtransformparameter.h"
#include "graphtransformconfig.h"

#include <QString>

#include <vector>
#include <map>
#include <memory>

class Graph;
class GraphComponent;
class GraphModel;
class TransformedGraph;
class Attribute;

class GraphTransform : public Cancellable
{
    friend class GraphModel;

public:
    GraphTransform() = default;
    ~GraphTransform() override = default;

    // Return value indicates that the graph changed
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

    // This is so that subclasses can access the config
    // Specifically, it is not a means to reconfigure an existing transform
    const GraphTransformConfig& config() const { return _config; }
    const std::vector<QString>& attributes() const { return _attributes; }

private:
    void setConfig(const GraphTransformConfig& config) { _config = config; }
    void setAttributes(const std::vector<QString>& attributes) { _attributes = attributes; }

protected:
    bool hasUnknownAttributes(const std::vector<QString>& referencedAttributes,
                              const std::vector<QString>& availableAttributes) const;

private:
    mutable TransformInfo* _info = nullptr;
    bool _repeating = false;
    GraphTransformConfig _config;
    std::vector<QString> _attributes;
};

struct DeclaredAttribute
{
    ValueType _valueType;
    QString _defaultVisualisation;
};

using DeclaredAttributes = std::map<QString, DeclaredAttribute>;

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
    virtual GraphTransformAttributeParameters attributeParameters() const { return {}; }
    virtual bool requiresCondition() const { return false; }
    virtual GraphTransformParameters parameters() const { return {}; }
    virtual DeclaredAttributes declaredAttributes() const { return {}; }

    virtual std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const = 0;

    GraphModel* graphModel() const { return _graphModel; }
};

using IdentityTransform = GraphTransform;

#endif // GRAPHTRANSFORM_H
