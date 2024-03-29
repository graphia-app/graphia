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

#include "averageattributetransform.h"

#include "transform/transformedgraph.h"

#include "graph/graphmodel.h"

#include <memory>
#include <type_traits>

#include <QObject>
#include <QRegularExpression>

static Alert averageAttributeTransformConfigIsValid(const GraphModel& graphModel,
    const GraphTransformConfig& config)
{
    const auto& attributeNames = config.attributeNames();
    if(attributeNames.size() != 2)
        return {AlertType::Error, QObject::tr("Invalid parameters")};

    auto firstAttribute = graphModel.attributeValueByName(attributeNames.at(0));
    auto secondAttribute = graphModel.attributeValueByName(attributeNames.at(1));

    if(firstAttribute.elementType() != secondAttribute.elementType())
        return {AlertType::Error, QObject::tr("Attributes must both be node or edge attributes, not a mixture")};

    return {AlertType::None, {}};
}

void AverageAttributeTransform::apply(TransformedGraph& target)
{
    setPhase(QObject::tr("Averaging Attribute"));

    auto alert = averageAttributeTransformConfigIsValid(*_graphModel, config());
    if(alert._type != AlertType::None)
    {
        addAlert(alert);
        return;
    }

    auto sharedValuesAttributeName = config().attributeNames().at(0);
    auto sourceAttributeName = config().attributeNames().at(1);

    auto sharedValuesAttribute = _graphModel->attributeValueByName(sharedValuesAttributeName);
    auto sourceAttribute = _graphModel->attributeValueByName(sourceAttributeName);

    sourceAttributeName = Attribute::prettify(sourceAttributeName);
    sharedValuesAttributeName = Attribute::prettify(sharedValuesAttributeName);

    auto meanAttributeName = QObject::tr("Mean %1 of %2")
        .arg(sourceAttributeName, sharedValuesAttributeName);

    auto& meanAttribute = _graphModel->createAttribute(meanAttributeName)
        .setDescription(QObject::tr("The mean of %1 for each value of %2.")
        .arg(sourceAttributeName, sharedValuesAttributeName))
        .setFlag(AttributeFlag::AutoRange);

    auto setAverageFunction = [&](const auto& elementIds)
    {
        using E = typename std::remove_reference<decltype(elementIds)>::type::value_type;

        struct SharedValue
        {
            size_t _count = 0;
            double _total = 0.0;

            double mean() const { return _total / static_cast<double>(_count); }
        };

        std::map<QString, SharedValue> values;
        ElementIdArray<E, const SharedValue*> elementValues(target);

        for(auto elementId : elementIds)
        {
            auto value = sharedValuesAttribute.stringValueOf(elementId);

            if(!value.isEmpty())
            {
                auto* v = &values[value];
                v->_count++;
                v->_total += sourceAttribute.numericValueOf(elementId);
                elementValues[elementId] = v;
            }
        }

        ElementIdArray<E, double> averages(target);

        for(auto elementId : elementIds)
            averages[elementId] = elementValues[elementId]->mean();

        meanAttribute.setFloatValueFn([averages](E elementId)
        {
            return averages[elementId];
        });
    };

    switch(sourceAttribute.elementType())
    {
    case ElementType::Node: setAverageFunction(target.nodeIds()); break;
    case ElementType::Edge: setAverageFunction(target.edgeIds()); break;
    default: qFatal("Unhandled ElementType"); break;
    }
}

bool AverageAttributeTransformFactory::configIsValid(const GraphTransformConfig& graphTransformConfig) const
{
    return averageAttributeTransformConfigIsValid(*graphModel(), graphTransformConfig)._type == AlertType::None;
}

std::unique_ptr<GraphTransform> AverageAttributeTransformFactory::create(
    const GraphTransformConfig&) const
{
    return std::make_unique<AverageAttributeTransform>(*graphModel());
}
