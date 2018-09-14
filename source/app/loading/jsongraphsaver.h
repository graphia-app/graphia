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
    JSONGraphSaver(const QUrl& url, IGraphModel* graphModel) : _url(url), _graphModel(graphModel) {}
    bool save() override;
};

class JSONGraphSaverFactory : public SaverFactory<JSONGraphSaver>
{
public:
    QString name() const override { return QStringLiteral("JSON Graph"); }
    QString extension() const override { return QStringLiteral(".json"); }
};


#endif // JSONGRAPHEXPORTER_H
