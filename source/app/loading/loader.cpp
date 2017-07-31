#include "loader.h"
#include "saver.h"

#include "application.h"

#include "shared/plugins/iplugin.h"
#include "shared/utils/scope_exit.h"

#include <QString>
#include <QFileInfo>
#include <QDataStream>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

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

    auto totalBytes = file.size();
    int bytesRead = 0;
    int bytesDecompressed = 0;
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
            if(maxReadSize >= 0 && bytesDecompressed >= maxReadSize)
                return true;

        } while (zstream.avail_out == 0);
    } while (ret != Z_STREAM_END);

    return true;
}

static bool load(const QString& filePath, QByteArray& byteArray,
                 int maxReadSize = -1, ProgressFn progressFn = [](int){})
{
    if(isCompressed(filePath))
        return decompress(filePath, byteArray, maxReadSize, progressFn);

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

    if(!load(url.path(), byteArray, Saver::MaxHeaderSize))
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
    QJsonDocument jsonHeader = QJsonDocument::fromJson(headerString.toUtf8());

    if(jsonHeader.isNull() || !jsonHeader.isObject())
        return false;

    auto jsonHeaderObject = jsonHeader.object();

    if(!jsonHeaderObject.contains("version"))
        return false;

    if(!jsonHeaderObject.contains("pluginName"))
        return false;

    if(!jsonHeaderObject.contains("pluginDataVersion"))
        return false;

    if(header != nullptr)
    {
        header->_version            = jsonHeaderObject["version"].toInt();
        header->_pluginName         = jsonHeaderObject["pluginName"].toString();
        header->_pluginDataVersion  = jsonHeaderObject["pluginDataVersion"].toInt();
    }

    return true;
}

bool Loader::parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progressFn)
{
    Header header;
    if(!parseHeader(url, &header))
        return false;

    QByteArray byteArray;

    if(!load(url.path(), byteArray, -1, progressFn))
        return false;

    progressFn(-1);

    auto jsonDocument = QJsonDocument::fromJson(byteArray);

    if(jsonDocument.isNull() || !jsonDocument.isArray())
        return false;

    auto jsonArray = jsonDocument.array();

    if(jsonArray.size() != 2)
        return false;

    for(const auto& i : jsonArray)
    {
        if(!i.isObject())
            return false;
    }

    auto jsonBody = jsonArray.at(1).toObject();

    if(!jsonBody.contains("pluginData"))
        return false;

    auto pluginData = jsonBody["pluginData"].toString().toUtf8();

    if(QJsonDocument::fromJson(pluginData).isNull())
    {
        // If it's not JSON, it'll be hex encoded, so decode first
        auto decoded = QByteArray::fromHex(pluginData);
        _pluginInstance->load(decoded, graph, progressFn);
    }
    else
        _pluginInstance->load(pluginData, graph, progressFn);

    return false;
}

void Loader::setPluginInstance(IPluginInstance* pluginInstance)
{
    _pluginInstance = pluginInstance;
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
