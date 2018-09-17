#include "jsongraphsaver.h"

#include "json_helper.h"
#include "nativesaver.h"
#include "shared/attributes/iattribute.h"
#include "shared/graph/igraph.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"
#include "ui/document.h"

#include <QDebug>
#include <QFile>

bool JSONGraphSaver::save()
{
    json fileObject;
    fileObject["graph"] = graphAsJson(_graphModel->graph(), *this);
    setProgress(-1);

    int numElements = _graphModel->graph().numNodes() + _graphModel->graph().numEdges();
    int runningCount = 0;

    // Node Attributes
    _graphModel->mutableGraph().setPhase(QObject::tr("Node Attributes"));
    for(auto& node : fileObject["graph"]["nodes"])
    {
        NodeId nodeId = std::stoi(node["id"].get<std::string>());
        for(const auto& nodeAttributeName : _graphModel->attributeNames(ElementType::Node))
        {
            const auto& attribute = _graphModel->attributeByName(nodeAttributeName);

            auto byteArray = nodeAttributeName.toUtf8();
            auto name = byteArray.constData();

            if(attribute->valueType() == ValueType::String)
                node["metadata"][name] = attribute->stringValueOf(nodeId);
            else if(attribute->valueType() == ValueType::Int)
                node["metadata"][name] = attribute->intValueOf(nodeId);
            else if(attribute->valueType() == ValueType::Float)
                node["metadata"][name] = attribute->floatValueOf(nodeId);
        }

        runningCount++;
        setProgress(runningCount * 100 / numElements);
    }

    // Edge Attributes
    _graphModel->mutableGraph().setPhase(QObject::tr("Edge Attributes"));
    for(auto& edge : fileObject["graph"]["edges"])
    {
        EdgeId edgeId = std::stoi(edge["id"].get<std::string>());
        for(const auto& edgeAttributeName : _graphModel->attributeNames(ElementType::Edge))
        {
            const auto& attribute = _graphModel->attributeByName(edgeAttributeName);

            auto byteArray = edgeAttributeName.toUtf8();
            auto name = byteArray.constData();

            if(attribute->valueType() == ValueType::String)
                edge["metadata"][name] = attribute->stringValueOf(edgeId);
            else if(attribute->valueType() == ValueType::Int)
                edge["metadata"][name] = attribute->intValueOf(edgeId);
            else if(attribute->valueType() == ValueType::Float)
                edge["metadata"][name] = attribute->floatValueOf(edgeId);
        }

        runningCount++;
        setProgress(runningCount * 100 / numElements);
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

    int i;

    graph.setPhase(QObject::tr("Nodes"));
    i = 0;
    json nodes;
    for(auto nodeId : graph.nodeIds())
    {
        json node;
        node["id"] = std::to_string(static_cast<int>(nodeId));

        nodes.emplace_back(node);
        progressable.setProgress((i++ * 100) / graph.numNodes());
    }

    progressable.setProgress(-1);

    jsonObject["nodes"] = nodes;

    graph.setPhase(QObject::tr("Edges"));
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
        progressable.setProgress((i++ * 100) / graph.numEdges());
    }

    progressable.setProgress(-1);

    jsonObject["edges"] = edges;

    return jsonObject;
}

json JSONGraphSaver::nodeNamesAsJson(IGraphModel& graphModel, Progressable& progressable)
{
    graphModel.mutableGraph().setPhase(QObject::tr("Names"));
    json names;

    uint64_t i = 0;
    const auto& nodeIds = graphModel.mutableGraph().nodeIds();
    for(NodeId nodeId : nodeIds)
    {
        names.emplace_back(graphModel.nodeName(nodeId));
        progressable.setProgress(static_cast<int>((i++ * 100) / nodeIds.size()));
    }

    progressable.setProgress(-1);

    return names;
}

json JSONGraphSaver::nodePositionsAsJson(const IGraph& graph, const NodePositions& nodePositions,
                                Progressable& progressable)
{
    graph.setPhase(QObject::tr("Positions"));
    json positions;

    auto numNodePositions = std::distance(nodePositions.begin(), nodePositions.end());
    uint64_t i = 0;
    for(const auto& nodePosition : nodePositions)
    {
        auto v = nodePosition.newest();
        json vector({v.x(), v.y(), v.z()});

        positions.emplace_back(vector);
        progressable.setProgress((i++ * 100) / numNodePositions);
    }

    progressable.setProgress(-1);

    return positions;
}

json JSONGraphSaver::bookmarksAsJson(const Document& document)
{
    json jsonObject = json::object();

    auto bookmarks = document.bookmarks();
    for(const auto& bookmark : bookmarks)
    {
        json nodeIds;
        for(auto nodeId : document.nodeIdsForBookmark(bookmark))
            nodeIds.emplace_back(static_cast<int>(nodeId));

        auto byteArray = bookmark.toUtf8();
        auto bookmarkName = byteArray.constData();
        jsonObject[bookmarkName] = nodeIds;
    }

    return jsonObject;
}

json JSONGraphSaver::layoutSettingsAsJson(const Document& document)
{
    json jsonObject;

    auto settings = document.layoutSettings();
    for(const auto& setting : settings)
    {
        auto byteArray = setting.name().toUtf8();
        auto settingName = byteArray.constData();
        jsonObject[settingName] = setting.value();
    }

    return jsonObject;
}
