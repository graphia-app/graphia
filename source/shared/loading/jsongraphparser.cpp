#include "jsongraphparser.h"

#include "shared/utils/container.h"
#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"

#include <QDataStream>
#include <QFile>
#include <QUrl>
#include <QDebug>

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

        // Check if we've read more than we've been asked to
        if(-1 >= 0 && bytesRead >= -1)
            return true;

    } while(!input.atEnd());

    auto jsonBody = parseJsonFrom(byteArray, *this);

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

    qDebug() << "Parsing";

    const auto& jsonNodes = jsonGraphObject["nodes"];
    const auto& jsonEdges = jsonGraphObject["edges"];

    uint64_t i = 0;

    graphModel->mutableGraph().setPhase(QObject::tr("Nodes"));

    std::map<std::string, NodeId> jsonIdToNodeId;
    for(const auto& jsonNode : jsonNodes)
    {
        NodeId nodeId;
        std::string nodeJsonId = "";

        if(u::contains(jsonNode, "id"))
        {
            nodeJsonId = jsonNode["id"].get<std::string>();
            nodeId = graphModel->mutableGraph().addNode();
            jsonIdToNodeId[nodeJsonId] = nodeId;

            if(u::contains(jsonNode, "label"))
            {
                std::string nodeJsonLabel = jsonNode["label"].get<std::string>();
                graphModel->setNodeName(nodeId, QString::fromStdString(nodeJsonLabel));
            }

            if(u::contains(jsonNode, "metadata") && userNodeData != nullptr)
            {
                auto metadata = jsonNode["metadata"];
                for(auto it = metadata.begin(); it != metadata.end(); ++it)
                {
                    qDebug() << QString::fromStdString(it.key());
                    if(it.value().is_string())
                        userNodeData->setValueBy(nodeId, QString::fromStdString(it.key()), QString::fromStdString(it.value().get<std::string>()));
                    else if(it.value().is_number_integer())
                        userNodeData->setValueBy(nodeId, QString::fromStdString(it.key()), QString::number(it.value().get<int>()));
                    else if(it.value().is_number_float())
                        userNodeData->setValueBy(nodeId, QString::fromStdString(it.key()), QString::number(it.value().get<double>()));
                }
            }
        }

        progressable.setProgress(static_cast<int>((i++ * 100) / jsonNodes.size()));
    }

    qDebug() << "json nodes Done";

    progressable.setProgress(-1);

    i = 0;

    graphModel->mutableGraph().setPhase(QObject::tr("Edges"));
    for(const auto& jsonEdge : jsonEdges)
    {
        EdgeId edgeId;
        NodeId sourceId;
        NodeId targetId;

        if(u::contains(jsonEdge, "source") && u::contains(jsonEdge, "target"))
        {
            sourceId = jsonIdToNodeId.at(jsonEdge["source"].get<std::string>());
            targetId = jsonIdToNodeId.at(jsonEdge["target"].get<std::string>());

            edgeId = graphModel->mutableGraph().addEdge(sourceId, targetId);

            if(u::contains(jsonEdge, "metadata") && userEdgeData != nullptr)
            {
                auto metadata = jsonEdge["metadata"];
                for(auto it = metadata.begin(); it != metadata.end(); ++it)
                {
                    if(it.value().is_string())
                        userEdgeData->setValueBy(edgeId, QString::fromStdString(it.key()), QString::fromStdString(it.value().get<std::string>()));
                    else if(it.value().is_number_integer())
                        userEdgeData->setValueBy(edgeId, QString::fromStdString(it.key()), QString::number(it.value().get<int>()));
                    else if(it.value().is_number_float())
                        userEdgeData->setValueBy(edgeId, QString::fromStdString(it.key()), QString::number(it.value().get<double>()));
                }
            }
        }

        progressable.setProgress(static_cast<int>((i++ * 100) / jsonEdges.size()));
    }

    qDebug() << "json edges Done";

    progressable.setProgress(-1);
    return true;
}
