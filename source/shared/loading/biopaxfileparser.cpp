#include "biopaxfileparser.h"

BiopaxHandler::BiopaxHandler(BiopaxFileParser& parser, IGraphModel& graphModel, UserNodeData* userNodeData,
                             int lineCount) :
    _parser(&parser),
    _graphModel(&graphModel), _lineCount(lineCount), _userNodeData(userNodeData)
{}

bool BiopaxHandler::startDocument()
{
    return true;
}

bool BiopaxHandler::endDocument()
{
    for(const auto& tempEdge : _temporaryEdges)
    {
        for(const auto& sourceNodeString : tempEdge._sources)
        {
            auto sourceNodeId = _nodeMap.find(sourceNodeString);
            if(sourceNodeId == _nodeMap.end())
            {
                _errorString = QStringLiteral("Invalid Edge Source. Edge - Source:").arg(sourceNodeString);
                return false;
            }

            for(const auto& targetNodeString : tempEdge._targets)
            {
                auto targetNodeId = _nodeMap.find(targetNodeString);
                if(targetNodeId == _nodeMap.end())
                {
                    _errorString = QStringLiteral("Invalid Edge Target. Edge - Source: %1 Target: %2")
                                       .arg(sourceNodeString, targetNodeString);
                    return false;
                }

                const EdgeId& edgeId =
                    _graphModel->mutableGraph().addEdge(sourceNodeId->second, targetNodeId->second);
                _edgeIdMap[tempEdge] = edgeId;
            }
        }
    }
    return true;
}

bool BiopaxHandler::startElement(const QString&, const QString& localName, const QString&,
                                 const QXmlAttributes& atts)
{
    if(_edgeElementNames.contains(localName))
    {
        _temporaryEdges.push_back({});
        _activeTemporaryEdges.push(&_temporaryEdges.back());

        if(!_activeNodes.empty())
        {
            auto targetString = atts.value(QStringLiteral("rdf:resource")).remove(QStringLiteral("#"));

            _temporaryEdges.back()._sources.push_back(_nodeIdToNameMap[_activeNodes.top()]);
            _temporaryEdges.back()._targets.push_back(targetString);

            // Some members infer a target edge
            if(localName == QStringLiteral("right") || localName == QStringLiteral("controlled"))
            {
                _temporaryEdges.back()._sources.push_back(targetString);
                _temporaryEdges.back()._targets.push_back(_nodeIdToNameMap[_activeNodes.top()]);
            }
        }
    }

    if(_nodeElementNames.contains(localName) &&
       !(!_activeElements.empty() && _nodeElementNames.contains(_activeElements.top())))
    {
        auto nodeId = _graphModel->mutableGraph().addNode();
        _nodeMap[atts.value(QStringLiteral("rdf:ID"))] = nodeId;
        _nodeIdToNameMap[nodeId] = atts.value(QStringLiteral("rdf:ID"));
        _activeNodes.push(nodeId);

        if(_userNodeData != nullptr)
        {
            auto id = atts.value(QStringLiteral("rdf:ID"));
            _userNodeData->setValueBy(nodeId, QObject::tr("ID"), id);
            _userNodeData->setValueBy(nodeId, QObject::tr("Class"), localName);
        }
    }

    _activeElements.push(localName);
    return true;
}

bool BiopaxHandler::endElement(const QString&, const QString& localName, const QString&)
{
    _parser->setProgress(_locator->lineNumber() * 100 / _lineCount );

    if(_nodeElementNames.contains(localName))
        _activeNodes.pop();

    if(_edgeElementNames.contains(localName))
        _activeTemporaryEdges.pop();

    _activeElements.pop();

    return true;
}

bool BiopaxHandler::characters(const QString& ch)
{
    if(!_activeNodes.empty())
    {
        if(_activeElements.top() == QStringLiteral("displayName"))
        {
            _userNodeData->setValueBy(_activeNodes.top(), QObject::tr("Node Name"), ch);
            _graphModel->setNodeName(_activeNodes.top(), ch);
        }
        if(_activeElements.top() == QStringLiteral("comment"))
        {
            _userNodeData->setValueBy(_activeNodes.top(), QObject::tr("Comment"), ch);
            _graphModel->setNodeName(_activeNodes.top(), ch);
        }
    }

    return true;
}

void BiopaxHandler::setDocumentLocator(QXmlLocator* locator)
{
    _locator = locator;
}

QString BiopaxHandler::errorString() const
{
    return _errorString;
}

bool BiopaxHandler::warning(const QXmlParseException &)
{
    return true;
}

bool BiopaxHandler::error(const QXmlParseException &)
{
    return true;
}

bool BiopaxHandler::fatalError(const QXmlParseException &)
{
    return true;
}

BiopaxFileParser::BiopaxFileParser(UserNodeData* userNodeData) : _userNodeData(userNodeData)
{
    // Add this up front, so that it appears first in the attribute table
    userNodeData->add(QObject::tr("Node Name"));
}

bool BiopaxFileParser::parse(const QUrl& url, IGraphModel* graphModel)
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

    BiopaxHandler handler(*this, *graphModel, _userNodeData, lineCount);
    auto* source = new QXmlInputSource(&file);
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
