/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#include "app/graph/graphmodel.h"
#include "app/layout/nodepositions.h"
#include "shared/graph/igraph.h"
#include "shared/attributes/iattribute.h"
#include "shared/utils/container.h"

#include <QFile>
#include <QLocale>
#include <QObject>
#include <QXmlStreamWriter>
#include <QtGlobal>

#include <cstddef>
#include <map>
#include <set>

using namespace Qt::Literals::StringLiterals;

// Attribute::stringValueOf limits the precision of floating point values
template<typename E>
static QString valueAsString(const IAttribute& attribute, E elementId)
{
    if(attribute.valueType() == ValueType::Float)
        return QString::number(attribute.floatValueOf(elementId), 'g', QLocale::FloatingPointShortest);

    return attribute.displayValue(attribute.stringValueOf(elementId));
}

static QString valueAsString(float value)
{
    return QString::number(static_cast<double>(value), 'g', QLocale::FloatingPointShortest);
}

bool GraphMLSaver::save()
{
    auto* graphModel = dynamic_cast<GraphModel*>(_graphModel);

    Q_ASSERT(graphModel != nullptr);
    if(graphModel == nullptr)
        return false;

    QFile file(_url.toLocalFile());
    if(!file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text))
        return false;

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
    const auto exportedNames = exportedAttributeNames(*_graphModel);
    for(const auto& attributeName : _graphModel->attributeNames())
    {
        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / numElements));

        const auto* attribute = _graphModel->attributeByName(attributeName);
        if(attribute->hasParameter() || !u::contains(exportedNames, attributeName))
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

        stream.writeAttribute(u"attr.name"_s, exportedNames.at(attributeName));

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

    // Nodes are identified by their names, so they keep them when the file is loaded
    // again, unless any are the same as another's
    bool idsAreNames = true;
    std::set<QString> nodeNames;
    for(auto nodeId : _graphModel->graph().nodeIds())
    {
        if(!nodeNames.insert(_graphModel->nodeName(nodeId)).second)
        {
            idsAreNames = false;
            break;
        }
    }

    auto idOf = [&](NodeId nodeId)
    {
        return idsAreNames ? _graphModel->nodeName(nodeId) : u"n%1"_s.arg(static_cast<int>(nodeId));
    };

    stream.writeStartElement(u"graph"_s);
    stream.writeAttribute(u"edgedefault"_s, u"directed"_s);

    setPhase(QObject::tr("Nodes"));
    for(auto nodeId : _graphModel->graph().nodeIds())
    {
        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / numElements));

        stream.writeStartElement(u"node"_s);
        stream.writeAttribute(u"id"_s, idOf(nodeId));
        for(const auto& nodeAttributeName : _graphModel->attributeNames(ElementType::Node))
        {
            const auto* attribute = _graphModel->attributeByName(nodeAttributeName);
            if(attribute->hasParameter() || !u::contains(exportedNames, nodeAttributeName))
                continue;

            if((attribute->valueType() & ValueType::Numerical) && attribute->valueMissingOf(nodeId))
                continue;

            stream.writeStartElement(u"data"_s);
            stream.writeAttribute(u"key"_s, attributeToId.at(nodeAttributeName));
            stream.writeCharacters(valueAsString(*attribute, nodeId));
            stream.writeEndElement();
        }

        stream.writeStartElement(u"data"_s);
        stream.writeAttribute(u"key"_s, u"desc"_s);
        stream.writeCharacters(_graphModel->nodeName(nodeId));
        stream.writeEndElement();

        const auto& pos = graphModel->nodePositions().get(nodeId);
        stream.writeStartElement(u"data"_s);
        stream.writeAttribute(u"key"_s, u"x"_s);
        stream.writeCharacters(valueAsString(pos.x()));
        stream.writeEndElement();
        stream.writeStartElement(u"data"_s);
        stream.writeAttribute(u"key"_s, u"y"_s);
        stream.writeCharacters(valueAsString(pos.y()));
        stream.writeEndElement();
        stream.writeStartElement(u"data"_s);
        stream.writeAttribute(u"key"_s, u"z"_s);
        stream.writeCharacters(valueAsString(pos.z()));
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
        stream.writeAttribute(u"source"_s, idOf(edge.sourceId()));
        stream.writeAttribute(u"target"_s, idOf(edge.targetId()));
        for(const auto& edgeAttributeName : _graphModel->attributeNames(ElementType::Edge))
        {
            const auto* attribute = _graphModel->attributeByName(edgeAttributeName);
            if(attribute->hasParameter() || !u::contains(exportedNames, edgeAttributeName))
                continue;

            if((attribute->valueType() & ValueType::Numerical) && attribute->valueMissingOf(edgeId))
                continue;

            stream.writeStartElement(u"data"_s);
            stream.writeAttribute(u"key"_s, attributeToId.at(edgeAttributeName));
            stream.writeCharacters(valueAsString(*attribute, edgeId));
            stream.writeEndElement();
        }
        stream.writeEndElement(); // edge
    }
    stream.writeEndElement(); // graph
    stream.writeEndElement(); // graphml

    stream.writeEndDocument();
    return true;
}
