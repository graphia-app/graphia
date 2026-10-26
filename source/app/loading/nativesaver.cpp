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

#include "nativesaver.h"

#include "jsongraphsaver.h"

#include "shared/graph/grapharray_json.h"

#include "shared/plugins/iplugin.h"
#include "shared/utils/scope_exit.h"
#include "shared/utils/string.h"
#include "shared/loading/userelementdata.h"

#include "app/attributes/enrichmenttablemodel.h"
#include "app/graph/graphmodel.h"
#include "app/graph/mutablegraph.h"
#include "app/ui/document.h"

#include <zlib.h>

#include <QDataStream>
#include <QFile>
#include <QMetaType>
#include <QObject>
#include <QtGlobal>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <utility>
#include <vector>

const int NativeSaver::Version = 9;
const int NativeSaver::MaxHeaderSize = 1 << 12;

static bool compress(const QByteArray& byteArray, const QString& filePath, Progressable& progressable)
{
    QFile file(filePath);

    if(!file.open(QIODevice::WriteOnly))
        return false;

    auto totalBytes = static_cast<uint64_t>(byteArray.size());
    uint64_t bytePosition = 0;
    QDataStream input(byteArray);
    QDataStream output(&file);

    z_stream zstream = {};
    auto ret = deflateInit2(&zstream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                            MAX_WBITS + 16, // 16 means write gzip header/trailer
                            8, Z_DEFAULT_STRATEGY);
    if(ret != Z_OK)
        return false;

    auto atExit = std::experimental::make_scope_exit([&zstream] { deflateEnd(&zstream); });
    Q_UNUSED(atExit);

    int flush = Z_ERRNO;

    do
    {
        const int ChunkSize = 1 << 14;
        std::vector<unsigned char> inBuffer(ChunkSize);

        auto numBytes = input.readRawData(reinterpret_cast<char*>(inBuffer.data()), ChunkSize);

        bytePosition += static_cast<uint64_t>(numBytes);
        progressable.setProgress(static_cast<int>((bytePosition * 100u) / totalBytes));

        zstream.avail_in = static_cast<uInt>(numBytes);
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

            numBytes = ChunkSize - static_cast<int>(zstream.avail_out);
            if(output.writeRawData(reinterpret_cast<const char*>(outBuffer.data()), numBytes) !=
               numBytes)
                return false;

        } while(zstream.avail_out == 0);
        Q_ASSERT(zstream.avail_in == 0);

    } while(flush != Z_FINISH);
    Q_ASSERT(ret == Z_STREAM_END);

    return true;
}

static json bookmarksAsJson(const std::map<QString, NodeIdSet>& bookmarks)
{
    json jsonObject = json::object();

    for(const auto& [bookmark, bookmarkedNodeIds] : bookmarks)
    {
        json nodeIds;
        std::ranges::copy(bookmarkedNodeIds, std::back_inserter(nodeIds));

        auto byteArray = bookmark.toUtf8();
        const auto* bookmarkName = byteArray.constData();
        jsonObject[bookmarkName] = nodeIds;
    }

    return jsonObject;
}

static json layoutSettingsAsJson(const std::vector<LayoutSettingKeyValue>& settings)
{
    json jsonObject;

    for(const auto& setting : settings)
    {
        auto byteArray = setting._name.toUtf8();
        const auto* settingName = byteArray.constData();
        jsonObject[settingName] = setting._value;
    }

    return jsonObject;
}

static json enrichmentTableModelAsJson(const EnrichmentTableModel& table)
{
    json jsonObject;

    const auto& tableData = table.tableData();
    for(size_t row = 0; row < tableData.size(); row++)
    {
        for(const auto& v : tableData.at(row))
        {
            if(v.typeId() == QMetaType::QString)
                jsonObject["data"][row].push_back(v.toString().toStdString());
            else if(v.typeId() == QMetaType::Double || v.typeId() == QMetaType::Int)
                jsonObject["data"][row].push_back(v.toDouble());
        }
    }

    jsonObject["selectionA"] = table.selectionA();
    jsonObject["selectionB"] = table.selectionB();

    return jsonObject;
}

