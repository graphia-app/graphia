#include "nativeloader.h"
#include "nativesaver.h"

#include "application.h"

#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"
#include "shared/plugins/iplugin.h"
#include "shared/utils/scope_exit.h"
#include "shared/utils/container.h"
#include "shared/loading/progress_iterator.h"
#include "shared/loading/jsongraphparser.h"

#include <QString>
#include <QFileInfo>
#include <QDataStream>

#include <vector>

#include <json_helper.h>

#include <zlib.h>

static bool isCompressed(const QString& filePath)
{
    QFile file(filePath);

    const int GzipHeaderSize = 10;
    if(!file.open(QIODevice::ReadOnly) || file.size() < GzipHeaderSize)
        return false;

    unsigned char header[GzipHeaderSize];

    if(file.read(reinterpret_cast<char*>(header), GzipHeaderSize) != GzipHeaderSize) // NOLINT
        return false;

    // Gzip magic number
    return header[0] == 0x1f && header[1] == 0x8b;
}

static bool decompress(const QString& filePath, QByteArray& byteArray,
                       int maxReadSize = -1, Loader* loader = nullptr)
{
    QFile file(filePath);

    if(!file.open(QIODevice::ReadOnly))
        return false;

    uint64_t totalBytes = file.size();

    if(totalBytes == 0)
        return false;

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
        std::vector<unsigned char> inBuffer(ChunkSize);

        auto numBytes = input.readRawData(reinterpret_cast<char*>(inBuffer.data()), ChunkSize); // NOLINT

        bytesRead += numBytes;

        if(loader != nullptr)
            loader->setProgress((bytesRead * 100) / totalBytes);

        zstream.avail_in = numBytes;
        if(zstream.avail_in == 0)
            break;

        zstream.next_in = static_cast<z_const Bytef*>(inBuffer.data());

        do
        {
            std::vector<unsigned char> outBuffer(ChunkSize);
            zstream.avail_out = ChunkSize;
            zstream.next_out = static_cast<Bytef*>(outBuffer.data());

            ret = inflate(&zstream, Z_NO_FLUSH);
            Q_ASSERT(ret != Z_STREAM_ERROR);

            switch(ret)
            {
            case Z_NEED_DICT:
            case Z_DATA_ERROR:
            case Z_MEM_ERROR:
                return false;
            }

            numBytes = ChunkSize - zstream.avail_out;
            bytesDecompressed += numBytes;
            byteArray.append(reinterpret_cast<const char*>(outBuffer.data()), numBytes); // NOLINT

            // Check if we've read more than we've been asked to
            if(maxReadSize >= 0 && bytesDecompressed >= static_cast<uint64_t>(maxReadSize))
                return true;

        } while(zstream.avail_out == 0);
    } while(ret != Z_STREAM_END);

    return true;
}

static bool load(const QString& filePath, QByteArray& byteArray,
                 int maxReadSize = -1, IGraph* graph = nullptr,
                 Loader* loader = nullptr)
{
    if(isCompressed(filePath))
    {
        if(graph != nullptr)
            graph->setPhase(QObject::tr("Decompressing"));

        return decompress(filePath, byteArray, maxReadSize, loader);
    }

    QFile file(filePath);

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

        if(loader != nullptr)
            loader->setProgress((bytesRead * 100) / totalBytes);

        // Check if we've read more than we've been asked to
        if(maxReadSize >= 0 && bytesRead >= maxReadSize)
            return true;

    } while(!input.atEnd());

    return true;
}

struct Header
{
    int _version = -1;
    QString _pluginName;
    int _pluginDataVersion = -1;
};

