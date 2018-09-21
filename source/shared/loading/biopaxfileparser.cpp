#include "biopaxfileparser.h"


BiopaxHandler::BiopaxHandler(BiopaxFileParser& parser, IGraphModel& graphModel,
                               UserNodeData* userNodeData, int lineCount)
                               : _parser(&parser), _graphModel(&graphModel),
                                 _lineCount(lineCount), _userNodeData(userNodeData)
{}

bool BiopaxHandler::startDocument()
{
    return true;
}

bool BiopaxHandler::endDocument()
{
    qDebug() << "End Document";
    for(const auto& tempEdge : _temporaryEdges)
    {
        for(auto sourceNodeString : tempEdge._sources)
        {
            qDebug() << "Source String" << sourceNodeString;
            auto sourceNodeId = _nodeMap.find(sourceNodeString);
            if(sourceNodeId == _nodeMap.end())
            {
                _errorString = QStringLiteral("Invalid Edge Source. Edge - Source:").arg(sourceNodeString);
                return false;
            }

            for(auto targetNodeString : tempEdge._targets)
            {
                qDebug() << "Target String" << sourceNodeString;

                auto targetNodeId = _nodeMap.find(targetNodeString);
                if(targetNodeId == _nodeMap.end())
                {
                    _errorString = QStringLiteral("Invalid Edge Target. Edge - Source: %1 Target: %2").arg(sourceNodeString, targetNodeString);
                    return false;
                }

                const EdgeId& edgeId = _graphModel->mutableGraph().addEdge(sourceNodeId->second, targetNodeId->second);
                _edgeIdMap[tempEdge] = edgeId;
            }
        }
    }
    return true;
}

bool BiopaxHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts)
{
    if(_edgeElementNames.contains(localName, Qt::CaseInsensitive))
    {
        qDebug() << "Edge Found" << localName << atts.value(QStringLiteral("rdf:ID"));

        _temporaryEdges.push_back({});
        _activeTemporaryEdges.push(&_temporaryEdges.back());

        if(!_activeNodes.empty())
        {
            auto targetString = atts.value(QStringLiteral("rdf:resource")).remove("#");
            qDebug() << "Edge Found" << _nodeIdToNameMap[_activeNodes.top()] << atts.value(QStringLiteral("rdf:resource")).remove("#");

            _temporaryEdges.back()._sources.push_back(_nodeIdToNameMap[_activeNodes.top()]);
            _temporaryEdges.back()._targets.push_back(targetString);
            if(localName == "right")
            {
                _temporaryEdges.back()._sources.push_back(targetString);
                _temporaryEdges.back()._targets.push_back(_nodeIdToNameMap[_activeNodes.top()]);
            }
        }
    }

    if(_nodeElementNames.contains(localName, Qt::CaseInsensitive))
    {
        auto nodeId = _graphModel->mutableGraph().addNode();
        _nodeMap[atts.value(QStringLiteral("rdf:ID"))] = nodeId;
        _nodeIdToNameMap[nodeId] = atts.value(QStringLiteral("rdf:ID"));
        _activeNodes.push(nodeId);

        qDebug() << "Node Found" << localName << atts.value(QStringLiteral("rdf:ID"));

        if(_userNodeData != nullptr)
        {
            auto id = atts.value(QStringLiteral("rdf:ID"));
            _userNodeData->setValueBy(nodeId, QObject::tr("ID"), id);
        }
    }

    _activeElements.push(localName);
    return true;
}

bool BiopaxHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
    if(_nodeElementNames.contains(localName, Qt::CaseInsensitive))
        _activeNodes.pop();

    if(_edgeElementNames.contains(localName, Qt::CaseInsensitive))
        _activeTemporaryEdges.pop();

    _activeElements.pop();

    return true;
}

bool BiopaxHandler::characters(const QString &ch)
{
    if(!_activeNodes.empty())
    {
        if(_activeElements.top() == QStringLiteral("displayName"))
        {
            _userNodeData->setValueBy(_activeNodes.top(), QObject::tr("Node Name"), ch);
            _graphModel->setNodeName(_activeNodes.top(), ch);
        }
    }

    return true;
}

void BiopaxHandler::setDocumentLocator(QXmlLocator *locator)
{

}

QString BiopaxHandler::errorString() const
{
    return "";
}

bool BiopaxHandler::warning(const QXmlParseException &exception)
{
    return true;
}

bool BiopaxHandler::error(const QXmlParseException &exception)
{
    return true;
}

bool BiopaxHandler::fatalError(const QXmlParseException &exception)
{
    return true;
}

BiopaxFileParser::BiopaxFileParser(UserNodeData* userNodeData) :
    _userNodeData(userNodeData)
{
    userNodeData->add(QObject::tr("Node Name"));
}

bool BiopaxFileParser::parse(const QUrl &url, IGraphModel *graphModel)
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