bool NativeSaver::save()
{
    json jsonArray;

    auto* graphModel = _graphModel;

    Q_ASSERT(graphModel != nullptr);
    if(graphModel == nullptr)
        return false;

    auto& graph = graphModel->mutableGraph();

    json header;
    header["version"] = NativeSaver::Version;
    header["pluginName"] = graphModel->pluginName();
    header["pluginDataVersion"] = graphModel->pluginDataVersion();
    header["appVersion"] = VERSION;
    jsonArray.emplace_back(header);

    // The header must fit within a certain size, which is the maximum the loader will look at
    if(json({header}).dump().size() > MaxHeaderSize)
        return false;

    json content;

    content["graph"] = JSONGraphSaver::graphAsJson(graph, *this);
    content["nodeNames"] = u::graphArrayAsJson(graphModel->nodeNames(), graph.nodeIds(), this);

    content["userNodeData"] = graphModel->userNodeData().save(graph, graph.nodeIds(), *this);
    content["userEdgeData"] = graphModel->userEdgeData().save(graph, graph.edgeIds(), *this);

    json layout;

    layout["algorithm"] = _content._layoutName;
    layout["settings"] = layoutSettingsAsJson(_content._layoutSettings);

    layout["positions"] = u::graphArrayAsJson(graphModel->nodePositions(), graph.nodeIds(), this,
    [](const auto& v)
    {
        return json({v.x(), v.y(), v.z()});
    });

    layout["paused"] = _content._layoutPaused;
    content["layout"] = layout;

    content["nodeSize"] = graphModel->nodeSize();
    content["edgeSize"] = graphModel->edgeSize();
    content["textSize"] = graphModel->textSize();

    content["projection"] = _content._projection;
    content["2dshading"] = _content._shading2D;
    content["3dshading"] = _content._shading3D;

    content["transforms"] = u::toQStringVector(_content._transforms);
    content["visualisations"] = u::toQStringVector(_content._visualisations);

    content["bookmarks"] = bookmarksAsJson(_content._bookmarks);

    content["log"] = _content._log;

    for(const auto* table : _content._enrichmentTableModels)
        content["enrichmentTables"].push_back(enrichmentTableModelAsJson(*table));

    auto uiDataJson = json::parse(_uiData.cbegin(), _uiData.cend(), nullptr, false);

    if(uiDataJson.is_object() || uiDataJson.is_array())
        content["ui"] = uiDataJson;

    setPhase(graphModel->pluginDisplayName());
    auto pluginData = _pluginInstance->save(graph, *this);

    setProgress(-1);

    auto pluginDataJson = json::parse(pluginData.begin(), pluginData.end(), nullptr, false);

    // If the plugin data is itself JSON, just whack it in
    // as is, but if it's not, hex encode it
    if(!pluginDataJson.is_discarded() && (pluginDataJson.is_object() || pluginDataJson.is_array()))
        content["pluginData"] = pluginDataJson;
    else
        content["pluginData"] = QString(pluginData.toHex());

    auto pluginUiDataJson = json::parse(_pluginUiData.cbegin(), _pluginUiData.cend(), nullptr, false);

    if(!pluginUiDataJson.is_discarded() && (pluginUiDataJson.is_object() || pluginUiDataJson.is_array()))
        content["pluginUiData"] = pluginUiDataJson;
    else
        content["pluginUiData"] = QString(_pluginUiData.toHex());

    jsonArray.emplace_back(content);

    setPhase(QObject::tr("Compressing"));
    return compress(QByteArray::fromStdString(jsonArray.dump()), _fileUrl.toLocalFile(), *this);
}

std::unique_ptr<ISaver> NativeSaverFactory::create(const QUrl& url, Document* document,
                                             const IPluginInstance* pluginInstance, const QByteArray& uiData,
                                             const QByteArray& pluginUiData)
{
    NativeSaver::Content content;

    content._layoutName = document->layoutName();

    for(const auto& setting : document->layoutSettings())
        content._layoutSettings.push_back({setting.name(), setting.value()});

    content._layoutPaused = document->layoutPauseState() == LayoutPauseState::Paused;

    content._projection = document->projection();
    content._shading2D = document->shading2D();
    content._shading3D = document->shading3D();

    content._transforms = document->transforms();
    content._visualisations = document->visualisations();

    const auto bookmarks = document->bookmarks();
    for(const auto& bookmark : bookmarks)
        content._bookmarks.emplace(bookmark, document->nodeIdsForBookmark(bookmark));

    content._log = document->log();

    for(const auto& variant : document->enrichmentTableModels())
        content._enrichmentTableModels.push_back(variant.value<EnrichmentTableModel*>());

    return std::make_unique<NativeSaver>(url, dynamic_cast<GraphModel*>(document->graphModel()),
        std::move(content), pluginInstance, uiData, pluginUiData);
}
