#ifndef JSONGRAPHEXPORTER_H
#define JSONGRAPHEXPORTER_H

#include "saverfactory.h"

#include <QString>

class JSONGraphSaver : public ISaver
{
private:
    const QUrl& _url;
    IGraphModel* _graphModel;
public:
    static QString name() { return QStringLiteral("JSON Graph"); }
    static QString extension() { return QStringLiteral("json"); }

    static json graphAsJson(const IGraph &graph, Progressable &progressable);
    static json nodePositionsAsJson(const IGraph& graph, const NodePositions& nodePositions,
                                    Progressable& progressable);
    static json nodeNamesAsJson(IGraphModel& graphModel, Progressable& progressable);
    static json bookmarksAsJson(const Document& document);
    static json layoutSettingsAsJson(const Document& document);

    JSONGraphSaver(const QUrl& url, IGraphModel* graphModel) : _url(url), _graphModel(graphModel) {}
    bool save() override;

};

using JSONGraphSaverFactory = SaverFactory<JSONGraphSaver>;

#endif // JSONGRAPHEXPORTER_H
