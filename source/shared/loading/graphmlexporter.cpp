#include "graphmlexporter.h"

#include <QString>
#include <QFile>
#include <QUrl>
#include <QXmlStreamWriter>
#include <shared/graph/igraphmodel.h>
#include <shared/graph/igraph.h>
#include <shared/attributes/iattribute.h>

GraphMLExporter::GraphMLExporter()
{

}

void GraphMLExporter::uncancel()
{
}

void GraphMLExporter::cancel()
{
}

bool GraphMLExporter::cancelled() const
{
    return false;
}

void GraphMLExporter::setProgress(int percent)
{
}

bool GraphMLExporter::save(const QUrl &url, IGraphModel *graphModel) const
{
    QFile file(url.toLocalFile());
    file.open(QIODevice::WriteOnly);
    QXmlStreamWriter stream(&file);
    stream.setAutoFormatting(true);

    stream.writeStartElement("graphml");
    stream.writeStartElement("graph");
    stream.writeAttribute("edgedefault", "directed");

    int keyId = 0;
    std::map<QString, QString> idToAttribute;
    std::map<QString, QString> attributeToId;
    for(const auto& attributeName : graphModel->attributeNames())
    {
        auto attribute = graphModel->attributeByName(attributeName);
        stream.writeStartElement("key");
        stream.writeAttribute("id", QStringLiteral("d%1").arg(keyId));
        idToAttribute[QStringLiteral("d%1").arg(keyId)] = attributeName;
        attributeToId[attributeName] = QStringLiteral("d%1").arg(keyId);

        if(attribute->elementType() == ElementType::Node)
            stream.writeAttribute("for", "node");
        if(attribute->elementType() == ElementType::Edge)
            stream.writeAttribute("for", "edge");

        stream.writeAttribute("attr.name", attributeName);

        QString valueTypeToString = "";
        switch(attribute->valueType())
        {
        case ValueType::Int:
            valueTypeToString = "int";
            break;
        case ValueType::Float:
            valueTypeToString = "float";
            break;
        case ValueType::String:
            valueTypeToString = "string";
            break;
        case ValueType::Numerical:
            valueTypeToString = "float";
            break;
        default:
            valueTypeToString = "string";
            break;
        }
        stream.writeAttribute("attr.type", valueTypeToString);
        stream.writeEndElement();
        keyId++;
    }

    for(auto nodeId : graphModel->graph().nodeIds())
    {
        stream.writeStartElement("node");
        stream.writeAttribute("id", QStringLiteral("n%1").arg(static_cast<int>(nodeId)));
        for(const auto& nodeAttributeName : graphModel->attributeNames(ElementType::Node))
        {
            const auto& attribute = graphModel->attributeByName(nodeAttributeName);
            stream.writeStartElement("data");
            stream.writeAttribute("key", attributeToId.at(nodeAttributeName));
            stream.writeCharacters(attribute->stringValueOf(nodeId));
            stream.writeEndElement();
        }
        stream.writeEndElement();
    }
    int edgeCount = 0;
    for(auto edgeId : graphModel->graph().edgeIds())
    {
        auto& edge = graphModel->graph().edgeById(edgeId);
        stream.writeStartElement("edge");
        stream.writeAttribute("id", QStringLiteral("e%1").arg(edgeCount++));
        stream.writeAttribute("source", QStringLiteral("n%1").arg(static_cast<int>(edge.sourceId())));
        stream.writeAttribute("target", QStringLiteral("n%1").arg(static_cast<int>(edge.targetId())));
        for(const auto& edgeAttributeName : graphModel->attributeNames(ElementType::Edge))
        {
            const auto& attribute = graphModel->attributeByName(edgeAttributeName);
            stream.writeStartElement("data");
            stream.writeAttribute("key", attributeToId.at(edgeAttributeName));
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

QString GraphMLExporter::name() const
{
    return "GraphML";
}

QString GraphMLExporter::extension() const
{
    return ".graphml";
}
