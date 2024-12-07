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

#include "jsongraphsaver.h"

#include "nativesaver.h"
#include "shared/attributes/iattribute.h"
#include "shared/graph/igraph.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"

#include <json_helper.h>

#include <QDebug>
#include <QFile>

bool JSONGraphSaver::save()
{
    json fileObject;
    fileObject["graph"] = graphAsJson(_graphModel->graph(), *this);
    setProgress(-1);

    const size_t numElements = _graphModel->graph().numNodes() + _graphModel->graph().numEdges();
    size_t runningCount = 0;

    // Node Attributes
    setPhase(QObject::tr("Node Attributes"));
    for(auto& node : fileObject["graph"]["nodes"])
    {
        const NodeId nodeId = std::stoi(node["id"].get<std::string>());
        for(const auto& nodeAttributeName : _graphModel->attributeNames(ElementType::Node))
        {
            const auto* attribute = _graphModel->attributeByName(nodeAttributeName);
            if(attribute->hasParameter())
                continue;

            auto byteArray = nodeAttributeName.toUtf8();
            const auto* name = byteArray.constData();

            if(attribute->valueType() == ValueType::String)
                node["metadata"][name] = attribute->stringValueOf(nodeId);
            else if(attribute->valueType() == ValueType::Int)
                node["metadata"][name] = attribute->intValueOf(nodeId);
            else if(attribute->valueType() == ValueType::Float)
                node["metadata"][name] = attribute->floatValueOf(nodeId);
        }

        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / numElements));
    }

    // Edge Attributes
    setPhase(QObject::tr("Edge Attributes"));
    for(auto& edge : fileObject["graph"]["edges"])
    {
        const EdgeId edgeId = std::stoi(edge["id"].get<std::string>());
        for(const auto& edgeAttributeName : _graphModel->attributeNames(ElementType::Edge))
        {
            const auto* attribute = _graphModel->attributeByName(edgeAttributeName);
            if(attribute->hasParameter())
                continue;

            auto byteArray = edgeAttributeName.toUtf8();
            const auto* name = byteArray.constData();

            if(attribute->valueType() == ValueType::String)
                edge["metadata"][name] = attribute->stringValueOf(edgeId);
            else if(attribute->valueType() == ValueType::Int)
                edge["metadata"][name] = attribute->intValueOf(edgeId);
            else if(attribute->valueType() == ValueType::Float)
                edge["metadata"][name] = attribute->floatValueOf(edgeId);
        }

        runningCount++;
        setProgress(static_cast<int>(runningCount * 100 / numElements));
    }

    QFile file(_url.toLocalFile());
    file.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text);
    file.write(QByteArray::fromStdString(fileObject.dump()));
    file.close();
    return true;
}

json JSONGraphSaver::graphAsJson(const IGraph& graph, Progressable& progressable)
{
    json jsonObject;

    jsonObject["directed"] = true;

    int i= 0;

    progressable.setPhase(QObject::tr("Nodes"));
    i = 0;
    json nodes;
    for(auto nodeId : graph.nodeIds())
    {
        json node;
        node["id"] = std::to_string(static_cast<int>(nodeId));

        nodes.emplace_back(node);
        progressable.setProgress((i++ * 100) /
            static_cast<int>(graph.numNodes()));
    }

    progressable.setProgress(-1);

    jsonObject["nodes"] = nodes;

    progressable.setPhase(QObject::tr("Edges"));
    i = 0;
    json edges;
    for(auto edgeId : graph.edgeIds())
    {
        const auto& edge = graph.edgeById(edgeId);

        json jsonEdge;
        jsonEdge["id"] = std::to_string(static_cast<int>(edgeId));
        jsonEdge["source"] = std::to_string(static_cast<int>(edge.sourceId()));
        jsonEdge["target"] = std::to_string(static_cast<int>(edge.targetId()));

        edges.emplace_back(jsonEdge);
        progressable.setProgress((i++ * 100) /
            static_cast<int>(graph.numEdges()));
    }

    progressable.setProgress(-1);

    jsonObject["edges"] = edges;

    return jsonObject;
}
