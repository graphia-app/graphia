#ifndef GMLEXPORTER_H
#define GMLEXPORTER_H

#include "graph/graphmodel.h"
#include "loading/saverfactory.h"

class GMLSaver : public ISaver
{
private:
    const QUrl& _url;
    IGraphModel* _graphModel;
    static QString indent(int level) { return QStringLiteral("    ").repeated(level); }

public:
    static QString name() { return QStringLiteral("GML"); }
    static QString extension() { return QStringLiteral("gml"); }
    GMLSaver(const QUrl& url, IGraphModel* graphModel) : _url(url), _graphModel(graphModel) {}
    bool save() override;
};

using GMLSaverFactory = SaverFactory<GMLSaver>;

#endif // GMLEXPORTER_H
