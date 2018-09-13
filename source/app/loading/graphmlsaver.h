#ifndef GRAPHMLEXPORTER_H
#define GRAPHMLEXPORTER_H

#include "loading/isaver.h"

#include <QString>
#include <QUrl>

class GraphMLSaverFactory : public ISaverFactory
{
    QString name() const override { return QStringLiteral("GraphML"); }
    QString extension() const override { return QStringLiteral(".graphml"); }
    std::unique_ptr<ISaver> create(const QUrl& url, Document* document,
                                   const IPluginInstance*,
                                   const QByteArray&, const QByteArray&) override;
};

class GraphMLSaver : public ISaver
{
private:
    const QUrl& _url;
    IGraphModel* _graphModel;
public:
    GraphMLSaver(const QUrl& url, IGraphModel* graphModel) : _url(url), _graphModel(graphModel) {}
    bool save() override;
};

#endif // GRAPHMLEXPORTER_H
