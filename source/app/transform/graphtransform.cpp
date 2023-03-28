/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#include "graphtransform.h"
#include "transformedgraph.h"

#include "graph/graph.h"
#include "graph/graphmodel.h"

#include "shared/commands/icommand.h"

#include "shared/utils/container.h"

using namespace Qt::Literals::StringLiterals;

static bool hasUnknownAttributes(const std::vector<QString>& attributeNames,
    const GraphModel& graphModel, const GraphTransform& transform)
{
    bool unknownAttributes = false;

    for(const auto& attributeName : attributeNames)
    {
        if(!graphModel.attributeExists(attributeName))
        {
            transform.addAlert(AlertType::Error, QObject::tr(R"(Unknown Attribute: "%1")").arg(attributeName));
            unknownAttributes = true;
        }
    }

    return unknownAttributes;
}

static bool hasInvalidAttributes(const std::vector<QString>& attributeNames,
    const GraphModel& graphModel, const GraphTransform& transform)
{
    const bool invalidAttributes =
    std::any_of(attributeNames.begin(), attributeNames.end(),
    [&graphModel](const auto& attributeName)
    {
        return !graphModel.attributeIsValid(attributeName);
    });

    if(invalidAttributes)
    {
        transform.addAlert(AlertType::Error, QObject::tr("One or more invalid attributes"));
        return true;
    }

    return false;
}

bool GraphTransform::applyAndUpdate(TransformedGraph& target, const GraphModel& graphModel)
{
    bool anyChange = false;
    bool change = false;

    do
    {
        target.resetChangeOccurred({});
        clearPhase();

        auto attributeNames = config().referencedAttributeNames();

        if(hasUnknownAttributes(attributeNames, graphModel, *this))
            continue;

        if(hasInvalidAttributes(attributeNames, graphModel, *this))
            continue;

        apply(target);
        target.update();
        change = target.changeOccurred({});
        anyChange = anyChange || change;
    } while(repeating() && change && !cancelled());

    return anyChange;
}

void GraphTransform::setProgressable(Progressable* progressable)
{
    _progressable = progressable;
}

void GraphTransform::setProgress(int percent)
{
    if(_progressable != nullptr)
        _progressable->setProgress(percent);
}

void GraphTransform::setPhase(const QString& phase)
{
    if(_progressable != nullptr)
        _progressable->setPhase(phase);
}

QString GraphTransformFactory::image() const
{
    if(category() == QObject::tr("Attributes"))
        return u"qrc:///transforms/images/attributes.svg"_s;

    if(category() == QObject::tr("Clustering"))
        return u"qrc:///transforms/images/clustering.svg"_s;

    if(category() == QObject::tr("Edge Reduction"))
        return u"qrc:///transforms/images/edgereduction.svg"_s;

    if(category() == QObject::tr("Filters"))
        return u"qrc:///transforms/images/filters.svg"_s;

    if(category() == QObject::tr("Metrics"))
        return u"qrc:///transforms/images/metrics.svg"_s;

    if(category() == QObject::tr("Structural"))
        return u"qrc:///transforms/images/structural.svg"_s;

    return u"qrc:///transforms/images/default.svg"_s;
}

GraphTransformAttributeParameter GraphTransformFactory::attributeParameter(const QString& parameterName) const
{
    const auto& p = attributeParameters();
    auto it = std::find_if(p.begin(), p.end(),
    [&parameterName](const auto& attributeParameter)
    {
        return attributeParameter.name() == parameterName;
    });

    if(it != p.end())
        return *it;

    return {};
}

GraphTransformParameter GraphTransformFactory::parameter(const QString& parameterName) const
{
    const auto& p = parameters();
    auto it = std::find_if(p.begin(), p.end(),
    [&parameterName](const auto& parameter)
    {
        return parameter.name() == parameterName;
    });

    if(it != p.end())
        return *it;

    return {};
}

void GraphTransformFactory::setMissingParametersToDefault(GraphTransformConfig& graphTransformConfig) const
{
    for(const auto& parameter : parameters())
    {
        if(!graphTransformConfig.hasParameter(parameter.name()))
        {
            const auto& v = parameter.initialValue();
            switch(v.typeId())
            {
            case QMetaType::Int:
                graphTransformConfig.setParameterValue(parameter.name(), v.toInt());
                break;

            case QMetaType::Double:
                graphTransformConfig.setParameterValue(parameter.name(), v.toDouble());
                break;

            case QMetaType::QString:
                graphTransformConfig.setParameterValue(parameter.name(), v.toString());
                break;

            case QMetaType::QStringList:
                graphTransformConfig.setParameterValue(parameter.name(), v.toStringList().first());
                break;

            default:
                break;
            }
        }
    }
}
