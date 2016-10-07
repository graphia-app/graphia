#include "graphmlparser.h"

#include <QtXml/QXmlInputSource>
#include <QDebug>
#include <QUrl>
#include <shared/utils/utils.h>

GraphMLHandler::GraphMLHandler(IMutableGraph &mutableGraph, const IParser::ProgressFn &progress,
                               NodeAttributes* nodeAttributes, int lineCount)
                               : _graph(mutableGraph), _progress(progress),
                                 _lineCount(lineCount), _otherNodeAttributes(nodeAttributes)
{
    if(_otherNodeAttributes != nullptr)
        _otherNodeAttributes->add(QObject::tr("Node Name"));
}

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
        _errorString = "Not all GraphML Elements are terminated. Stack not empty";
        return false;
    }

    // Finally populate graph with Edges from tempEdges
    for(auto tempEdge : _temporaryEdges)
    {
        const EdgeId& edgeId = _graph.addEdge(_nodeMap.at(tempEdge._source), _nodeMap.at(tempEdge._target));
        _edgeIdMap[tempEdge] = edgeId;
    }

    // Populate EdgeAttributes with new EdgeIds and attributes
    for(auto edgeAttr : _tempEdgeAttributes)
    {
        for(auto idAttributePair : edgeAttr.second)
        {
            auto& tempEdge = idAttributePair.first;
            auto& attr = idAttributePair.second;

            _edgeAttributes[edgeAttr.first][_edgeIdMap.at(tempEdge)] = attr;
        }
    }

    return true;
}

bool GraphMLHandler::startElement(const QString &, const QString &localName, const QString &, const QXmlAttributes &atts)
{
    // New Edge
    if(localName == "edge")
    {
        // Do not parse if nested nodes/edges
        if(!_activeNodes.empty())
        {
            QString attributes = "";
            for(int i = 0; i < atts.count(); ++i)
                attributes += atts.localName(i) + " " + atts.value(i) + ", ";

            _errorString = "Nested Nodes + Edges " + attributes
                    + " Line: " + QString::number(_locator->lineNumber());
            return false;
        }

        _temporaryEdges.push_back({atts.value("source"), atts.value("target")});
        _activeTemporaryEdges.push(&_temporaryEdges.back());
        // Check if id has already been used
        if(u::contains(_temporaryEdgeMap, atts.value("id")))
        {
            _errorString = "Edge ID already in use. Edge ID: " + atts.value("id")
                    + " Line: " + QString::number(_locator->lineNumber());
            return false;
        }
        if(atts.value("id").length() > 0)
            _temporaryEdgeMap[atts.value("id")] = _temporaryEdges.back();


    }
    // New Node
    else if(localName == "node")
    {
        // Do not parse if nested nodes/edges
        if(!_activeTemporaryEdges.empty())
        {
            QString attributes = "";
            for(int i = 0; i < atts.count(); ++i)
                attributes += atts.localName(i) + " " + atts.value(i) + ", ";

            _errorString = "Nested Nodes + Edges " + attributes
                    + " Line: " + QString::number(_locator->lineNumber());
            return false;
        }

        auto nodeId = _graph.addNode();
        _activeNodes.push(nodeId);
        // Check if id has already been used
        if(u::contains(_nodeMap, atts.value("id")))
        {
            _errorString = "Node ID already in use. Node ID: " + atts.value("id")
                    + " Line: " + QString::number(_locator->lineNumber());
            return false;
        }
        _nodeMap[atts.value("id")] = _activeNodes.top();

        if(_otherNodeAttributes != nullptr)
        {
            _otherNodeAttributes->addNodeId(nodeId);
            _otherNodeAttributes->setValueByNodeId(nodeId, QObject::tr("Node Name"),
                                              atts.value("id"));
        }
    }
    // New Attribute Type
    else if(localName == "key")
    {
        AttributeKey attributeKey;
        attributeKey._name = atts.value("attr.name");
        attributeKey._type = atts.value("attr.type");

        auto key = std::make_pair(atts.value("id"), atts.value("for"));
        // Check if id has already been used
        if(u::contains(_attributeKeyMap, key))
        {
            _errorString = "Attribute Key ID already in use. Attribute Key ID: " + atts.value("id")
                    + " Line: " + QString::number(_locator->lineNumber());
            return false;
        }
        _attributeKeyMap[key] = attributeKey;

        _activeAttributeKeys.push(&_attributeKeyMap[key]);
    }
    // Data tags are likely attribute data
    else if(localName == "data")
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

            auto pairKey = std::make_pair(atts.value("key"), "node");
            if(u::contains(_attributeKeyMap, pairKey))
            {
                auto attributeKey = _attributeKeyMap.at(pairKey);
                _nodeAttributes[attributeKey].emplace(_activeNodes.top(), attributeKey);
                // Set Attribute as active for access later in characters
                _activeAttributes.push(&_nodeAttributes[attributeKey][_activeNodes.top()]);
            }
            auto noForPairKey = std::make_pair(atts.value("key"), "");
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

            auto pairKey = std::make_pair(atts.value("key"), "edge");
            if(u::contains(_attributeKeyMap, pairKey))
            {
                auto attributeKey = _attributeKeyMap.at(pairKey);
                _tempEdgeAttributes[attributeKey].emplace(*_activeTemporaryEdges.top(), attributeKey);
                // Set Attribute as active for access later in characters
                _activeAttributes.push(&_tempEdgeAttributes[attributeKey][*_activeTemporaryEdges.top()]);
            }
            auto noForPairKey = std::make_pair(atts.value("key"), "");
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
    _progress(_locator->lineNumber() * 100 / _lineCount );

    if(localName == "node")
        _activeNodes.pop();
    if(localName == "edge")
        _activeTemporaryEdges.pop();
    else if(localName == "key")
        _activeAttributeKeys.pop();
    else if(localName == "data" && (!_activeNodes.empty() || !_activeTemporaryEdges.empty()))
    {
        auto attribute = _activeAttributes.top();

        if(_otherNodeAttributes != nullptr && !_activeNodes.empty() && !attribute->_name.isEmpty())
        {
            _otherNodeAttributes->add(attribute->_name);
            _otherNodeAttributes->setValueByNodeId(_activeNodes.top(), attribute->_name, attribute->_value.toString());
        }

        _activeAttributes.pop();
    }

    _activeElements.pop();
    return true;
}

bool GraphMLHandler::characters(const QString &ch)
{
    // Default tag for Attribute Keys
    if(_activeElements.top() == "default" && !_activeAttributeKeys.empty())
    {
        _activeAttributeKeys.top()->_default = ch.trimmed();
    }
    // Data value for Attribute
    else if(_activeElements.top() == "data" && !_activeAttributes.empty())
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

GraphMLParser::GraphMLParser(NodeAttributes* nodeAttributes) :
    _nodeAttributes(nodeAttributes)
{}

bool GraphMLParser::parse(const QUrl &url, IMutableGraph &graph, const IParser::ProgressFn &progress)
{
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

    GraphMLHandler handler(graph, progress, _nodeAttributes, lineCount);
    QXmlInputSource *source = new QXmlInputSource(&file);
    QXmlSimpleReader xmlReader;
    xmlReader.setContentHandler(&handler);
    xmlReader.setErrorHandler(&handler);
    xmlReader.parse(source);

    if(handler.errorString() != "")
    {
        qDebug() << handler.errorString();
        return false;
    }

    return true;
}