static bool parseHeader(const QUrl& url, Header* header = nullptr)
{
    QByteArray byteArray;

    if(!load(url.toLocalFile(), byteArray, NativeSaver::MaxHeaderSize))
        return false;

    // byteArray now has a JSON fragment that hopefully includes the header
    QString fragment(byteArray);

    // Strip off the leading [
    fragment.replace(QRegularExpression(QStringLiteral(R"(^\s*\[\s*)")), QLatin1String(""));

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

bool Loader::parse(const QUrl& url, IGraphModel* graphModel)
{
    Q_ASSERT(graphModel != nullptr);
    if(graphModel == nullptr)
        return false;

    Header header;
    if(!parseHeader(url, &header))
        return false;

    auto version = header._version;

    QByteArray byteArray;

    if(!load(url.toLocalFile(), byteArray, -1, &graphModel->mutableGraph(), this))
        return false;

    setProgress(-1);

    auto jsonArray = parseJsonFrom(byteArray, this);

    if(cancelled())
        return false;

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

    if(!JsonGraphParser::parseGraphObject(jsonGraph, graphModel, *this, true))
        return false;

    setProgress(-1);

    if(u::contains(jsonBody, "nodeNames"))
    {
        NodeId nodeId(0);
        for(const auto& jsonNodeName : jsonBody["nodeNames"])
        {
            if(graphModel->mutableGraph().containsNodeId(nodeId))
                graphModel->setNodeName(nodeId, jsonNodeName);

            ++nodeId;
        }
    }

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

    if(u::contains(jsonBody, "bookmarks"))
    {
        const auto bookmarks = jsonBody["bookmarks"];
        for(auto bookmarkIt = bookmarks.begin(); bookmarkIt != bookmarks.end(); ++bookmarkIt)
        {
            QString name = QString::fromStdString(bookmarkIt.key());
            const auto& array = bookmarkIt.value();

            if(array.is_array())
            {
                NodeIdSet nodeIds;
                nodeIds.reserve(array.size());

                for(const auto& nodeId : array)
                    nodeIds.insert(static_cast<int>(nodeId));

                _bookmarks.insert({name, nodeIds});
            }
        }
    }

    if(u::contains(jsonBody, "enrichmentTables"))
    {
        for(const auto& tableModel : jsonBody["enrichmentTables"])
        {
            _enrichmentTablesData.emplace_back();
            auto& table = _enrichmentTablesData.back();
            // If Data is empty then it's just an empty table
            if(u::contains(tableModel, "data"))
            {
                for(auto& dataRow : tableModel["data"])
                {
                    table.emplace_back();
                    auto& row = table.back();
                    row.reserve(dataRow.size());
                    for(auto& value : dataRow)
                    {
                        if(value.is_number())
                            row.emplace_back(value.get<std::double_t>());
                        else
                            row.emplace_back(QString::fromStdString(value.get<std::string>()));
                    }
                }
            }
        }
    }

    if(u::contains(jsonBody, "layout"))
    {
        const auto& jsonLayout = jsonBody["layout"];

        if(u::contains(jsonLayout, "algorithm"))
            _layoutName = QString::fromStdString(jsonLayout["algorithm"]);

        if(u::contains(jsonLayout, "settings"))
        {
            const auto settings = jsonLayout["settings"];
            for(auto settingsIt = settings.begin(); settingsIt != settings.end(); ++settingsIt)
            {
                QString name = QString::fromStdString(settingsIt.key());
                const auto& value = settingsIt.value();

                if(value.is_number())
                    _layoutSettings.push_back({name, value});
            }
        }

        if(u::contains(jsonLayout, "positions"))
        {
            _nodePositions = std::make_unique<ExactNodePositions>(graphModel->mutableGraph());

            NodeId nodeId(0);
            for(const auto& jsonPosition : jsonLayout["positions"])
            {
                if(graphModel->mutableGraph().containsNodeId(nodeId))
                {
                    const auto& jsonPositionArray = jsonPosition;

                    _nodePositions->set(nodeId, QVector3D(
                        jsonPositionArray.at(0),
                        jsonPositionArray.at(1),
                        jsonPositionArray.at(2)));
                }

                ++nodeId;
            }
        }

        _layoutPaused = jsonLayout["paused"];
    }

    if(version >= 2 && u::contains(jsonBody, "ui"))
    {
        const auto& jsonUiDataJsonValue = jsonBody["ui"];

        if(jsonUiDataJsonValue.is_object() || jsonUiDataJsonValue.is_array())
            _uiData = QByteArray::fromStdString(jsonUiDataJsonValue.dump());
        else
            return false;
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

    if(!_pluginInstance->load(pluginData, header._pluginDataVersion, graphModel->mutableGraph(), *this))
        return false;

    const auto pluginUiDataKey = version >= 2 ? "pluginUiData" : "ui";
    if(u::contains(jsonBody, pluginUiDataKey))
    {
        const auto& pluginUiDataJsonValue = jsonBody[pluginUiDataKey];

        if(pluginUiDataJsonValue.is_object() || pluginUiDataJsonValue.is_array())
            _pluginUiData = QByteArray::fromStdString(pluginUiDataJsonValue.dump());
        else if(pluginUiDataJsonValue.is_string())
            _pluginUiData = QByteArray::fromHex(QByteArray::fromStdString(pluginUiDataJsonValue));
        else
            return false;

        _pluginUiDataVersion = header._pluginDataVersion;
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

    return QLatin1String("");
}

bool Loader::canOpen(const QUrl& url)
{
    Header header;
    auto result = parseHeader(url, &header);

    return result;
}
