#ifndef GRAPHMLEXPORTER_H
#define GRAPHMLEXPORTER_H

#include "loading/saverfactory.h"

#include <QString>
#include <QUrl>

class GraphMLSaver : public ISaver
{
private:
    const QUrl& _url;
    IGraphModel* _graphModel;
public:
    GraphMLSaver(const QUrl& url, IGraphModel* graphModel) : _url(url), _graphModel(graphModel) {}
    bool save() override;
};

class GraphMLSaverFactory : public SaverFactory<GraphMLSaver>
{
    QString name() const override { return QStringLiteral("GraphML"); }
    QString extension() const override { return QStringLiteral(".graphml"); }
};

#endif // GRAPHMLEXPORTER_H
