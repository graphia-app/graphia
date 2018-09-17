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
    static QString name() { return QStringLiteral("GraphML"); }
    static QString extension() { return QStringLiteral("graphml"); }
    GraphMLSaver(const QUrl& url, IGraphModel* graphModel) : _url(url), _graphModel(graphModel) {}
    bool save() override;
};

using GraphMLSaverFactory = SaverFactory<GraphMLSaver>;

#endif // GRAPHMLEXPORTER_H
