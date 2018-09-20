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
//    qDebug() << namespaceURI;
//    qDebug() << localName;
//    for(int i = 0; i < atts.count(); ++i)
//    {
//        qDebug() << atts.localName(i) << atts.value(i) << atts.uri(i);
//    }

    if(!_activeTemporaryEdges.empty())
    {
        qDebug() << "Active Edges not empty" << localName;

        if(localName == QStringLiteral("left"))
        {
            _activeTemporaryEdges.top()->_sources.push_back(atts.value("rdf:resource").remove("#"));
            qDebug() << "LEFT" << localName;
        }
        if(localName == QStringLiteral("right"))
        {
            _activeTemporaryEdges.top()->_targets.push_back(atts.value("rdf:resource").remove("#"));
            qDebug() << "RIGHT" << localName;
        }
    }

    if(nodeElementNames.contains(localName))
    {
        auto nodeId = _graphModel->mutableGraph().addNode();
        _nodeMap[atts.value(QStringLiteral("rdf:ID"))] = nodeId;
        _activeNodes.push(nodeId);

        qDebug() << "Node Found" << localName << atts.value(QStringLiteral("rdf:ID"));

        if(_userNodeData != nullptr)
        {
            auto nodeName = atts.value(QStringLiteral("rdf:ID"));
            _userNodeData->setValueBy(nodeId, QObject::tr("Node Name"), nodeName);
            _graphModel->setNodeName(nodeId, nodeName);
        }
    }
    if(edgeElementNames.contains(localName))
    {
        qDebug() << "Edge Found" << localName << atts.value(QStringLiteral("rdf:ID"));

        _temporaryEdges.push_back({});
        _activeTemporaryEdges.push(&_temporaryEdges.back());
    }
    return true;
}

bool BiopaxHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
    if(nodeElementNames.contains(localName))
        _activeNodes.pop();

    if(edgeElementNames.contains(localName))
        _activeTemporaryEdges.pop();

    return true;
}

bool BiopaxHandler::characters(const QString &ch)
{
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
