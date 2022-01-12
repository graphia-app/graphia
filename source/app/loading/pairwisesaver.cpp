/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#include "pairwisesaver.h"

#include "shared/attributes/iattribute.h"
#include "shared/graph/igraph.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"
#include "ui/document.h"

#include <QFile>
#include <QRegularExpression>
#include <QString>
#include <QTextStream>

// Find an attribute that looks like an edge weight
static QString findEdgeWeightAttributeName(const IGraphModel* graphModel)
{
    auto edgeAttributeNames = graphModel->attributeNamesMatching(
    [](const IAttribute& attribute)
    {
        return !attribute.hasParameter() &&
            attribute.elementType() == ElementType::Edge &&
            attribute.valueType() & ValueType::Numerical;
    });

    auto isWeight = [](const QString& attributeName) { return attributeName.contains(QStringLiteral("weight"), Qt::CaseInsensitive); };
    auto isValue = [](const QString& attributeName) { return attributeName.contains(QStringLiteral("value"), Qt::CaseInsensitive); };

    edgeAttributeNames.erase(std::remove_if(edgeAttributeNames.begin(), edgeAttributeNames.end(),
    [&isWeight, &isValue](const auto& attributeName)
    {
        return !isWeight(attributeName) && !isValue(attributeName);
    }), edgeAttributeNames.end());

    std::sort(edgeAttributeNames.begin(), edgeAttributeNames.end(),
    [&isWeight, &isValue](const auto& a, const auto& b)
    {
        auto aIsWeight = isWeight(a);
        auto bIsWeight = isWeight(b);

        if(aIsWeight && bIsWeight) return a < b;
        if(aIsWeight) return true;
        if(bIsWeight) return false;

        auto aIsValue = isValue(a);
        auto bIsValue = isValue(b);

        if(aIsValue && bIsValue) return a < b;
        if(aIsValue) return true;
        if(bIsValue) return false;

        return a < b;
    });

    if(edgeAttributeNames.empty())
        return {};

    return edgeAttributeNames.front();
}

bool PairwiseSaver::save()
{
    QFile file(_url.toLocalFile());
    file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);

    QTextStream stream(&file);
    int edgeCount = _graphModel->graph().numEdges();
    int runningCount = 0;

    auto escape = [](QString string)
    {
        string.replace(QStringLiteral(R"(")"), QStringLiteral(R"(\")"));
        return string;
    };

    auto edgeWeightAttributeName = findEdgeWeightAttributeName(_graphModel);

    _graphModel->mutableGraph().setPhase(QObject::tr("Edges"));
    for(auto edgeId : _graphModel->graph().edgeIds())
    {
        const auto& edge = _graphModel->graph().edgeById(edgeId);
        auto sourceName = escape(_graphModel->nodeName(edge.sourceId()));
        auto targetName = escape(_graphModel->nodeName(edge.targetId()));

        if(sourceName.isEmpty())
            sourceName = QString::number(static_cast<int>(edge.sourceId()));

        if(targetName.isEmpty())
            targetName = QString::number(static_cast<int>(edge.targetId()));

        if(!edgeWeightAttributeName.isEmpty())
        {
            const auto* attribute = _graphModel->attributeByName(edgeWeightAttributeName);
            stream << QStringLiteral(R"("%1" "%2" %3)""\n").arg(sourceName, targetName)
                .arg(attribute->floatValueOf(edgeId));
        }
        else
            stream << QStringLiteral(R"(""%1" "%2")""\n").arg(sourceName, targetName);

        runningCount++;
        setProgress(runningCount * 100 / edgeCount);
    }

    return true;
}
