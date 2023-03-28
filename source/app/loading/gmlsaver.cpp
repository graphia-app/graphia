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

#include "gmlsaver.h"

#include "shared/graph/imutablegraph.h"
#include "ui/document.h"

#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

using namespace Qt::Literals::StringLiterals;

bool GMLSaver::save()
{
    QFile file(_url.toLocalFile());
    file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
    int level = 0;

    const size_t numElements = _graphModel->attributeNames().size() +
        _graphModel->graph().numNodes() +
        _graphModel->graph().numEdges();
    size_t runningCount = 0;

    auto escape = [](const QString& string)
    {
        return string.toHtmlEscaped();
    };

    std::map<QString, QString> alphanumAttributeNames;
    setPhase(QObject::tr("Attributes"));
    for(const auto& attributeName : _graphModel->attributeNames())
    {
        auto cleanName = attributeName;
        static const QRegularExpression re(uR"([^a-zA-Z\d])"_s);
        cleanName.remove(re);
        if(cleanName.isEmpty())
            cleanName = u"Attribute"_s;

        // Duplicate attributenames can occur when removing non alphanum chars, append a number.
        if(alphanumAttributeNames.find(cleanName) != alphanumAttributeNames.end())
        {
            int suffix = 1;
            while(true)
            {
                auto uniqueCleanName = cleanName;
                uniqueCleanName.append(QString::number(suffix++));
                if(alphanumAttributeNames.find(uniqueCleanName) == alphanumAttributeNames.end())
                {
                    cleanName = uniqueCleanName;
                    break;
                }
            }
        }

        if(cleanName.at(0).isDigit())
            cleanName = u"Attribute"_s + cleanName;

        alphanumAttributeNames[attributeName] = cleanName;

        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / numElements));
    }

    QTextStream stream(&file);
    stream << "graph\n[\n";
    level++;

    auto attributes = [&](auto elementId, const std::vector<QString>& attributeNames)
    {
        for(const auto& attributeName : attributeNames)
        {
            const auto* attribute = _graphModel->attributeByName(attributeName);
            if(attribute->hasParameter())
                continue;

            if(attribute->valueType() == ValueType::String)
            {
                const QString escapedValue = escape(attribute->stringValueOf(elementId));
                stream << indent(level) << alphanumAttributeNames[attributeName] << " "
                       << u"\"%1\""_s.arg(escapedValue) << "\n";
            }
            else if(attribute->valueType() & ValueType::Numerical)
            {
                auto value = attribute->numericValueOf(elementId);
                if(!std::isnan(value))
                {
                    stream << indent(level) << alphanumAttributeNames[attributeName] << " "
                           << value << "\n";
                }
            }
        }
    };

    setPhase(QObject::tr("Nodes"));
    for(auto nodeId : _graphModel->graph().nodeIds())
    {
        const QString nodeName = escape(_graphModel->nodeName(nodeId));
        stream << indent(level) << "node\n";
        stream << indent(level) << "[\n";
        level++;
        stream << indent(level) << "id " << static_cast<int>(nodeId) << "\n";
        const QString labelString = u"\"%1\""_s.arg(nodeName);
        stream << indent(level) << "label " << labelString << "\n";
        attributes(nodeId, _graphModel->attributeNames(ElementType::Node));
        stream << indent(--level) << "]\n"; // node

        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / numElements));
    }

    setPhase(QObject::tr("Edges"));
    for(auto edgeId : _graphModel->graph().edgeIds())
    {
        const auto& edge = _graphModel->graph().edgeById(edgeId);

        stream << indent(level) << "edge\n";
        stream << indent(level) << "[\n";
        level++;
        stream << indent(level) << "source " << static_cast<int>(edge.sourceId()) << "\n";
        stream << indent(level) << "target " << static_cast<int>(edge.targetId()) << "\n";
        attributes(edgeId, _graphModel->attributeNames(ElementType::Edge));
        stream << indent(--level) << "]\n"; // edge

        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / numElements));
    }

    stream << indent(--level) << "]\n"; // graph

    return true;
}
