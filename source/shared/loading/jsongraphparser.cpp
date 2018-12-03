#include "jsongraphparser.h"

#include "shared/utils/container.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"

#include <QDataStream>
#include <QFile>
#include <QUrl>

static QString jsonIdToQString(const json& idObject)
{
    if(idObject.is_number_integer())
        return QString::number(idObject.get<int>());

    if(idObject.is_string())
        return QString::fromStdString(idObject.get<std::string>());

    return {};
}

bool JsonGraphParser::parse(const QUrl &url, IGraphModel *graphModel)
{
    QFile file(url.toLocalFile());
    QByteArray byteArray;

    if(!file.open(QIODevice::ReadOnly))
        return false;

    auto totalBytes = file.size();

    if(totalBytes == 0)
        return false;

    int bytesRead = 0;
    QDataStream input(&file);

    do
    {
        const int ChunkSize = 2 << 16;
        std::vector<unsigned char> buffer(ChunkSize);

        auto numBytes = input.readRawData(reinterpret_cast<char*>(buffer.data()), ChunkSize);
        byteArray.append(reinterpret_cast<char*>(buffer.data()), numBytes);

        bytesRead += numBytes;

        setProgress((bytesRead * 100) / totalBytes);
    } while(!input.atEnd());

    auto jsonBody = parseJsonFrom(byteArray, this);

    if(cancelled())
        return false;

    if(jsonBody.is_null() || !jsonBody.is_object())
        return false;

    if(!u::contains(jsonBody, "graph") || !jsonBody["graph"].is_object())
        return false;

    return parseGraphObject(jsonBody["graph"], graphModel, *this, _userNodeData, _userEdgeData);
}

bool JsonGraphParser::parseGraphObject(const json& jsonGraphObject, IGraphModel* graphModel,
                                       Progressable& progressable, UserNodeData* userNodeData,
                                       UserEdgeData* userEdgeData)
{
    if(!u::contains(jsonGraphObject, "nodes") || !u::contains(jsonGraphObject, "edges"))
        return false;

    const auto& jsonNodes = jsonGraphObject["nodes"];
    const auto& jsonEdges = jsonGraphObject["edges"];

    uint64_t i = 0;

    graphModel->mutableGraph().setPhase(QObject::tr("Nodes"));

    std::map<QString, NodeId> jsonIdToNodeId;
    for(const auto& jsonNode : jsonNodes)
    {
        NodeId nodeId;
        QString nodeJsonId;

        if(!u::contains(jsonNode, "id"))
            continue;

        nodeJsonId = jsonIdToQString(jsonNode["id"]);
        if(nodeJsonId.isEmpty())
            continue;

        nodeId = graphModel->mutableGraph().addNode();
        jsonIdToNodeId[nodeJsonId] = nodeId;

        if(u::contains(jsonNode, "label"))
        {
            auto nodeJsonLabel = jsonNode["label"].get<std::string>();
            graphModel->setNodeName(nodeId, QString::fromStdString(nodeJsonLabel));
        }

        if(u::contains(jsonNode, "metadata") && userNodeData != nullptr)
        {
            auto metadata = jsonNode["metadata"];
            for(auto it = metadata.begin(); it != metadata.end(); ++it)
            {
                auto key = QString::fromStdString(it.key());
                QString value;

                if(it.value().is_string())
                    value = QString::fromStdString(it.value().get<std::string>());
                else if(it.value().is_number_integer())
                    value = QString::number(it.value().get<int>());
                else if(it.value().is_number_float())
                    value = QString::number(it.value().get<double>());

                userNodeData->setValueBy(nodeId, key, value);
            }
        }

        progressable.setProgress(static_cast<int>((i++ * 100) / jsonNodes.size()));
    }

    progressable.setProgress(-1);

    i = 0;

    graphModel->mutableGraph().setPhase(QObject::tr("Edges"));
    for(const auto& jsonEdge : jsonEdges)
    {
        if(!u::contains(jsonEdge, "source") || !u::contains(jsonEdge, "target"))
            continue;

        QString jsonSourceId = jsonIdToQString(jsonEdge["source"]);
        if(jsonSourceId.isEmpty())
            continue;

        QString jsonTargetId = jsonIdToQString(jsonEdge["target"]);
        if(jsonTargetId.isEmpty())
            continue;

        NodeId sourceId = jsonIdToNodeId.at(jsonSourceId);
        NodeId targetId = jsonIdToNodeId.at(jsonTargetId);
        EdgeId edgeId = graphModel->mutableGraph().addEdge(sourceId, targetId);

        if(u::contains(jsonEdge, "metadata") && userEdgeData != nullptr)
        {
            auto metadata = jsonEdge["metadata"];
            for(auto it = metadata.begin(); it != metadata.end(); ++it)
            {
                QString key = QString::fromStdString(it.key());
                QString value;

                if(it.value().is_string())
                    value = QString::fromStdString(it.value().get<std::string>());
                else if(it.value().is_number_integer())
                    value = QString::number(it.value().get<int>());
                else if(it.value().is_number_float())
                    value = QString::number(it.value().get<double>());

                userEdgeData->setValueBy(edgeId, key, value);
            }

        }

        progressable.setProgress(static_cast<int>((i++ * 100) / jsonEdges.size()));
    }

    progressable.setProgress(-1);
    return true;
}
