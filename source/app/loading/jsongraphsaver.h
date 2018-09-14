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
    // For the factory template we must appease
    static QString name() { return QStringLiteral("JSON Graph"); }
    static QString extension() { return QStringLiteral(".json"); }
    JSONGraphSaver(const QUrl& url, IGraphModel* graphModel) : _url(url), _graphModel(graphModel) {}
    bool save() override;
};

using JSONGraphSaverFactory = SaverFactory<JSONGraphSaver>;

#endif // JSONGRAPHEXPORTER_H
