#include "loader.h"
#include "saver.h"

#include "application.h"

#include "shared/graph/imutablegraph.h"
#include "shared/plugins/iplugin.h"
#include "shared/utils/scope_exit.h"

#include <QString>
#include <QFileInfo>
#include <QDataStream>

#include "json_helper.h"

#include "thirdparty/zlib/zlib_disable_warnings.h"
#include "thirdparty/zlib/zlib.h"

static bool isCompressed(const QString& filePath)
{
    QFile file(filePath);

    const int GzipHeaderSize = 10;
    if(!file.open(QIODevice::ReadOnly) || file.size() < GzipHeaderSize)
        return false;

    unsigned char header[GzipHeaderSize];

    if(file.read(reinterpret_cast<char*>(header), GzipHeaderSize) != GzipHeaderSize)
        return false;

    // Gzip magic number
    return header[0] == 0x1f && header[1] == 0x8b;
}

static bool decompress(const QString& filePath, QByteArray& byteArray,
                       int maxReadSize = -1, ProgressFn progressFn = [](int){})
{
    QFile file(filePath);

    if(!file.open(QIODevice::ReadOnly))
        return false;

    uint64_t totalBytes = file.size();
    uint64_t bytesRead = 0;
    uint64_t bytesDecompressed = 0;
    QDataStream input(&file);

    z_stream zstream = {};
    auto ret = inflateInit2(&zstream, MAX_WBITS + 32); // 32 means read gzip header/trailer
    if(ret != Z_OK)
        return false;

    auto atExit = std::experimental::make_scope_exit([&zstream]
    {
       inflateEnd(&zstream);
    });
    Q_UNUSED(atExit);

    do
    {
        const int ChunkSize = 1 << 14;
        unsigned char inBuffer[ChunkSize];

        auto numBytes = input.readRawData(reinterpret_cast<char*>(inBuffer), ChunkSize);

        bytesRead += numBytes;
        progressFn((bytesRead * 100) / totalBytes);

        zstream.avail_in = numBytes;
        if(zstream.avail_in == 0)
            break;

        zstream.next_in = inBuffer;

        do
        {
            unsigned char outBuffer[ChunkSize];
            zstream.avail_out = ChunkSize;
            zstream.next_out = outBuffer;

            ret = inflate(&zstream, Z_NO_FLUSH);
            Q_ASSERT(ret != Z_STREAM_ERROR);

            switch(ret)
            {
            case Z_NEED_DICT:
                ret = Z_DATA_ERROR;
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                return false;
            }

            numBytes = ChunkSize - zstream.avail_out;
            bytesDecompressed += numBytes;
            byteArray.append(reinterpret_cast<const char*>(outBuffer), numBytes);

            // Check if we've read more than we've been asked to
            if(maxReadSize >= 0 && bytesDecompressed >= static_cast<uint64_t>(maxReadSize))
                return true;

        } while (zstream.avail_out == 0);
    } while (ret != Z_STREAM_END);

    return true;
}

static bool load(const QString& filePath, QByteArray& byteArray,
                 int maxReadSize = -1, IGraph* graph = nullptr,
                 ProgressFn progressFn = [](int){})
{
    if(isCompressed(filePath))
    {
        if(graph != nullptr)
            graph->setPhase(QObject::tr("Decompressing"));

        return decompress(filePath, byteArray, maxReadSize, progressFn);
    }

    QFile file(filePath);

    if(!file.open(QIODevice::ReadOnly))
        return false;

    auto totalBytes = file.size();
    int bytesRead = 0;
    QDataStream input(&file);

    do
    {
        const int ChunkSize = 2 << 16;
        char buffer[ChunkSize];

        auto numBytes = input.readRawData(buffer, ChunkSize);
        byteArray.append(buffer, numBytes);

        bytesRead += numBytes;
        progressFn((bytesRead * 100) / totalBytes);

        // Check if we've read more than we've been asked to
        if(maxReadSize >= 0 && bytesRead >= maxReadSize)
            return true;

    } while(!input.atEnd());

    return true;
}

struct Header
{
    int _version;
    QString _pluginName;
    int _pluginDataVersion;
};

static bool parseHeader(const QUrl& url, Header* header = nullptr)
{
    QByteArray byteArray;

    if(!load(url.toLocalFile(), byteArray, Saver::MaxHeaderSize))
        return false;

    // byteArray now has a JSON fragment that hopefully includes the header
    QString fragment(byteArray);

    // Strip off the leading [
    fragment.replace(QRegularExpression(R"(^\s*\[\s*)"), "");

    int position = 0;
    int braceDepth = 0;
    bool inQuotes = false;
    bool escaping = false;
    for(auto character : fragment)
    {
        if(!escaping)
        {
            switch(character.toLatin1())
            {
            case '{': braceDepth++; break;
            case '}': braceDepth--; break;
            case '"': inQuotes = !inQuotes; break;
            case '\\': escaping = true; break;
            default: break;
            }
        }
        else
            escaping = false;

        position++;

        if(braceDepth == 0)
            break;
    }

    QString headerString = fragment.left(position);
    auto headerByteArray = headerString.toUtf8();
    json jsonHeader = json::parse(headerByteArray.begin(), headerByteArray.end(), nullptr, false);

    if(jsonHeader.is_null() || !jsonHeader.is_object())
        return false;

    if(!u::contains(jsonHeader, "version"))
        return false;

    if(!u::contains(jsonHeader, "pluginName"))
        return false;

    if(!u::contains(jsonHeader, "pluginDataVersion"))
        return false;

    if(header != nullptr)
    {
        header->_version            = jsonHeader["version"];
        header->_pluginName         = QString::fromStdString(jsonHeader["pluginName"]);
        header->_pluginDataVersion  = jsonHeader["pluginDataVersion"];
    }

    return true;
}

