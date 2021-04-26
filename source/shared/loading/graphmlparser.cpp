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

#include "graphmlparser.h"

#include "shared/graph/igraphmodel.h"
#include "shared/graph/elementid_debug.h"
#include "shared/graph/imutablegraph.h"
#include "shared/utils/container.h"

#include <QXmlStreamReader>
#include <QFile>
#include <QDebug>
#include <QUrl>

#include <stack>
#include <map>

// http://graphml.graphdrawing.org/primer/graphml-primer.html

GraphMLParser::GraphMLParser(UserNodeData* userNodeData, UserEdgeData* userEdgeData) :
    _userNodeData(userNodeData), _userEdgeData(userEdgeData)
{
    // Add this up front, so that it appears first in the attribute table
    userNodeData->add(QObject::tr("Node Name"));
}

bool GraphMLParser::parse(const QUrl& url, IGraphModel* graphModel)
{
    Q_ASSERT(graphModel != nullptr);
    if(graphModel == nullptr)
        return false;

    QFile file(url.toLocalFile());
    if(!file.open(QFile::ReadOnly))
    {
        setFailureReason(QStringLiteral("Unable to Open File: %1").arg(url.toLocalFile()));
        return false;
    }

    auto fileSize = file.size();
    if(fileSize == 0)
    {
        setFailureReason(QObject::tr("File is empty."));
        return false;
    }

    setProgress(-1);

    QXmlStreamReader xsr(&file);
    std::stack<QString> stack;
    std::map<QString, NodeId> nodes;
    std::map<QString, QString> nodeAttributes;
    std::map<QString, QString> edgeAttributes;

    bool graphmlElementFound = false;
    int graphNestLevel = 0;
    NodeId activeNodeId;
    EdgeId activeEdgeId;
    QString activeKey;

    auto processToken = [&](QXmlStreamReader::TokenType tokenType)
    {
        if(!activeNodeId.isNull() && !activeEdgeId.isNull())
        {
            setFailureReason(QStringLiteral("Node and edge both active: %1 %2")
                .arg(static_cast<int>(activeNodeId), static_cast<int>(activeEdgeId)));
            return false;
        }

        switch(tokenType)
        {
        case QXmlStreamReader::StartElement:
        {
            const auto& elementName = xsr.name();
            stack.push(elementName.toString());
            const auto& attributes = xsr.attributes();

            if(elementName == QStringLiteral("graphml"))
                graphmlElementFound = true;
            else if(elementName == QStringLiteral("graph"))
            {
                graphNestLevel++;

                if(graphNestLevel > 1)
                {
                    //FIXME: we just skip everything when nested
                    qDebug() << "WARNING: nested graphs not supported";
                }
            }
            else if(elementName == QStringLiteral("key"))
            {
                if(!attributes.hasAttribute("attr.name"))
                    break;

                if(!attributes.hasAttribute("for"))
                    break;

                if(!attributes.hasAttribute("id"))
                    break;

                const auto& attributeName = attributes.value("attr.name");
                const auto& keyId = attributes.value("id");

                if(attributes.value("for") == QStringLiteral("node"))
                    nodeAttributes.emplace(keyId.toString(), attributeName.toString());
                else if(attributes.value("for") == QStringLiteral("edge"))
                    edgeAttributes.emplace(keyId.toString(), attributeName.toString());
            }

            if(graphNestLevel != 1)
                break;

            if(elementName == QStringLiteral("node"))
            {
                if(!attributes.hasAttribute("id"))
                {
                    setFailureReason(QStringLiteral("Node has no id"));
                    return false;
                }

                const auto& nodeName = attributes.value("id").toString();

                if(u::contains(nodes, nodeName))
                {
                    setFailureReason(QStringLiteral("Duplicate node id: %1").arg(nodeName));
                    return false;
                }

                activeNodeId = graphModel->mutableGraph().addNode();
                nodes.emplace(nodeName, activeNodeId);

                _userNodeData->setValueBy(activeNodeId, QObject::tr("Node Name"), nodeName);
                graphModel->setNodeName(activeNodeId, nodeName);
            }
            else if(elementName == QStringLiteral("edge"))
            {
                if(!attributes.hasAttribute("source"))
                {
                    setFailureReason(QStringLiteral("Edge missing source"));
                    return false;
                }

                if(!attributes.hasAttribute("target"))
                {
                    setFailureReason(QStringLiteral("Edge missing target"));
                    return false;
                }

                const auto& sourceName = attributes.value("source").toString();
                const auto& targetName = attributes.value("target").toString();

                if(!u::contains(nodes, sourceName) || !u::contains(nodes, targetName))
                {
                    qDebug() << "WARNING: Edge has unknown source or target:" << sourceName << targetName;
                    break;
                }

                auto sourceId = nodes.at(sourceName);
                auto targetId = nodes.at(targetName);

                activeEdgeId = graphModel->mutableGraph().addEdge(sourceId, targetId);

                if(attributes.hasAttribute("id"))
                {
                    auto edgeName = attributes.value("id").toString();
                    _userEdgeData->setValueBy(activeEdgeId, QObject::tr("Edge Name"), edgeName);
                }
            }
            else if(elementName == QStringLiteral("data"))
            {
                if(!attributes.hasAttribute("key"))
                    break;

                activeKey = attributes.value("key").toString();
            }

            break;
        }

        case QXmlStreamReader::Characters:
        {
            if(graphNestLevel != 1)
                break;

            if(activeKey.isEmpty() || _userNodeData == nullptr)
                break;

            const auto& data = xsr.text().toString();

            if(!activeNodeId.isNull() && u::contains(nodeAttributes, activeKey))
                _userNodeData->setValueBy(activeNodeId, nodeAttributes.at(activeKey), data);
            else if(!activeEdgeId.isNull() && u::contains(edgeAttributes, activeKey))
                _userEdgeData->setValueBy(activeEdgeId, edgeAttributes.at(activeKey), data);

            break;
        }

        case QXmlStreamReader::EndElement:
        {
            const auto& elementName = xsr.name();

            if(stack.empty())
            {
                setFailureReason(QStringLiteral("Orphan end element: %1").arg(elementName));
                return false;
            }

            const auto& top = stack.top();
            if(top != xsr.name())
            {
                setFailureReason(QStringLiteral("Start and end element mismatch: %1 != %2")
                    .arg(top, elementName));
                return false;
            }

            stack.pop();

            if(elementName == QStringLiteral("graph"))
                graphNestLevel--;

            if(graphNestLevel != 1)
                break;

            if(elementName == QStringLiteral("node") && !activeNodeId.isNull())
                activeNodeId.setToNull();
            else if(elementName == QStringLiteral("edge") && !activeEdgeId.isNull())
                activeEdgeId.setToNull();
            else if(elementName == QStringLiteral("data") && !activeKey.isEmpty())
                activeKey.clear();

            break;
        }

        default:
            break;
        }

        return true;
    };

    bool success = true;
    while(!xsr.atEnd() && success)
    {
        auto percent = static_cast<int>((file.pos() * 100) / fileSize);
        setProgress(percent);
        success = processToken(xsr.readNext());

        if(cancelled())
            return false;
    }

    setProgress(-1);

    if(!success)
        return false;

    if(!graphmlElementFound)
    {
        setFailureReason(QStringLiteral("graphml header not found"));
        return false;
    }

    if(xsr.hasError())
    {
        setFailureReason(QStringLiteral("XML parse error: %1").arg(xsr.errorString()));
        return false;
    }

    return true;
}
