#include "saver.h"

#include "shared/plugins/iplugin.h"
#include "shared/utils/scope_exit.h"

#include "graph/graphmodel.h"

#include <QFile>
#include <QDataStream>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

#include "thirdparty/zlib/zlib_disable_warnings.h"
#include "thirdparty/zlib/zlib.h"

static bool compress(const QByteArray& byteArray, const QString& filePath, const ProgressFn& progressFn)
{
    QFile file(filePath);

    if(!file.open(QIODevice::WriteOnly))
        return false;

    auto totalBytes = byteArray.size();
    int bytePosition = 0;
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

bool Saver::encode(const ProgressFn& progressFn)
{
    QJsonArray jsonArray;

    QJsonObject header;
    header["version"] = 1;
    header["pluginName"] = _graphModel->pluginName();
    header["pluginDataVersion"] = _graphModel->pluginDataVersion();
    jsonArray.append(header);

    // The header must fit within a certain size, which is the maximum the loader will look at
    if(QJsonDocument(QJsonArray({header})).toJson().size() > MaxHeaderSize)
        return false;

    QJsonObject content;
    //FIXME Save real data

    _graphModel->mutableGraph().setPhase(_graphModel->pluginName());
    auto pluginData = _pluginInstance->save(_graphModel->mutableGraph(), progressFn);

    progressFn(-1);

    auto pluginJson = QJsonDocument::fromJson(pluginData);

    // If the plugin data is itself JSON, just whack it in
    // as is, but if it's not, hex encode it
    if(!pluginJson.isNull() && pluginJson.isObject())
        content["pluginData"] = pluginJson.object();
    else
        content["pluginData"] = QString(pluginData.toHex());

    jsonArray.append(content);

    _graphModel->mutableGraph().setPhase(QObject::tr("Compressing"));
    return compress(QJsonDocument(jsonArray).toJson(), _fileUrl.path(), progressFn);
}

#include "thirdparty/zlib/zlib_enable_warnings.h"