bool Loader::parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progressFn)
{
    Header header;
    if(!parseHeader(url, &header))
        return false;

    QByteArray byteArray;

    if(!load(url.toLocalFile(), byteArray, -1, &graph, progressFn))
        return false;

    progressFn(-1);

    auto jsonArray = json::parse(byteArray.begin(), byteArray.end(), nullptr, false);

    if(jsonArray.is_null() || !jsonArray.is_array())
        return false;

    if(jsonArray.size() != 2)
        return false;

    for(const auto& i : jsonArray)
    {
        if(!i.is_object())
            return false;
    }

    auto jsonBody = jsonArray.at(1);

    if(!u::contains(jsonBody, "graph") || !jsonBody["graph"].is_object())
        return false;

    const auto& jsonGraph = jsonBody["graph"];

    if(!u::contains(jsonGraph, "nodes") || !u::contains(jsonGraph, "edges"))
        return false;

    if(!u::contains(jsonGraph, "nodes") || !u::contains(jsonGraph, "edges"))
        return false;

    const auto& jsonNodes = jsonGraph["nodes"];
    const auto& jsonEdges = jsonGraph["edges"];

    uint64_t i = 0;

    graph.setPhase(QObject::tr("Nodes"));
    for(const auto& jsonNode : jsonNodes)
    {
        NodeId nodeId = jsonNode["id"].get<int>();

        if(!nodeId.isNull())
        {
            graph.reserveNodeId(nodeId);
            graph.addNode(nodeId);
        }

        progressFn(static_cast<int>((i++ * 100) /jsonNodes.size()));
    }

    progressFn(-1);

    i = 0;

    graph.setPhase(QObject::tr("Edges"));
    for(const auto& jsonEdge : jsonEdges)
    {
        EdgeId edgeId = jsonEdge["id"].get<int>();
        NodeId sourceId = jsonEdge["source"].get<int>();
        NodeId targetId = jsonEdge["target"].get<int>();

        if(!edgeId.isNull() && !sourceId.isNull() && !targetId.isNull())
        {
            graph.reserveEdgeId(edgeId);
            graph.addEdge(edgeId, sourceId, targetId);
        }

        progressFn(static_cast<int>((i++ * 100) / jsonEdges.size()));
    }

    progressFn(-1);

    if(u::contains(jsonBody, "transforms"))
    {
        for(const auto& transform : jsonBody["transforms"])
            _transforms.append(QString::fromStdString(transform));
    }

    if(u::contains(jsonBody, "visualisations"))
    {
        for(const auto& visualisation : jsonBody["visualisations"])
            _visualisations.append(QString::fromStdString(visualisation));
    }

    if(u::contains(jsonBody, "layout"))
    {
        const auto& jsonLayout = jsonBody["layout"];

        if(u::contains(jsonLayout, "positions"))
        {
            _nodePositions = std::make_unique<ExactNodePositions>(graph);

            NodeId nodeId(0);
            for(const auto& jsonPosition : jsonLayout["positions"])
            {
                const auto& jsonPositionArray = jsonPosition;

                _nodePositions->set(nodeId++, QVector3D(
                    jsonPositionArray.at(0),
                    jsonPositionArray.at(1),
                    jsonPositionArray.at(2)));
            }
        }

        _layoutPaused = jsonLayout["paused"];
    }

    if(!u::contains(jsonBody, "pluginData"))
        return false;

    const auto& pluginDataJsonValue = jsonBody["pluginData"];

    QByteArray pluginData;

    if(pluginDataJsonValue.is_object() || pluginDataJsonValue.is_array())
        pluginData = QByteArray::fromStdString(pluginDataJsonValue.dump());
    else if(pluginDataJsonValue.is_string())
        pluginData = QByteArray::fromHex(QByteArray::fromStdString(pluginDataJsonValue));
    else
        return false;

    if(!_pluginInstance->load(pluginData, header._pluginDataVersion, graph, progressFn))
        return false;

    if(u::contains(jsonBody, "ui"))
    {
        const auto& uiDataJsonValue = jsonBody["ui"];

        if(uiDataJsonValue.is_object() || uiDataJsonValue.is_array())
            _uiData = QByteArray::fromStdString(uiDataJsonValue.dump());
        else if(uiDataJsonValue.is_string())
            _uiData = QByteArray::fromHex(QByteArray::fromStdString(uiDataJsonValue));
        else
            return false;

        _pluginDataVersion = header._pluginDataVersion;
    }

    return true;
}

void Loader::setPluginInstance(IPluginInstance* pluginInstance)
{
    _pluginInstance = pluginInstance;
}

const ExactNodePositions* Loader::nodePositions() const
{
    return _nodePositions.get();
}

QString Loader::pluginNameFor(const QUrl& url)
{
    Header header;
    if(parseHeader(url, &header))
        return header._pluginName;

    return "";
}

bool Loader::canOpen(const QUrl& url)
{
    if(QFileInfo(url.toLocalFile()).suffix() != Application::nativeExtension())
        return false;

    Header header;
    auto result = parseHeader(url, &header);

    return result;
}

#include "thirdparty/zlib/zlib_enable_warnings.h"
