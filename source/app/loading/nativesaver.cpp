/* Copyright © 2013-2021 Graphia Technologies Ltd.
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
#include "shared/utils/iterator_range.h"
#include "shared/utils/scope_exit.h"
#include "shared/utils/string.h"
#include "shared/loading/userelementdata.h"

#include "graph/graphmodel.h"
#include "graph/mutablegraph.h"

#include "ui/document.h"

#include <QDataStream>
#include <QFile>
#include <QStringList>

#include <vector>

#include <zlib.h>

const int NativeSaver::Version = 6;
const int NativeSaver::MaxHeaderSize = 1 << 12;

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

    auto atExit = std::experimental::make_scope_exit([&zstream] { deflateEnd(&zstream); });
    Q_UNUSED(atExit);

    int flush = Z_ERRNO;

    do
    {
        const int ChunkSize = 1 << 14;
        std::vector<unsigned char> inBuffer(ChunkSize);

        auto numBytes = input.readRawData(reinterpret_cast<char*>(inBuffer.data()), ChunkSize); // NOLINT

        bytePosition += numBytes;
        progressable.setProgress(static_cast<int>((bytePosition * 100u) / totalBytes));

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

            numBytes = ChunkSize - static_cast<int>(zstream.avail_out);
            if(output.writeRawData(reinterpret_cast<const char*>(outBuffer.data()), numBytes) !=
               numBytes) // NOLINT
                return false;

        } while(zstream.avail_out == 0);
        Q_ASSERT(zstream.avail_in == 0);

    } while(flush != Z_FINISH);
    Q_ASSERT(ret == Z_STREAM_END);

    return true;
}

static json bookmarksAsJson(const Document& document)
{
    json jsonObject = json::object();

    auto bookmarks = document.bookmarks();
    for(const auto& bookmark : bookmarks)
    {
        json nodeIds;

        auto bookmarkedNodeIds = document.nodeIdsForBookmark(bookmark);
        std::copy(bookmarkedNodeIds.begin(), bookmarkedNodeIds.end(),
            std::back_inserter(nodeIds));

        auto byteArray = bookmark.toUtf8();
        const auto* bookmarkName = byteArray.constData();
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
        const auto* settingName = byteArray.constData();
        jsonObject[settingName] = setting.value();
    }

    return jsonObject;
}

static json enrichmentTableModelAsJson(const EnrichmentTableModel& table)
{
    json jsonObject;

    for(int row = 0; row < table.rowCount(); row++)
    {
        for(int column = 0; column < table.columnCount(); column++)
        {
            const auto& v = table.data(row, static_cast<EnrichmentTableModel::Results>(column));

            if(v.type() == QVariant::String)
                jsonObject["data"][row].push_back(v.toString().toStdString());
            else if(v.type() == QVariant::Double || v.type() == QVariant::Int)
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

    auto* graphModel = dynamic_cast<GraphModel*>(_document->graphModel());

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

    layout["algorithm"] = _document->layoutName();
    layout["settings"] = layoutSettingsAsJson(*_document);

    layout["positions"] = u::graphArrayAsJson(graphModel->nodePositions(), graph.nodeIds(), this,
    [](const auto& v)
    {
        return json({v.x(), v.y(), v.z()});
    });

    layout["paused"] = _document->layoutPauseState() == LayoutPauseState::Paused;
    content["layout"] = layout;

    content["nodeSize"] = _document->nodeSize();
    content["edgeSize"] = _document->edgeSize();

    content["projection"] = _document->projection();
    content["2dshading"] = _document->shading2D();
    content["3dshading"] = _document->shading3D();

    content["transforms"] = u::toQStringVector(_document->transforms());
    content["visualisations"] = u::toQStringVector(_document->visualisations());

    content["bookmarks"] = bookmarksAsJson(*_document);

    content["log"] = _document->log();

    for(const auto& variant : _document->enrichmentTableModels())
    {
        auto* table = variant.value<EnrichmentTableModel*>();
        content["enrichmentTables"].push_back(enrichmentTableModelAsJson(*table));
    }

    auto uiDataJson = json::parse(_uiData.begin(), _uiData.end(), nullptr, false);

    if(uiDataJson.is_object() || uiDataJson.is_array())
        content["ui"] = uiDataJson;

    graph.setPhase(graphModel->pluginName());
    auto pluginData = _pluginInstance->save(graph, *this);

    setProgress(-1);

    auto pluginDataJson = json::parse(pluginData.begin(), pluginData.end(), nullptr, false);

    // If the plugin data is itself JSON, just whack it in
    // as is, but if it's not, hex encode it
    if(!pluginDataJson.is_discarded() && (pluginDataJson.is_object() || pluginDataJson.is_array()))
        content["pluginData"] = pluginDataJson;
    else
        content["pluginData"] = QString(pluginData.toHex());

    auto pluginUiDataJson = json::parse(_pluginUiData.begin(), _pluginUiData.end(), nullptr, false);

    if(!pluginUiDataJson.is_discarded() && (pluginUiDataJson.is_object() || pluginUiDataJson.is_array()))
        content["pluginUiData"] = pluginUiDataJson;
    else
        content["pluginUiData"] = QString(_pluginUiData.toHex());

    jsonArray.emplace_back(content);

    graph.setPhase(QObject::tr("Compressing"));
    return compress(QByteArray::fromStdString(jsonArray.dump()), _fileUrl.toLocalFile(), *this);
}

std::unique_ptr<ISaver> NativeSaverFactory::create(const QUrl& url, Document* document,
                                             const IPluginInstance* pluginInstance, const QByteArray& uiData,
                                             const QByteArray& pluginUiData)
{
    return std::make_unique<NativeSaver>(url, document, pluginInstance, uiData, pluginUiData);
}
