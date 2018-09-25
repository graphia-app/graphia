#ifndef BIOPAXFILEPARSER_H
#define BIOPAXFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/plugins/basegenericplugin.h"
#include "shared/graph/imutablegraph.h"
#include "shared/plugins/userelementdata.h"

#include <QtXml/QXmlDefaultHandler>

#include <stack>

class BiopaxFileParser;

class BiopaxHandler : public QXmlDefaultHandler
{
    // http://www.biopax.org/owldoc/Level3/
    // Effectively, Entity and all subclasses are Nodes.
    // The members and properties of entities define the edges

    //CamalCase is Class definitions, mixedCase is properties
    QStringList _nodeElementNames =
    {
        "Entity",
        "Interaction",
        "PhysicalEntity",
        "Conversion",
        "Pathway",
        "DnaRegion",
        "SmallMolecule",
        "Dna",
        "Rna",
        "Complex",
        "Protein",
        "RnaRegion",
        "Gene",
        "Complex",
        "BiochemicalReaction",
        "Control",
        "Catalysis",
        "Degradation",
        "GeneticInteraction",
        "MolecularInteraction",
        "Modulation",
        "TemplateReaction",
        "TemplateReactionRegulation",
        "Transport",
        "TransportWithBiochemicalReaction"
    };
    // Edges are participant members subclasses
    // http://www.biopax.org/owldoc/Level3/objectproperties/participant___-1675119396.html
    QStringList _edgeElementNames =
    {
        "pathwayComponent",
        "memberPhysicalEntity",
        "left",
        "right",
        "controller",
        "controlled",
        "component",
        "product",
        "cofactor",
        "template",
        "participant"
    };
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

        // cppcheck-suppress noExplicitConstructor
        Attribute(const AttributeKey& key) // NOLINT
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
        QStringList _sources;
        QStringList _targets;
        friend bool operator<(const TemporaryEdge& l, const TemporaryEdge& r)
        {
            return std::tie(l._sources, l._targets)
                 < std::tie(r._sources, r._targets);
        }
    };

    BiopaxFileParser* _parser = nullptr;
    IGraphModel* _graphModel = nullptr;

    QString _errorString = QString();

    AttributeData<NodeId> _nodeAttributes;
    AttributeData<EdgeId> _edgeAttributes;
    AttributeData<TemporaryEdge> _tempEdgeAttributes;

    // Key = AttributeKeyID
    std::map<QString, TemporaryEdge> _temporaryEdgeMap;
    std::vector<TemporaryEdge> _temporaryEdges;
    std::map<QString, NodeId> _nodeMap;
    std::map<NodeId, QString> _nodeIdToNameMap;
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
    BiopaxHandler(BiopaxFileParser& parser, IGraphModel& graphModel, UserNodeData* userNodeData, int lineCount);
    bool startDocument() override;
    bool endDocument() override;
    bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts) override;
    bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName) override;
    bool characters(const QString &ch) override;
    void setDocumentLocator(QXmlLocator *locator) override;

    QString errorString() const override;
    bool warning(const QXmlParseException &exception) override;
    bool error(const QXmlParseException &exception) override;
    bool fatalError(const QXmlParseException &exception) override;
};

class BiopaxFileParser: public IParser
{

private:
    UserNodeData* _userNodeData;
    BiopaxHandler::AttributeData<NodeId> _nodeAttributeData;
    BiopaxHandler::AttributeData<EdgeId> _edgeAttributeData;

public:
    BiopaxFileParser(UserNodeData* userNodeData);

    bool parse(const QUrl &url, IGraphModel *graphModel) override;
    static bool canLoad(const QUrl &) { return true; }
};

#endif // BIOPAXFILEPARSER_H
