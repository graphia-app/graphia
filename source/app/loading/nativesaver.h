#ifndef SAVER_H
#define SAVER_H

#include <utility>

#include "isaver.h"
#include "shared/utils/progressable.h"

#include "graph/graphmodel.h"
#include "graph/mutablegraph.h"
#include "ui/document.h"

#include <json_helper.h>

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QUrl>

class Document;
class IGraph;
class IPluginInstance;

class NativeSaver : public ISaver
{
private:
    QUrl _fileUrl;
    Document* _document = nullptr;
    const IPluginInstance* _pluginInstance = nullptr;
    QByteArray _uiData;
    QByteArray _pluginUiData;

public:
    static const int Version;
    static const int MaxHeaderSize;

    NativeSaver(QUrl fileUrl, Document* document, const IPluginInstance* pluginInstance, QByteArray uiData,
                QByteArray pluginUiData) :
        _fileUrl(std::move(fileUrl)),
        _document(document), _pluginInstance(pluginInstance), _uiData(std::move(uiData)),
        _pluginUiData(std::move(pluginUiData))
    {}

    bool save() override;
};

class NativeSaverFactory : public ISaverFactory
{
public:
    QString name() const override { return Application::name(); }
    QString extension() const override { return Application::nativeExtension(); }
    std::unique_ptr<ISaver> create(const QUrl& url, Document* document, const IPluginInstance* pluginInstance,
                                   const QByteArray& uiData, const QByteArray& pluginUiData) override;
};

#endif // SAVER_H
