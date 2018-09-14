#ifndef SAVER_H
#define SAVER_H

#include <utility>

#include "isaver.h"
#include "json_helper.h"
#include "shared/utils/progressable.h"

#include "graph/graphmodel.h"
#include "graph/mutablegraph.h"
#include "ui/document.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QUrl>

class Document;
class IGraph;
class IPluginInstance;

class Saver : public ISaver
{
private:
    QUrl _fileUrl;
    Document* _document = nullptr;
    const IPluginInstance* _pluginInstance = nullptr;
    QByteArray _uiData;
    QByteArray _pluginUiData;

public:
    static const int MaxHeaderSize = 1 << 12;
    static json graphAsJson(const IGraph& graph, Progressable& progressable);

    Saver(QUrl fileUrl, Document* document, const IPluginInstance* pluginInstance, const QByteArray& uiData,
          const QByteArray& pluginUiData) :
        _fileUrl(std::move(fileUrl)),
        _document(document), _pluginInstance(pluginInstance), _uiData(uiData), _pluginUiData(pluginUiData)
    {}

    bool save() override;
};

class SaverFactory : public ISaverFactory
{
public:
    QString name() const override { return Application::name(); }
    QString extension() const override { return QStringLiteral(".%1").arg(Application::nativeExtension()); }
    std::unique_ptr<ISaver> create(const QUrl& url, Document* document, const IPluginInstance* pluginInstance,
                                   const QByteArray& uiData, const QByteArray& pluginUiData) override;
};

#endif // SAVER_H
