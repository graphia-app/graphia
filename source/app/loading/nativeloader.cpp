/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#include "nativeloader.h"
#include "nativesaver.h"

#include "application.h"

#include "shared/graph/igraphmodel.h"
#include "shared/graph/imutablegraph.h"
#include "shared/graph/grapharray_json.h"

#include "shared/plugins/iplugin.h"

#include "shared/utils/scope_exit.h"
#include "shared/utils/container.h"

#include "shared/loading/userelementdata.h"
#include "shared/loading/progress_iterator.h"
#include "shared/loading/jsongraphparser.h"

#include <QString>
#include <QDataStream>
#include <QRegularExpression>

#include <vector>
#include <thread>
#include <chrono>

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
            loader->setProgress(static_cast<int>((bytesRead * 100u) / totalBytes));

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

            numBytes = ChunkSize - static_cast<int>(zstream.avail_out);
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

    decltype(totalBytes) bytesRead = 0;
    QDataStream input(&file);

    do
    {
        const int ChunkSize = 2 << 16;
        std::vector<unsigned char> buffer(ChunkSize);

        auto numBytes = input.readRawData(reinterpret_cast<char*>(buffer.data()), ChunkSize);
        byteArray.append(reinterpret_cast<char*>(buffer.data()), numBytes);

        bytesRead += numBytes;

        if(loader != nullptr)
            loader->setProgress(static_cast<int>((bytesRead * 100u) / totalBytes));

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
    if(!url.isLocalFile())
        return false;

    QByteArray byteArray;

    if(!load(url.toLocalFile(), byteArray, NativeSaver::MaxHeaderSize))
        return false;

    // byteArray now has a JSON fragment that hopefully includes the header
    QString fragment(byteArray);

    // Strip off the leading [
    static const QRegularExpression re(QStringLiteral(R"(^\s*\[\s*)"));
    fragment.replace(re, {});

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

    if(jsonHeader.is_discarded() || jsonHeader.is_null() || !jsonHeader.is_object())
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

bool Loader::parse(const QUrl& url, IGraphModel* igraphModel)
{
    auto* graphModel = dynamic_cast<GraphModel*>(igraphModel);

    Q_ASSERT(graphModel != nullptr);
    if(graphModel == nullptr)
        return false;

    Header header;
    if(!parseHeader(url, &header))
        return false;

    auto version = header._version;

    if(version > NativeSaver::Version)
    {
        setFailureReason(QObject::tr("It was saved using a newer version of %1. "
            "Please visit the %1 website to upgrade.")
            .arg(Application::name()));
        return false;
    }

    QByteArray byteArray;

    if(!load(url.toLocalFile(), byteArray, -1, &graphModel->mutableGraph(), this))
        return false;

    setProgress(-1);

    graphModel->mutableGraph().setPhase(QObject::tr("Parsing"));

    auto jsonArray = parseJsonFrom(byteArray, this);

    if(cancelled())
        return false;

    if(jsonArray.is_null() || !jsonArray.is_array())
        return false;

    if(jsonArray.size() != 2)
        return false;

    auto allObjects = std::all_of(jsonArray.begin(), jsonArray.end(),
    [](const auto& i)
    {
       return i.is_object();
    });

    if(!allObjects)
        return false;

    auto jsonBody = jsonArray.at(1);

    if(u::contains(jsonBody, "infiniteParse"))
    {
        while(!cancelled())
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(1000ms);
        }

        return false;
    }

    if(!u::contains(jsonBody, "graph") || !jsonBody["graph"].is_object())
        return false;

    const auto& jsonGraph = jsonBody["graph"];

    if(!JsonGraphParser::parseGraphObject(jsonGraph, graphModel, *this, true))
        return false;

    setProgress(-1);

    if(u::contains(jsonBody, "nodeNames"))
    {
        if(version >= 4)
        {
            u::forEachJsonGraphArray(jsonBody["nodeNames"], [graphModel](NodeId nodeId, const QString& nodeName)
            {
                Q_ASSERT(graphModel->mutableGraph().containsNodeId(nodeId));
                graphModel->setNodeName(nodeId, nodeName);
            });
        }
        else
        {
            NodeId nodeId(0);
            for(const auto& jsonNodeName : jsonBody["nodeNames"])
            {
                if(graphModel->mutableGraph().containsNodeId(nodeId))
                    graphModel->setNodeName(nodeId, jsonNodeName);

                ++nodeId;
            }
        }
    }

    if(version < 6)
    {
        // Older files store the user attribute data in the plugin section
        const auto& pluginData = jsonBody["pluginData"];

        if(pluginData.is_object())
        {
            if(u::contains(pluginData, "userNodeData") && pluginData["userNodeData"].is_object())
            {
                if(!graphModel->userNodeData().load(pluginData["userNodeData"], *this))
                    return false;
            }

            if(u::contains(pluginData, "userEdgeData") && pluginData["userEdgeData"].is_object())
            {
                if(!graphModel->userEdgeData().load(pluginData["userEdgeData"], *this))
                    return false;
            }
        }
    }

    if(u::contains(jsonBody, "userNodeData") && jsonBody["userNodeData"].is_object())
    {
        if(!graphModel->userNodeData().load(jsonBody["userNodeData"], *this))
            return false;
    }

    if(u::contains(jsonBody, "userEdgeData") && jsonBody["userEdgeData"].is_object())
    {
        if(!graphModel->userEdgeData().load(jsonBody["userEdgeData"], *this))
            return false;
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

    if(u::contains(jsonBody, "projection"))
        _projection = jsonBody["projection"];

    if(u::contains(jsonBody, "2dshading"))
        _shading = jsonBody["2dshading"];

    if(u::contains(jsonBody, "3dshading"))
        _shading = jsonBody["3dshading"];

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
                    nodeIds.insert(nodeId.get<int>());

                _bookmarks.insert({name, nodeIds});
            }
        }
    }

    if(u::contains(jsonBody, "log"))
        _log = QString::fromStdString(jsonBody["log"]);

    if(u::contains(jsonBody, "enrichmentTables"))
    {
        for(const auto& tableModel : jsonBody["enrichmentTables"])
        {
            auto& data = _enrichmentTableData.emplace_back();

            // If Data is empty then it's just an empty table
            if(u::contains(tableModel, "data"))
            {
                for(const auto& dataRow : tableModel["data"])
                {
                    if(dataRow.is_null())
                    {
                        qDebug() << "null dataRow in enrichment table";
                        continue;
                    }

                    auto& row = data._table.emplace_back();
                    row.reserve(dataRow.size());

                    for(const auto& value : dataRow)
                    {
                        if(value.is_number())
                            row.emplace_back(value.get<std::double_t>());
                        else
                            row.emplace_back(QString::fromStdString(value.get<std::string>()));
                    }
                }
            }

            if(u::contains(tableModel, "selectionA"))
                data._selectionA = QString::fromStdString(tableModel["selectionA"]);

            if(u::contains(tableModel, "selectionB"))
                data._selectionB = QString::fromStdString(tableModel["selectionB"]);
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

            if(version >= 4)
            {
                u::forEachJsonGraphArray(jsonLayout["positions"], [this, graphModel](NodeId nodeId, const json& position)
                {
                    Q_ASSERT(graphModel->mutableGraph().containsNodeId(nodeId));

                    _nodePositions->set(nodeId, QVector3D(
                        position.at(0),
                        position.at(1),
                        position.at(2)));
                });
            }
            else
            {
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
        }

        _layoutPaused = jsonLayout["paused"];
    }

    if(u::contains(jsonBody, "nodeSize"))
        graphModel->setNodeSize(jsonBody["nodeSize"]);

    if(u::contains(jsonBody, "edgeSize"))
        graphModel->setEdgeSize(jsonBody["edgeSize"]);

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

    if(header._pluginDataVersion > _pluginInstance->plugin()->dataVersion())
    {
        setFailureReason(QObject::tr("Produced using a newer version of the plugin '%1'. Please upgrade.")
            .arg(_pluginInstance->plugin()->name()));
        return false;
    }

    if(!_pluginInstance->load(pluginData, header._pluginDataVersion, graphModel->mutableGraph(), *this))
    {
        setFailureReason(_pluginInstance->failureReason());
        return false;
    }

    const auto* pluginUiDataKey = version >= 2 ? "pluginUiData" : "ui";
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

    return {};
}

bool Loader::canOpen(const QUrl& url)
{
    if(!url.isLocalFile())
        return false;

    Header header;
    auto result = parseHeader(url, &header);

    return result;
}
