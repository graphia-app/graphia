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
#include "shared/graph/imutablegraph.h"
#include "shared/utils/container.h"

#include <QtXml/QXmlInputSource>
#include <QDebug>
#include <QUrl>

GraphMLHandler::GraphMLHandler(GraphMLParser& parser, IGraphModel& graphModel,
                               UserNodeData* userNodeData, int lineCount) : _parser(&parser), _graphModel(&graphModel),
                                 _lineCount(lineCount), _userNodeData(userNodeData)
{}

bool GraphMLHandler::startDocument()
{
    return true;
}

bool GraphMLHandler::endDocument()
{
    // Stacks should be empty
    if(!_activeAttributes.empty() || !_activeAttributeKeys.empty() ||
       !_activeNodes.empty() || !_activeTemporaryEdges.empty() ||
       !_activeElements.empty())
    {
        _errorString = QStringLiteral("Not all GraphML Elements are terminated. Stack not empty");
        return false;
    }

    // Finally populate graph with Edges from tempEdges
    for(const auto& tempEdge : _temporaryEdges)
    {
        auto sourceNodeId = _nodeMap.find(tempEdge._source);
        auto targetNodeId = _nodeMap.find(tempEdge._target);
        if(sourceNodeId == _nodeMap.end())
        {
            _errorString = QStringLiteral("Invalid Edge Source. Edge - Source: %1 Target: %2").arg(tempEdge._source, tempEdge._target);
            return false;
        }
        if(targetNodeId == _nodeMap.end())
        {
            _errorString = QStringLiteral("Invalid Edge Target. Edge - Source: %1 Target: %2").arg(tempEdge._source, tempEdge._target);
            return false;
        }
        const EdgeId& edgeId = _graphModel->mutableGraph().addEdge(sourceNodeId->second, targetNodeId->second);
        _edgeIdMap[tempEdge] = edgeId;
    }

    // Populate EdgeAttributes with new EdgeIds and attributes
    for(const auto& edgeAttr : _tempEdgeAttributes)
    {
        for(const auto& idAttributePair : edgeAttr.second)
        {
            const auto& tempEdge = idAttributePair.first;
            const auto& attr = idAttributePair.second;

            _edgeAttributes[edgeAttr.first][_edgeIdMap.at(tempEdge)] = attr;
        }
    }

    return true;
}

bool GraphMLHandler::startElement(const QString&, const QString& localName, const QString&, const QXmlAttributes& atts)
{
    // New Edge
    if(localName == QLatin1String("edge"))
    {
        // Do not parse if nested nodes/edges
        if(!_activeNodes.empty())
        {
            QString attributes = QLatin1String("");
            for(int i = 0; i < atts.count(); ++i)
                attributes += atts.localName(i) + " " + atts.value(i) + ", ";

            _errorString = "Nested Nodes + Edges " + attributes
                    + " Line: " + QString::number(_locator->lineNumber());
            return false;
        }

        _temporaryEdges.push_back({atts.value("source"), atts.value("target")});
        _activeTemporaryEdges.push(&_temporaryEdges.back());
        // Check if id has already been used
        if(u::contains(_temporaryEdgeMap, atts.value(QStringLiteral("id"))))
        {
            _errorString = "Edge ID already in use. Edge ID: " + atts.value(QStringLiteral("id"))
                    + " Line: " + QString::number(_locator->lineNumber());
            return false;
        }
        if(atts.value(QStringLiteral("id")).length() > 0)
            _temporaryEdgeMap[atts.value(QStringLiteral("id"))] = _temporaryEdges.back();


    }
    // New Node
    else if(localName == QLatin1String("node"))
    {
        // Do not parse if nested nodes/edges
        if(!_activeTemporaryEdges.empty())
        {
            QString attributes = QLatin1String("");
            for(int i = 0; i < atts.count(); ++i)
                attributes += atts.localName(i) + " " + atts.value(i) + ", ";

            _errorString = "Nested Nodes + Edges " + attributes
                    + " Line: " + QString::number(_locator->lineNumber());
            return false;
        }

        auto nodeId = _graphModel->mutableGraph().addNode();
        _activeNodes.push(nodeId);
        // Check if id has already been used
        if(u::contains(_nodeMap, atts.value(QStringLiteral("id"))))
        {
            _errorString = "Node ID already in use. Node ID: " + atts.value(QStringLiteral("id"))
                    + " Line: " + QString::number(_locator->lineNumber());
            return false;
        }
        _nodeMap[atts.value(QStringLiteral("id"))] = _activeNodes.top();

        if(_userNodeData != nullptr)
        {
            auto nodeName = atts.value(QStringLiteral("id"));
            _userNodeData->setValueBy(nodeId, QObject::tr("Node Name"), nodeName);
            _graphModel->setNodeName(nodeId, nodeName);
        }
    }
    // New Attribute Type
    else if(localName == QLatin1String("key"))
    {
        AttributeKey attributeKey;
        attributeKey._name = atts.value(QStringLiteral("attr.name"));
        attributeKey._type = atts.value(QStringLiteral("attr.type"));

        auto key = std::make_pair(atts.value(QStringLiteral("id")), atts.value(QStringLiteral("for")));
        // Check if id has already been used
        if(u::contains(_attributeKeyMap, key))
        {
            _errorString = "Attribute Key ID already in use. Attribute Key ID: " + atts.value(QStringLiteral("id"))
                    + " Line: " + QString::number(_locator->lineNumber());
            return false;
        }
        _attributeKeyMap[key] = attributeKey;

        _activeAttributeKeys.push(&_attributeKeyMap[key]);
    }
    // Data tags are likely attribute data
    else if(localName == QLatin1String("data"))
    {
        if(!_activeNodes.empty())
        {
            if(!_activeTemporaryEdges.empty())
            {
                _errorString = "Nested Nodes + Edges. Edge: "
                        + _activeTemporaryEdges.top()->_source + "," + _activeTemporaryEdges.top()->_target
                        + " Line: " + QString::number(_locator->lineNumber());
                return false;
            }

            auto pairKey = std::make_pair(atts.value(QStringLiteral("key")), "node");
            if(u::contains(_attributeKeyMap, pairKey))
            {
                auto attributeKey = _attributeKeyMap.at(pairKey);
                _nodeAttributes[attributeKey].emplace(_activeNodes.top(), attributeKey);
                // Set Attribute as active for access later in characters
                _activeAttributes.push(&_nodeAttributes[attributeKey][_activeNodes.top()]);
            }
            auto noForPairKey = std::make_pair(atts.value(QStringLiteral("key")), "");
            if(u::contains(_attributeKeyMap, noForPairKey))
            {
                auto attributeKey = _attributeKeyMap.at(noForPairKey);
                _nodeAttributes[attributeKey].emplace(_activeNodes.top(), attributeKey);
                // Set Attribute as active for access later in characters
                _activeAttributes.push(&_nodeAttributes[attributeKey][_activeNodes.top()]);
            }
        }
        else if(!_activeTemporaryEdges.empty())
        {
            if(!_activeNodes.empty())
            {
                _errorString = "Nested Nodes + Edges. Line: " + QString::number(_locator->lineNumber());
                return false;
            }

            auto pairKey = std::make_pair(atts.value(QStringLiteral("key")), "edge");
            if(u::contains(_attributeKeyMap, pairKey))
            {
                auto attributeKey = _attributeKeyMap.at(pairKey);
                _tempEdgeAttributes[attributeKey].emplace(*_activeTemporaryEdges.top(), attributeKey);
                // Set Attribute as active for access later in characters
                _activeAttributes.push(&_tempEdgeAttributes[attributeKey][*_activeTemporaryEdges.top()]);
            }
            auto noForPairKey = std::make_pair(atts.value(QStringLiteral("key")), "");
            if(u::contains(_attributeKeyMap, noForPairKey))
            {
                auto attributeKey = _attributeKeyMap.at(noForPairKey);
                _nodeAttributes[attributeKey].emplace(_activeNodes.top(), attributeKey);
                // Set Attribute as active for access later in characters
                _activeAttributes.push(&_nodeAttributes[attributeKey][_activeNodes.top()]);
            }
        }
    }
    _activeElements.push(localName);
    return true;
}

