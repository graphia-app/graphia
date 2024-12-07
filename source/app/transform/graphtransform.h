/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#ifndef GRAPHTRANSFORM_H
#define GRAPHTRANSFORM_H

#include "shared/graph/elementid.h"
#include "shared/graph/elementtype.h"

#include "shared/attributes/iattribute.h"

#include "shared/utils/flags.h"
#include "shared/utils/cancellable.h"
#include "shared/utils/progressable.h"

#include "transforminfo.h"
#include "graphtransformattributeparameter.h"
#include "graphtransformparameter.h"
#include "graphtransformconfig.h"

#include <QString>

#include <vector>
#include <memory>

class GraphModel;
class TransformedGraph;

class GraphTransform : public Progressable, public Cancellable
{
    friend class GraphModel;

public:
    GraphTransform() = default;
    ~GraphTransform() override = default;

    virtual void apply(TransformedGraph&) {}
    bool applyAndUpdate(TransformedGraph& target, const GraphModel& graphModel);

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

    int index() const { return _index; }

    // This is so that subclasses can access the config
    // Specifically, it is not a means to reconfigure an existing transform
    const GraphTransformConfig& config() const { return _config; }

    void setProgressable(Progressable* progressable);
    void setProgress(int percent) override;
    void setPhase(const QString& phase) override;

private:
    void setIndex(int index) { _index = index; }
    void setConfig(const GraphTransformConfig& config) { _config = config; }

private:
    mutable TransformInfo* _info = nullptr;
    bool _repeating = false;
    int _index = -1;
    GraphTransformConfig _config;
    Progressable* _progressable = nullptr;
};

struct DefaultVisualisation
{
    // This can also be set to the name of a string parameter, in which case
    // the parameter value will be used as the attribute name
    QString _attributeName;

    ElementType _elementType;
    ValueType _attributeValueType;
    Flags<AttributeFlag> _attributeFlags;
    QString _channel;
};

using DefaultVisualisations = std::vector<DefaultVisualisation>;

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
    virtual QString image() const;
    virtual QString category() const { return {}; }
    virtual ElementType elementType() const { return ElementType::None; }
    virtual GraphTransformAttributeParameters attributeParameters() const { return {}; }
    GraphTransformAttributeParameter attributeParameter(const QString& parameterName) const;
    virtual bool requiresCondition() const { return false; }
    virtual GraphTransformParameters parameters() const { return {}; }
    GraphTransformParameter parameter(const QString& parameterName) const;
    virtual DefaultVisualisations defaultVisualisations() const { return {}; }

    virtual bool configIsValid(const GraphTransformConfig&) const { return true; }
    void setMissingParametersToDefault(GraphTransformConfig& graphTransformConfig) const;
    virtual std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const = 0;

    GraphModel* graphModel() const { return _graphModel; }
};

using IdentityTransform = GraphTransform;

#endif // GRAPHTRANSFORM_H
