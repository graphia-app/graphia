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

#include "pairwisesaver.h"

#include "shared/attributes/iattribute.h"
#include "shared/graph/igraph.h"
#include "shared/graph/igraphmodel.h"

#include <QFile>
#include <QLocale>
#include <QObject>
#include <QTextStream>

#include <algorithm>
#include <cstddef>

using namespace Qt::Literals::StringLiterals;

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

    auto isWeight = [](const QString& attributeName) { return attributeName.contains(u"weight"_s, Qt::CaseInsensitive); };
    auto isValue = [](const QString& attributeName) { return attributeName.contains(u"value"_s, Qt::CaseInsensitive); };

    edgeAttributeNames.erase(std::ranges::remove_if(edgeAttributeNames,
    [&isWeight, &isValue](const auto& attributeName)
    {
        return !isWeight(attributeName) && !isValue(attributeName);
    }).begin(), edgeAttributeNames.end());

    std::ranges::sort(edgeAttributeNames,
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
    if(!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
        return false;

    QTextStream stream(&file);
    const size_t elementCount = _graphModel->graph().numEdges() + _graphModel->graph().numNodes();
    size_t runningCount = 0;

    auto nameOf = [this](NodeId nodeId)
    {
        auto name = _graphModel->nodeName(nodeId);
        name.replace(u"\""_s, u"\"\""_s);

        if(name.isEmpty())
            name = QString::number(static_cast<int>(nodeId));

        return name;
    };

    auto edgeWeightAttributeName = findEdgeWeightAttributeName(_graphModel);
    const auto* weightAttribute = !edgeWeightAttributeName.isEmpty() ?
        _graphModel->attributeByName(edgeWeightAttributeName) : nullptr;

    setPhase(QObject::tr("Edges"));
    for(auto edgeId : _graphModel->graph().edgeIds())
    {
        const auto& edge = _graphModel->graph().edgeById(edgeId);
        auto sourceName = nameOf(edge.sourceId());
        auto targetName = nameOf(edge.targetId());

        if(weightAttribute != nullptr && !weightAttribute->valueMissingOf(edgeId))
        {
            stream << u"\"%1\" \"%2\" %3\n"_s.arg(sourceName, targetName,
                QString::number(weightAttribute->floatValueOf(edgeId), 'g', QLocale::FloatingPointShortest));
        }
        else
            stream << u"\"%1\" \"%2\"\n"_s.arg(sourceName, targetName);

        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / elementCount));
    }

    // A row with only one name is a node with no edges
    setPhase(QObject::tr("Nodes"));
    for(auto nodeId : _graphModel->graph().nodeIds())
    {
        if(_graphModel->graph().nodeById(nodeId).degree() == 0)
            stream << u"\"%1\"\n"_s.arg(nameOf(nodeId));

        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / elementCount));
    }

    return true;
}
