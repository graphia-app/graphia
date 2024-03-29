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

using namespace Qt::Literals::StringLiterals;

// http://graphml.graphdrawing.org/primer/graphml-primer.html

GraphMLParser::GraphMLParser(IUserNodeData* userNodeData, IUserEdgeData* userEdgeData) :
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
        setFailureReason(QObject::tr("Unable to Open File."));
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
            setFailureReason(QObject::tr("Node and edge both active: %1 %2")
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

            if(elementName == u"graphml"_s)
                graphmlElementFound = true;
            else if(elementName == u"graph"_s)
            {
                graphNestLevel++;

                if(graphNestLevel > 1)
                {
                    //FIXME: we just skip everything when nested
                    qDebug() << "WARNING: nested graphs not supported";
                }
            }
            else if(elementName == u"key"_s)
            {
                if(!attributes.hasAttribute(u"attr.name"_s))
                    break;

                if(!attributes.hasAttribute(u"for"_s))
                    break;

                if(!attributes.hasAttribute(u"id"_s))
                    break;

                const auto& attributeName = attributes.value(u"attr.name"_s);
                const auto& keyId = attributes.value(u"id"_s);

                if(attributes.value(u"for"_s) == u"node"_s)
                    nodeAttributes.emplace(keyId.toString(), attributeName.toString());
                else if(attributes.value(u"for"_s) == u"edge"_s)
                    edgeAttributes.emplace(keyId.toString(), attributeName.toString());
            }

            if(graphNestLevel != 1)
                break;

            if(elementName == u"node"_s)
            {
                if(!attributes.hasAttribute(u"id"_s))
                {
                    setFailureReason(QObject::tr("Node has no id."));
                    return false;
                }

                const auto& nodeName = attributes.value(u"id"_s).toString();

                if(u::contains(nodes, nodeName))
                {
                    setFailureReason(QObject::tr("Duplicate node id: %1").arg(nodeName));
                    return false;
                }

                activeNodeId = graphModel->mutableGraph().addNode();
                nodes.emplace(nodeName, activeNodeId);

                _userNodeData->setValueBy(activeNodeId, QObject::tr("Node Name"), nodeName);
                graphModel->setNodeName(activeNodeId, nodeName);
            }
            else if(elementName == u"edge"_s)
            {
                if(!attributes.hasAttribute(u"source"_s))
                {
                    setFailureReason(QObject::tr("Edge missing source."));
                    return false;
                }

                if(!attributes.hasAttribute(u"target"_s))
                {
                    setFailureReason(QObject::tr("Edge missing target."));
                    return false;
                }

                const auto& sourceName = attributes.value(u"source"_s).toString();
                const auto& targetName = attributes.value(u"target"_s).toString();

                if(!u::contains(nodes, sourceName) || !u::contains(nodes, targetName))
                {
                    qDebug() << "WARNING: Edge has unknown source or target:" << sourceName << targetName;
                    break;
                }

                auto sourceId = nodes.at(sourceName);
                auto targetId = nodes.at(targetName);

                activeEdgeId = graphModel->mutableGraph().addEdge(sourceId, targetId);

                if(attributes.hasAttribute(u"id"_s))
                {
                    auto edgeName = attributes.value(u"id"_s).toString();
                    _userEdgeData->setValueBy(activeEdgeId, QObject::tr("Edge Name"), edgeName);
                }
            }
            else if(elementName == u"data"_s)
            {
                if(!attributes.hasAttribute(u"key"_s))
                    break;

                activeKey = attributes.value(u"key"_s).toString();
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
                setFailureReason(QObject::tr("Orphan end element: %1").arg(elementName));
                return false;
            }

            const auto& top = stack.top();
            if(top != xsr.name())
            {
                setFailureReason(QObject::tr("Start and end element mismatch: %1 != %2")
                    .arg(top, elementName));
                return false;
            }

            stack.pop();

            if(elementName == u"graph"_s)
                graphNestLevel--;

            if(graphNestLevel != 1)
                break;

            if(elementName == u"node"_s && !activeNodeId.isNull())
                activeNodeId.setToNull();
            else if(elementName == u"edge"_s && !activeEdgeId.isNull())
                activeEdgeId.setToNull();
            else if(elementName == u"data"_s && !activeKey.isEmpty())
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
        setFailureReason(QObject::tr("graphml header not found."));
        return false;
    }

    if(xsr.hasError())
    {
        setFailureReason(QObject::tr("XML parse error: %1").arg(xsr.errorString()));
        return false;
    }

    return true;
}
