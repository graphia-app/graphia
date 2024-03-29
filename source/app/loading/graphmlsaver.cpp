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

#include "graphmlsaver.h"

#include "graph/graph.h"
#include "graph/graphmodel.h"
#include "layout/nodepositions.h"
#include "shared/attributes/iattribute.h"
#include "shared/graph/imutablegraph.h"
#include "ui/document.h"

#include <QFile>
#include <QString>
#include <QUrl>
#include <QXmlStreamWriter>

using namespace Qt::Literals::StringLiterals;

bool GraphMLSaver::save()
{
    auto* graphModel = dynamic_cast<GraphModel*>(_graphModel);

    Q_ASSERT(graphModel != nullptr);
    if(graphModel == nullptr)
        return false;

    QFile file(_url.toLocalFile());
    file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);

    const size_t numElements = _graphModel->attributeNames().size() +
        _graphModel->graph().numNodes() +
        _graphModel->graph().numEdges();
    size_t runningCount = 0;

    QXmlStreamWriter stream(&file);
    stream.setAutoFormatting(true);

    stream.writeStartDocument();

    stream.writeStartElement(u"graphml"_s);
    stream.writeAttribute(u"xmlns"_s, u"http://graphml.graphdrawing.org/xmlns"_s);
    stream.writeAttribute(u"xmlns:xsi"_s, u"http://www.w3.org/2001/XMLSchema-instance"_s);
    stream.writeAttribute(u"xsi:schemaLocation"_s, u"http://graphml.graphdrawing.org/xmlns "_s +
        u"http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd"_s);

    stream.writeStartElement(u"key"_s);
    stream.writeAttribute(u"id"_s, u"desc"_s);
    stream.writeAttribute(u"for"_s, u"node"_s);
    stream.writeAttribute(u"attr.name"_s, u"Description"_s);
    stream.writeAttribute(u"attr.type"_s, u"string"_s);
    stream.writeEndElement();

    // Add position attribute keys
    stream.writeStartElement(u"key"_s);
    stream.writeAttribute(u"id"_s, u"x"_s);
    stream.writeAttribute(u"for"_s, u"node"_s);
    stream.writeAttribute(u"attr.name"_s, u"x"_s);
    stream.writeAttribute(u"attr.type"_s, u"float"_s);
    stream.writeEndElement();

    stream.writeStartElement(u"key"_s);
    stream.writeAttribute(u"id"_s, u"y"_s);
    stream.writeAttribute(u"for"_s, u"node"_s);
    stream.writeAttribute(u"attr.name"_s, u"y"_s);
    stream.writeAttribute(u"attr.type"_s, u"float"_s);
    stream.writeEndElement();

    stream.writeStartElement(u"key"_s);
    stream.writeAttribute(u"id"_s, u"z"_s);
    stream.writeAttribute(u"for"_s, u"node"_s);
    stream.writeAttribute(u"attr.name"_s, u"z"_s);
    stream.writeAttribute(u"attr.type"_s, u"float"_s);
    stream.writeEndElement();

    // Add attribute keys
    setPhase(QObject::tr("Attributes"));
    int keyId = 0;
    std::map<QString, QString> idToAttribute;
    std::map<QString, QString> attributeToId;
    for(const auto& attributeName : _graphModel->attributeNames())
    {
        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / numElements));

        const auto* attribute = _graphModel->attributeByName(attributeName);
        if(attribute->hasParameter())
            continue;

        stream.writeStartElement(u"key"_s);
        stream.writeAttribute(u"id"_s, u"d%1"_s.arg(keyId));
        idToAttribute[u"d%1"_s.arg(keyId)] = attributeName;
        attributeToId[attributeName] = u"d%1"_s.arg(keyId);

        if(attribute->elementType() == ElementType::Node)
            stream.writeAttribute(u"for"_s, u"node"_s);
        if(attribute->elementType() == ElementType::Edge)
            stream.writeAttribute(u"for"_s, u"edge"_s);
        if(attribute->elementType() == ElementType::Component)
            stream.writeAttribute(u"for"_s, u"graph"_s);

        stream.writeAttribute(u"attr.name"_s, attributeName);

        QString valueTypeToString;
        switch(attribute->valueType())
        {
        case ValueType::Int: valueTypeToString = u"int"_s; break;
        case ValueType::Float: valueTypeToString = u"float"_s; break;
        case ValueType::String: valueTypeToString = u"string"_s; break;
        case ValueType::Numerical: valueTypeToString = u"float"_s; break;
        default: valueTypeToString = u"string"_s; break;
        }
        stream.writeAttribute(u"attr.type"_s, valueTypeToString);
        stream.writeEndElement();
        keyId++;
    }

    stream.writeStartElement(u"graph"_s);
    stream.writeAttribute(u"edgedefault"_s, u"directed"_s);

    setPhase(QObject::tr("Nodes"));
    for(auto nodeId : _graphModel->graph().nodeIds())
    {
        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / numElements));

        stream.writeStartElement(u"node"_s);
        stream.writeAttribute(u"id"_s, u"n%1"_s.arg(static_cast<int>(nodeId)));
        for(const auto& nodeAttributeName : _graphModel->attributeNames(ElementType::Node))
        {
            const auto* attribute = _graphModel->attributeByName(nodeAttributeName);
            if(attribute->hasParameter())
                continue;

            stream.writeStartElement(u"data"_s);
            stream.writeAttribute(u"key"_s, attributeToId.at(nodeAttributeName));
            stream.writeCharacters(attribute->stringValueOf(nodeId).toHtmlEscaped());
            stream.writeEndElement();
        }

        stream.writeStartElement(u"data"_s);
        stream.writeAttribute(u"key"_s, u"desc"_s);
        stream.writeCharacters(_graphModel->nodeName(nodeId).toHtmlEscaped());
        stream.writeEndElement();

        const auto& pos = graphModel->nodePositions().get(nodeId);
        stream.writeStartElement(u"data"_s);
        stream.writeAttribute(u"key"_s, u"x"_s);
        stream.writeCharacters(QString::number(static_cast<double>(pos.x())));
        stream.writeEndElement();
        stream.writeStartElement(u"data"_s);
        stream.writeAttribute(u"key"_s, u"y"_s);
        stream.writeCharacters(QString::number(static_cast<double>(pos.y())));
        stream.writeEndElement();
        stream.writeStartElement(u"data"_s);
        stream.writeAttribute(u"key"_s, u"z"_s);
        stream.writeCharacters(QString::number(static_cast<double>(pos.z())));
        stream.writeEndElement();

        stream.writeEndElement();
    }

    setPhase(QObject::tr("Edges"));
    int edgeCount = 0;
    for(auto edgeId : _graphModel->graph().edgeIds())
    {
        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / numElements));

        const auto& edge = _graphModel->graph().edgeById(edgeId);
        stream.writeStartElement(u"edge"_s);
        stream.writeAttribute(u"id"_s, u"e%1"_s.arg(edgeCount++));
        stream.writeAttribute(u"source"_s,
                              u"n%1"_s.arg(static_cast<int>(edge.sourceId())));
        stream.writeAttribute(u"target"_s,
                              u"n%1"_s.arg(static_cast<int>(edge.targetId())));
        for(const auto& edgeAttributeName : _graphModel->attributeNames(ElementType::Edge))
        {
            const auto* attribute = _graphModel->attributeByName(edgeAttributeName);
            if(attribute->hasParameter())
                continue;

            stream.writeStartElement(u"data"_s);
            stream.writeAttribute(u"key"_s, attributeToId.at(edgeAttributeName));
            stream.writeCharacters(attribute->stringValueOf(edgeId).toHtmlEscaped());
            stream.writeEndElement();
        }
        stream.writeEndElement(); // edge
    }
    stream.writeEndElement(); // graph
    stream.writeEndElement(); // graphml

    stream.writeEndDocument();
    return true;
}
