#ifndef SAVER_H
#define SAVER_H

#include "shared/loading/progressfn.h"

#include <QUrl>
#include <QStringList>
#include <QString>
#include <QByteArray>

class Document;
class IPluginInstance;

class Saver
{
private:
    QUrl _fileUrl;

    Document* _document = nullptr;
    const IPluginInstance* _pluginInstance = nullptr;
    QByteArray _pluginUIData;

public:
    static const int MaxHeaderSize = 1 << 12;

    explicit Saver(const QUrl& fileUrl) { _fileUrl = fileUrl; }

    QUrl fileUrl() const { return _fileUrl; }

    void setDocument(Document* document) { _document = document; }
    void setPluginInstance(const IPluginInstance* pluginInstance) { _pluginInstance = pluginInstance; }
    void setPluginUIData(const QByteArray& pluginUIData) { _pluginUIData = pluginUIData; }

    bool encode(const ProgressFn& progressFn);
};

#endif // SAVER_H
