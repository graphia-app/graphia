#include "saver.h"

#include "shared/plugins/iplugin.h"
#include "shared/utils/scope_exit.h"
#include "shared/utils/iterator_range.h"
#include "shared/utils/string.h"

#include "graph/mutablegraph.h"
#include "graph/graphmodel.h"

#include "ui/document.h"

#include <QFile>
#include <QDataStream>
#include <QStringList>

#include <vector>

#include "thirdparty/zlib/zlib_disable_warnings.h"
#include "thirdparty/zlib/zlib.h"

static bool compress(const QByteArray& byteArray, const QString& filePath, Progressable& progressable)
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
        std::vector<unsigned char> inBuffer(ChunkSize);

        auto numBytes = input.readRawData(reinterpret_cast<char*>(inBuffer.data()), ChunkSize); // NOLINT

        bytePosition += numBytes;
        progressable.setProgress((bytePosition * 100) / totalBytes);

        zstream.avail_in = numBytes;
        zstream.next_in = static_cast<z_const Bytef*>(inBuffer.data());
        flush = input.atEnd() ? Z_FINISH : Z_NO_FLUSH;

        do
        {
            std::vector<unsigned char> outBuffer(ChunkSize);
            zstream.avail_out = ChunkSize;
            zstream.next_out = static_cast<Bytef*>(outBuffer.data());

            ret = deflate(&zstream, flush);
            if(ret == Z_STREAM_ERROR)
                return false;

            numBytes = ChunkSize - zstream.avail_out;
            if(output.writeRawData(reinterpret_cast<const char*>(outBuffer.data()), numBytes) != numBytes) // NOLINT
                return false;

        } while(zstream.avail_out == 0);
        Q_ASSERT(zstream.avail_in == 0);

    } while(flush != Z_FINISH);
    Q_ASSERT(ret == Z_STREAM_END);

    return true;
}

json Saver::graphAsJson(const IGraph& graph, Progressable& progressable)
{
    json jsonObject;

    jsonObject["directed"] = true;

    int i;

    graph.setPhase(QObject::tr("Nodes")); i = 0;
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

    graph.setPhase(QObject::tr("Edges")); i = 0;
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

static json nodeNamesAsJson(IGraphModel& graphModel,
                            Progressable& progressable)
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

static json nodePositionsAsJson(const IGraph& graph, const NodePositions& nodePositions,
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

static json bookmarksAsJson(const Document& document)
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

static json layoutSettingsAsJson(const Document& document)
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

bool Saver::encode(Progressable& progressable)
{
    json jsonArray;

    auto graphModel = dynamic_cast<GraphModel*>(_document->graphModel());
    Q_ASSERT(graphModel != nullptr);

    json header;
    header["version"] = 3;
    header["pluginName"] = graphModel->pluginName();
    header["pluginDataVersion"] = graphModel->pluginDataVersion();
    jsonArray.emplace_back(header);

    // The header must fit within a certain size, which is the maximum the loader will look at
    if(json({header}).dump().size() > MaxHeaderSize)
        return false;

    json content;

    content["graph"] = graphAsJson(graphModel->mutableGraph(), progressable);
    content["nodeNames"] = nodeNamesAsJson(*graphModel, progressable);

    json layout;

    layout["algorithm"] = _document->layoutName();
    layout["settings"] = layoutSettingsAsJson(*_document);
    layout["positions"] = nodePositionsAsJson(graphModel->mutableGraph(),
                                              graphModel->nodePositions(),
                                              progressable);
    layout["paused"] = _document->layoutPauseState() == LayoutPauseState::Paused;
    content["layout"] = layout;

    content["transforms"] = u::toQStringVector(_document->transforms());
    content["visualisations"] = u::toQStringVector(_document->visualisations());

    content["bookmarks"] = bookmarksAsJson(*_document);

    for(auto table : *_document->enrichmentTableModels())
        content["enrichmentTables"].push_back(table->toJson());

    auto uiDataJson = json::parse(_uiData.begin(), _uiData.end(), nullptr, false);

    if(uiDataJson.is_object() || uiDataJson.is_array())
        content["ui"] = uiDataJson;

    graphModel->mutableGraph().setPhase(graphModel->pluginName());
    auto pluginData = _pluginInstance->save(graphModel->mutableGraph(), progressable);

    progressable.setProgress(-1);

    auto pluginDataJson = json::parse(pluginData.begin(), pluginData.end(), nullptr, false);

    // If the plugin data is itself JSON, just whack it in
    // as is, but if it's not, hex encode it
    if(pluginDataJson.is_object() || pluginDataJson.is_array())
        content["pluginData"] = pluginDataJson;
    else
        content["pluginData"] = QString(pluginData.toHex());

    auto pluginUiDataJson = json::parse(_pluginUiData.begin(), _pluginUiData.end(), nullptr, false);

    if(pluginUiDataJson.is_object() || pluginUiDataJson.is_array())
        content["pluginUiData"] = pluginUiDataJson;
    else
        content["pluginUiData"] = QString(_pluginUiData.toHex());

    jsonArray.emplace_back(content);

    graphModel->mutableGraph().setPhase(QObject::tr("Compressing"));
    return compress(QByteArray::fromStdString(jsonArray.dump()), _fileUrl.toLocalFile(), progressable);
}

#include "thirdparty/zlib/zlib_enable_warnings.h"
