#include "saver.h"

#include "shared/plugins/iplugin.h"
#include "shared/utils/scope_exit.h"

#include "ui/document.h"

#include <QFile>
#include <QDataStream>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include <QStringList>

#include "thirdparty/zlib/zlib_disable_warnings.h"
#include "thirdparty/zlib/zlib.h"

static bool compress(const QByteArray& byteArray, const QString& filePath, const ProgressFn& progressFn)
{
    QFile file(filePath);

    if(!file.open(QIODevice::WriteOnly))
        return false;

    uint64_t totalBytes = byteArray.size();
    uint64_t bytePosition = 0;
    QDataStream input(byteArray);
    QDataStream output(&file);

    z_stream zstream = {};
    auto ret = deflateInit2(&zstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                            MAX_WBITS + 16, // 16 means write gzip header/trailer
                            8, Z_DEFAULT_STRATEGY);
    if(ret != Z_OK)
        return false;

    auto atExit = std::experimental::make_scope_exit([&zstream]
    {
       deflateEnd(&zstream);
    });
    Q_UNUSED(atExit);

    int flush = Z_ERRNO;

    do
    {
        const int ChunkSize = 1 << 14;
        unsigned char inBuffer[ChunkSize];

        auto numBytes = input.readRawData(reinterpret_cast<char*>(inBuffer), ChunkSize);

        bytePosition += numBytes;
        progressFn((bytePosition * 100) / totalBytes);

        zstream.avail_in = numBytes;
        zstream.next_in = inBuffer;
        flush = input.atEnd() ? Z_FINISH : Z_NO_FLUSH;

        do
        {
            unsigned char outBuffer[ChunkSize];
            zstream.avail_out = ChunkSize;
            zstream.next_out = outBuffer;

            ret = deflate(&zstream, flush);
            Q_ASSERT(ret != Z_STREAM_ERROR);

            numBytes = ChunkSize - zstream.avail_out;
            if(output.writeRawData(reinterpret_cast<const char*>(outBuffer), numBytes) != numBytes)
                return false;

        } while(zstream.avail_out == 0);
        Q_ASSERT(zstream.avail_in == 0);

    } while(flush != Z_FINISH);
    Q_ASSERT(ret == Z_STREAM_END);

    return true;
}

static QJsonObject graphAsJson(const IGraph& graph, const ProgressFn& progressFn)
{
    QJsonObject jsonObject;

    jsonObject["directed"] = true;

    int i;

    graph.setPhase(QObject::tr("Nodes")); i = 0;
    QJsonArray nodes;
    for(auto nodeId : graph.nodeIds())
    {
        QJsonObject node;
        node["id"] = static_cast<int>(nodeId);

        nodes.append(node);
        progressFn((i++ * 100) / graph.numNodes());
    }

    progressFn(-1);

    jsonObject["nodes"] = nodes;

    graph.setPhase(QObject::tr("Edges")); i = 0;
    QJsonArray edges;
    for(auto edgeId : graph.edgeIds())
    {
        const auto& edge = graph.edgeById(edgeId);

        QJsonObject jsonEdge;
        jsonEdge["id"] = static_cast<int>(edgeId);
        jsonEdge["source"] = static_cast<int>(edge.sourceId());
        jsonEdge["target"] = static_cast<int>(edge.targetId());

        edges.append(jsonEdge);
        progressFn((i++ * 100) / graph.numEdges());
    }

    progressFn(-1);

    jsonObject["edges"] = edges;

    return jsonObject;
}

static QJsonArray nodePositionsAsJson(const IGraph& graph, const NodePositions& nodePositions,
                                      const ProgressFn& progressFn)
{
    int i = 0;

    graph.setPhase(QObject::tr("Positions"));
    QJsonArray positions;
    for(auto nodeId : graph.nodeIds())
    {
        QJsonObject position;
        position["id"] = static_cast<int>(nodeId);

        const auto& nodePosition = nodePositions.get(nodeId);
        QJsonArray vector({nodePosition.x(), nodePosition.y(), nodePosition.z()});

        position["position"] = vector;

        positions.append(position);
        progressFn((i++ * 100) / graph.numNodes());
    }

    progressFn(-1);

    return positions;
}

static QJsonArray stringListToJsonArray(const QStringList& stringList)
{
    QJsonArray jsonArray;

    for(const auto& string : stringList)
        jsonArray.append(string);

    return jsonArray;
}

bool Saver::encode(const ProgressFn& progressFn)
{
    QJsonArray jsonArray;

    auto graphModel = _document->graphModel();

    QJsonObject header;
    header["version"] = 1;
    header["pluginName"] = graphModel->pluginName();
    header["pluginDataVersion"] = graphModel->pluginDataVersion();
    jsonArray.append(header);

    // The header must fit within a certain size, which is the maximum the loader will look at
    if(QJsonDocument(QJsonArray({header})).toJson().size() > MaxHeaderSize)
        return false;

    QJsonObject content;

    content["graph"] = graphAsJson(graphModel->mutableGraph(), progressFn);

    QJsonObject layout;

    layout["positions"] = nodePositionsAsJson(graphModel->mutableGraph(),
                                              graphModel->nodePositions(),
                                              progressFn);
    layout["paused"] = _document->layoutPauseState() == LayoutPauseState::Paused;
    content["layout"] = layout;

    content["transforms"] = stringListToJsonArray(_document->transforms());
    content["visualisations"] = stringListToJsonArray(_document->visualisations());

    graphModel->mutableGraph().setPhase(graphModel->pluginName());
    auto pluginData = _pluginInstance->save(graphModel->mutableGraph(), progressFn);

    progressFn(-1);

    auto pluginJson = QJsonDocument::fromJson(pluginData);

    // If the plugin data is itself JSON, just whack it in
    // as is, but if it's not, hex encode it
    if(!pluginJson.isNull() && pluginJson.isObject())
        content["pluginData"] = pluginJson.object();
    else if(!pluginJson.isNull() && pluginJson.isArray())
        content["pluginData"] = pluginJson.array();
    else
        content["pluginData"] = QString(pluginData.toHex());

    auto uiDataJson = QJsonDocument::fromJson(_uiData);

    if(!uiDataJson.isNull() && uiDataJson.isObject())
        content["uiData"] = uiDataJson.object();
    else if(!uiDataJson.isNull() && uiDataJson.isArray())
        content["uiData"] = uiDataJson.array();
    else
        content["uiData"] = QString(_uiData.toHex());

    jsonArray.append(content);

    graphModel->mutableGraph().setPhase(QObject::tr("Compressing"));
    return compress(QJsonDocument(jsonArray).toJson(), _fileUrl.path(), progressFn);
}

#include "thirdparty/zlib/zlib_enable_warnings.h"
