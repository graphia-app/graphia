#include "graphmlexporter.h"

#include "graph/graph.h"
#include "graph/graphmodel.h"
#include "layout/nodepositions.h"
#include "shared/attributes/iattribute.h"

#include <QFile>
#include <QString>
#include <QUrl>
#include <QXmlStreamWriter>

void GraphMLExporter::uncancel() {}

void GraphMLExporter::cancel() {}

bool GraphMLExporter::cancelled() const { return false; }

bool GraphMLExporter::save(const QUrl& url, IGraphModel* graphModel)
{
    auto castGraphModel = dynamic_cast<GraphModel*>(graphModel);
    Q_ASSERT(castGraphModel != nullptr);

    QFile file(url.toLocalFile());
    file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);

    size_t fileCount = graphModel->attributeNames().size() +
                       static_cast<size_t>(graphModel->graph().numNodes()) +
                       static_cast<size_t>(graphModel->graph().numEdges());
    size_t runningCount = 0;

    QXmlStreamWriter stream(&file);
    stream.setAutoFormatting(true);

    stream.writeStartElement(QStringLiteral("graphml"));
    stream.writeStartElement(QStringLiteral("graph"));
    stream.writeAttribute(QStringLiteral("edgedefault"), QStringLiteral("directed"));

    // Add position attribute keys
    stream.writeStartElement(QStringLiteral("key"));
    stream.writeAttribute(QStringLiteral("id"), QStringLiteral("x"));
    stream.writeAttribute(QStringLiteral("attr.name"), QStringLiteral("x"));
    stream.writeAttribute(QStringLiteral("attr.type"), QStringLiteral("float"));
    stream.writeAttribute(QStringLiteral("for"), QStringLiteral("node"));
    stream.writeEndElement();

    stream.writeStartElement(QStringLiteral("key"));
    stream.writeAttribute(QStringLiteral("id"), QStringLiteral("y"));
    stream.writeAttribute(QStringLiteral("attr.name"), QStringLiteral("y"));
    stream.writeAttribute(QStringLiteral("attr.type"), QStringLiteral("float"));
    stream.writeAttribute(QStringLiteral("for"), QStringLiteral("node"));
    stream.writeEndElement();

    stream.writeStartElement(QStringLiteral("key"));
    stream.writeAttribute(QStringLiteral("id"), QStringLiteral("z"));
    stream.writeAttribute(QStringLiteral("attr.name"), QStringLiteral("z"));
    stream.writeAttribute(QStringLiteral("attr.type"), QStringLiteral("float"));
    stream.writeAttribute(QStringLiteral("for"), QStringLiteral("node"));
    stream.writeEndElement();

    // Add attribute keys
    int keyId = 0;
    std::map<QString, QString> idToAttribute;
    std::map<QString, QString> attributeToId;
    for(const auto& attributeName : graphModel->attributeNames())
    {
        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / fileCount));

        auto attribute = graphModel->attributeByName(attributeName);
        stream.writeStartElement(QStringLiteral("key"));
        stream.writeAttribute(QStringLiteral("id"), QStringLiteral("d%1").arg(keyId));
        idToAttribute[QStringLiteral("d%1").arg(keyId)] = attributeName;
        attributeToId[attributeName] = QStringLiteral("d%1").arg(keyId);

        if(attribute->elementType() == ElementType::Node)
            stream.writeAttribute(QStringLiteral("for"), QStringLiteral("node"));
        if(attribute->elementType() == ElementType::Edge)
            stream.writeAttribute(QStringLiteral("for"), QStringLiteral("edge"));

        stream.writeAttribute(QStringLiteral("attr.name"), attributeName);

        QString valueTypeToString;
        switch(attribute->valueType())
        {
        case ValueType::Int: valueTypeToString = QStringLiteral("int"); break;
        case ValueType::Float: valueTypeToString = QStringLiteral("float"); break;
        case ValueType::String: valueTypeToString = QStringLiteral("string"); break;
        case ValueType::Numerical: valueTypeToString = QStringLiteral("float"); break;
        default: valueTypeToString = QStringLiteral("string"); break;
        }
        stream.writeAttribute(QStringLiteral("attr.type"), valueTypeToString);
        stream.writeEndElement();
        keyId++;
    }

    for(auto nodeId : graphModel->graph().nodeIds())
    {
        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / fileCount));

        stream.writeStartElement(QStringLiteral("node"));
        stream.writeAttribute(QStringLiteral("id"), QStringLiteral("n%1").arg(static_cast<int>(nodeId)));
        for(const auto& nodeAttributeName : graphModel->attributeNames(ElementType::Node))
        {
            const auto& attribute = graphModel->attributeByName(nodeAttributeName);
            stream.writeStartElement(QStringLiteral("data"));
            stream.writeAttribute(QStringLiteral("key"), attributeToId.at(nodeAttributeName));
            stream.writeCharacters(attribute->stringValueOf(nodeId));
            stream.writeEndElement();
        }

        const auto& pos = castGraphModel->nodePositions().get(nodeId);
        stream.writeStartElement(QStringLiteral("data"));
        stream.writeAttribute(QStringLiteral("key"), QStringLiteral("x"));
        stream.writeCharacters(QString::number(static_cast<double>(pos.x())));
        stream.writeEndElement();
        stream.writeStartElement(QStringLiteral("data"));
        stream.writeAttribute(QStringLiteral("key"), QStringLiteral("y"));
        stream.writeCharacters(QString::number(static_cast<double>(pos.y())));
        stream.writeEndElement();
        stream.writeStartElement(QStringLiteral("data"));
        stream.writeAttribute(QStringLiteral("key"), QStringLiteral("z"));
        stream.writeCharacters(QString::number(static_cast<double>(pos.z())));
        stream.writeEndElement();

        stream.writeEndElement();
    }
    int edgeCount = 0;
    for(auto edgeId : graphModel->graph().edgeIds())
    {
        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / fileCount));

        auto& edge = graphModel->graph().edgeById(edgeId);
        stream.writeStartElement(QStringLiteral("edge"));
        stream.writeAttribute(QStringLiteral("id"), QStringLiteral("e%1").arg(edgeCount++));
        stream.writeAttribute(QStringLiteral("source"), QStringLiteral("n%1").arg(static_cast<int>(edge.sourceId())));
        stream.writeAttribute(QStringLiteral("target"), QStringLiteral("n%1").arg(static_cast<int>(edge.targetId())));
        for(const auto& edgeAttributeName : graphModel->attributeNames(ElementType::Edge))
        {
            const auto& attribute = graphModel->attributeByName(edgeAttributeName);
            stream.writeStartElement(QStringLiteral("data"));
            stream.writeAttribute(QStringLiteral("key"), attributeToId.at(edgeAttributeName));
            stream.writeCharacters(attribute->stringValueOf(edgeId));
            stream.writeEndElement();
        }
        stream.writeEndElement(); // edge
    }
    stream.writeEndElement(); // graph
    stream.writeEndElement(); // graphml

    stream.writeEndDocument();
    return true;
}

QString GraphMLExporter::name() const { return QStringLiteral("GraphML"); }

QString GraphMLExporter::extension() const { return QStringLiteral(".graphml"); }
