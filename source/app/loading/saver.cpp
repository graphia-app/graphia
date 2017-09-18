#include "saver.h"

#include "shared/plugins/iplugin.h"
#include "shared/utils/scope_exit.h"

#include "ui/document.h"

#include <QFile>
#include <QDataStream>

#include "json_helper.h"

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
        zstream.next_in = static_cast<z_const Bytef*>(inBuffer);
        flush = input.atEnd() ? Z_FINISH : Z_NO_FLUSH;

        do
        {
            unsigned char outBuffer[ChunkSize];
            zstream.avail_out = ChunkSize;
            zstream.next_out = static_cast<Bytef*>(outBuffer);

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

static json graphAsJson(const IGraph& graph, const ProgressFn& progressFn)
{
    json jsonObject;

    jsonObject["directed"] = true;

    int i;

    graph.setPhase(QObject::tr("Nodes")); i = 0;
    json nodes;
    for(auto nodeId : graph.nodeIds())
    {
        json node;
        node["id"] = static_cast<int>(nodeId);

        nodes.emplace_back(node);
        progressFn((i++ * 100) / graph.numNodes());
    }

    progressFn(-1);

    jsonObject["nodes"] = nodes;

    graph.setPhase(QObject::tr("Edges")); i = 0;
    json edges;
    for(auto edgeId : graph.edgeIds())
    {
        const auto& edge = graph.edgeById(edgeId);

        json jsonEdge;
        jsonEdge["id"] = static_cast<int>(edgeId);
        jsonEdge["source"] = static_cast<int>(edge.sourceId());
        jsonEdge["target"] = static_cast<int>(edge.targetId());

        edges.emplace_back(jsonEdge);
        progressFn((i++ * 100) / graph.numEdges());
    }

    progressFn(-1);

    jsonObject["edges"] = edges;

    return jsonObject;
}

static json nodePositionsAsJson(const IGraph& graph, const NodePositions& nodePositions,
                                const ProgressFn& progressFn)
{
    int i = 0;

    graph.setPhase(QObject::tr("Positions"));
    json positions;
    for(auto nodeId : graph.nodeIds())
    {
        const auto& nodePosition = nodePositions.get(nodeId);
        json vector({nodePosition.x(), nodePosition.y(), nodePosition.z()});

        positions.emplace_back(vector);
        progressFn((i++ * 100) / graph.numNodes());
    }

    progressFn(-1);

    return positions;
}

bool Saver::encode(const ProgressFn& progressFn)
{
    json jsonArray;

    auto graphModel = _document->graphModel();

    json header;
    header["version"] = 1;
    header["pluginName"] = graphModel->pluginName();
    header["pluginDataVersion"] = graphModel->pluginDataVersion();
    jsonArray.emplace_back(header);

    // The header must fit within a certain size, which is the maximum the loader will look at
    if(json({header}).dump().size() > MaxHeaderSize)
        return false;

    json content;

    content["graph"] = graphAsJson(graphModel->mutableGraph(), progressFn);

    json layout;

    layout["positions"] = nodePositionsAsJson(graphModel->mutableGraph(),
                                              graphModel->nodePositions(),
                                              progressFn);
    layout["paused"] = _document->layoutPauseState() == LayoutPauseState::Paused;
    content["layout"] = layout;

    content["transforms"] = u::toQStringVector(_document->transforms());
    content["visualisations"] = u::toQStringVector(_document->visualisations());

    graphModel->mutableGraph().setPhase(graphModel->pluginName());
    auto pluginData = _pluginInstance->save(graphModel->mutableGraph(), progressFn);

    progressFn(-1);

    auto pluginDataJson = json::parse(pluginData.begin(), pluginData.end(), nullptr, false);

    // If the plugin data is itself JSON, just whack it in
    // as is, but if it's not, hex encode it
    if(pluginDataJson.is_object() || pluginDataJson.is_array())
        content["pluginData"] = pluginDataJson;
    else
        content["pluginData"] = QString(pluginData.toHex());

    auto uiDataJson = json::parse(_uiData.begin(), _uiData.end(), nullptr, false);

    if(uiDataJson.is_object() || uiDataJson.is_array())
        content["ui"] = uiDataJson;
    else
        content["ui"] = QString(_uiData.toHex());

    jsonArray.emplace_back(content);

    graphModel->mutableGraph().setPhase(QObject::tr("Compressing"));
    return compress(QByteArray::fromStdString(jsonArray.dump()), _fileUrl.toLocalFile(), progressFn);
}

#include "thirdparty/zlib/zlib_enable_warnings.h"
