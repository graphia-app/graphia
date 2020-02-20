/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

static QString escape(const QString& string)
{
    return string.toHtmlEscaped();
}

bool GMLSaver::save()
{
    QFile file(_url.toLocalFile());
    file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
    int level = 0;

    size_t numElements = _graphModel->attributeNames().size() +
                       static_cast<size_t>(_graphModel->graph().numNodes()) +
                       static_cast<size_t>(_graphModel->graph().numEdges());
    size_t runningCount = 0;

    std::map<QString, QString> alphanumAttributeNames;
    _graphModel->mutableGraph().setPhase(QObject::tr("Attributes"));
    for(const auto& nodeAttributeName : _graphModel->attributeNames())
    {
        auto cleanName = nodeAttributeName;
        cleanName.remove(QRegularExpression(QStringLiteral("[^a-zA-Z\\d]")));
        if(cleanName.isEmpty())
            cleanName = QStringLiteral("Attribute");

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

        alphanumAttributeNames[nodeAttributeName] = cleanName;

        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / numElements));
    }

    QTextStream stream(&file);
    stream << "graph" << endl << "[" << endl;
    level++;

    auto attributes = [&](auto elementId, const std::vector<QString>& attributeNames)
    {
        for(const auto& attributeName : attributeNames)
        {
            const auto& attribute = _graphModel->attributeByName(attributeName);
            if(attribute->valueType() == ValueType::String)
            {
                QString escapedValue = escape(attribute->stringValueOf(elementId));
                stream << indent(level) << alphanumAttributeNames[attributeName] << " "
                       << QStringLiteral("\"%1\"").arg(escapedValue) << endl;
            }
            else if(attribute->valueType() & ValueType::Numerical)
            {
                auto value = attribute->numericValueOf(elementId);
                if(!std::isnan(value))
                {
                    stream << indent(level) << alphanumAttributeNames[attributeName] << " "
                           << value << endl;
                }
            }
        }
    };

    _graphModel->mutableGraph().setPhase(QObject::tr("Nodes"));
    for(auto nodeId : _graphModel->graph().nodeIds())
    {
        QString nodeName = escape(_graphModel->nodeName(nodeId));
        stream << indent(level) << "node" << endl;
        stream << indent(level) << "[" << endl;
        level++;
        stream << indent(level) << "id " << static_cast<int>(nodeId) << endl;
        QString labelString = QStringLiteral("\"%1\"").arg(nodeName);
        stream << indent(level) << "label " << labelString << endl;
        attributes(nodeId, _graphModel->attributeNames(ElementType::Node));
        stream << indent(--level) << "]" << endl; // node

        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / numElements));
    }

    _graphModel->mutableGraph().setPhase(QObject::tr("Edges"));
    for(auto edgeId : _graphModel->graph().edgeIds())
    {
        const auto& edge = _graphModel->graph().edgeById(edgeId);

        stream << indent(level) << "edge" << endl;
        stream << indent(level) << "[" << endl;
        level++;
        stream << indent(level) << "source " << static_cast<int>(edge.sourceId()) << endl;
        stream << indent(level) << "target " << static_cast<int>(edge.targetId()) << endl;
        attributes(edgeId, _graphModel->attributeNames(ElementType::Edge));
        stream << indent(--level) << "]" << endl; // edge

        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / numElements));
    }

    stream << indent(--level) << "]" << endl; // graph

    return true;
}