bool GraphMLHandler::endElement(const QString &, const QString &localName, const QString &)
{
    if(_parser->cancelled())
    {
        _errorString = QObject::tr("User cancelled");
        return false;
    }

    _parser->setProgress(_locator->lineNumber() * 100 / _lineCount );

    if(localName == QLatin1String("node"))
        _activeNodes.pop();
    if(localName == QLatin1String("edge"))
        _activeTemporaryEdges.pop();
    else if(localName == QLatin1String("key"))
        _activeAttributeKeys.pop();
    else if(localName == QLatin1String("data") && (!_activeNodes.empty() || !_activeTemporaryEdges.empty()))
    {
        const auto* attribute = _activeAttributes.top();

        if(_userNodeData != nullptr && !_activeNodes.empty() && !attribute->_name.isEmpty())
            _userNodeData->setValueBy(_activeNodes.top(), attribute->_name, attribute->_value.toString());

        _activeAttributes.pop();
    }

    _activeElements.pop();
    return true;
}

bool GraphMLHandler::characters(const QString &ch)
{
    // Default tag for Attribute Keys
    if(_activeElements.top() == QLatin1String("default") && !_activeAttributeKeys.empty())
    {
        _activeAttributeKeys.top()->_default = ch.trimmed();
    }
    // Data value for Attribute
    else if(_activeElements.top() == QLatin1String("data") && !_activeAttributes.empty())
    {
        _activeAttributes.top()->_value = ch.trimmed();
    }
    return true;
}

void GraphMLHandler::setDocumentLocator(QXmlLocator* locator)
{
    _locator = locator;
}

QString GraphMLHandler::errorString() const
{
    return _errorString;
}

bool GraphMLHandler::warning(const QXmlParseException &)
{
    return true;
}

bool GraphMLHandler::error(const QXmlParseException &)
{
    return true;
}

bool GraphMLHandler::fatalError(const QXmlParseException &)
{
    return true;
}

GraphMLParser::GraphMLParser(UserNodeData* userNodeData) :
    _userNodeData(userNodeData)
{
    // Add this up front, so that it appears first in the attribute table
    userNodeData->add(QObject::tr("Node Name"));
}

bool GraphMLParser::parse(const QUrl &url, IGraphModel* graphModel)
{
    Q_ASSERT(graphModel != nullptr);
    if(graphModel == nullptr)
        return false;

    QFile file(url.toLocalFile());
    int lineCount = 0;
    if(file.open(QFile::ReadOnly))
    {
        while(!file.atEnd())
        {
            file.readLine();
            lineCount++;
        }
        file.seek(0);
    }
    else
    {
        qDebug() << "Unable to Open File" + url.toLocalFile();
        return false;
    }

    setProgress(-1);

    GraphMLHandler handler(*this, *graphModel, _userNodeData, lineCount);
    auto *source = new QXmlInputSource(&file);
    QXmlSimpleReader xmlReader;
    xmlReader.setContentHandler(&handler);
    xmlReader.setErrorHandler(&handler);
    xmlReader.parse(source);

    if(handler.errorString() != QLatin1String(""))
    {
        qDebug() << handler.errorString();
        return false;
    }

    return true;
}
