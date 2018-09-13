#ifndef JSONGRAPHEXPORTER_H
#define JSONGRAPHEXPORTER_H

#include "isaver.h"

#include <QString>

class JSONGraphSaver : public ISaver
{
private:
    const QUrl& _url;
    IGraphModel* _graphModel;
public:
    JSONGraphSaver(const QUrl& url, IGraphModel* graphModel) : _url(url), _graphModel(graphModel) {}
    bool save() override;
};

class JSONGraphSaverFactory : public ISaverFactory
{
public:
    QString name() const override { return QStringLiteral("JSON Graph"); }
    QString extension() const override { return QStringLiteral(".json"); }
    std::unique_ptr<ISaver> create(const QUrl &url, Document *document,
                                   const IPluginInstance *pluginInstance,
                                   const QByteArray &uiData, const QByteArray &pluginUiData) override;
};


#endif // JSONGRAPHEXPORTER_H
