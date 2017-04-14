#ifndef GRAPHMLPARSER_H
#define GRAPHMLPARSER_H

#include "shared/loading/baseparser.h"
#include "shared/plugins/basegenericplugin.h"
#include "shared/graph/imutablegraph.h"
#include <QtXml/QXmlDefaultHandler>
#include <stack>

class UserNodeData;

class GraphMLHandler : public QXmlDefaultHandler
{
public:
    struct AttributeKey
    {
        QString _name;
        QVariant _default;
        QString _type;
        friend bool operator<(const AttributeKey& l, const AttributeKey& r)
        {
            return std::tie(l._name, l._default, l._type)
                 < std::tie(r._name, r._default, r._type);
        }
    };

    struct Attribute
    {
        QString _name;
        QVariant _default;
        QString _type;
        QVariant _value;

        //cppcheck-suppress noExplicitConstructor
        Attribute(const AttributeKey& key)
        {
            _name = key._name;
            _default = key._default;
            _type = key._type;
            _value = key._default;
        }

        Attribute() = default;
    };

    template<typename T>
    using AttributeData = std::map<AttributeKey, std::map<T, Attribute>>;

private:
    struct TemporaryEdge
    {
        QString _source;
        QString _target;
        friend bool operator<(const TemporaryEdge& l, const TemporaryEdge& r)
        {
            return std::tie(l._source, l._target)
                 < std::tie(r._source, r._target);
        }
    };

    IMutableGraph* _graph;

    const IParser::ProgressFn* _progress;
    QString _errorString = QString();

    AttributeData<NodeId> _nodeAttributes;
    AttributeData<EdgeId> _edgeAttributes;
    AttributeData<TemporaryEdge> _tempEdgeAttributes;

    // Key = AttributeKeyID
    std::map<QString, TemporaryEdge> _temporaryEdgeMap;
    std::vector<TemporaryEdge> _temporaryEdges;
    std::map<QString, NodeId> _nodeMap;
    std::map<TemporaryEdge, EdgeId> _edgeIdMap;
    // Key = pair(Attribute Key ID, 'For' element name)
    std::map<std::pair<QString, QString>, AttributeKey> _attributeKeyMap;

    std::stack<NodeId> _activeNodes;
    std::stack<TemporaryEdge*> _activeTemporaryEdges;
    std::stack<QString> _activeElements;
    std::stack<AttributeKey*> _activeAttributeKeys;
    std::stack<Attribute*> _activeAttributes;

    QXmlLocator* _locator = nullptr;
    int _lineCount = 0;

    UserNodeData* _userNodeData;

public:
    GraphMLHandler(IMutableGraph& mutableGraph, const IParser::ProgressFn& progress, UserNodeData* userNodeData, int lineCount);
    bool startDocument();
    bool endDocument();
    bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts);
    bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName);
    bool characters(const QString &ch);
    void setDocumentLocator(QXmlLocator *locator);

    QString errorString() const;
    bool warning(const QXmlParseException &exception);
    bool error(const QXmlParseException &exception);
    bool fatalError(const QXmlParseException &exception);
};

class GraphMLParser: public BaseParser
{
private:
    UserNodeData* _userNodeData;
    GraphMLHandler::AttributeData<NodeId> _nodeAttributeData;
    GraphMLHandler::AttributeData<EdgeId> _edgeAttributeData;

public:
    explicit GraphMLParser(UserNodeData *userNodeData = nullptr);
    bool parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progress);
};

#endif // GRAPHMLPARSER_H
