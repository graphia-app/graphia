#ifndef GMLEXPORTER_H
#define GMLEXPORTER_H

#include "graph/graphmodel.h"
#include "loading/isaver.h"

class GMLSaver : public ISaver
{
private:
    const QUrl& _url;
    IGraphModel* _graphModel;
    QString indent(int level) { return QStringLiteral("    ").repeated(level); }

public:
    GMLSaver(const QUrl& url, IGraphModel* graphModel) : _url(url), _graphModel(graphModel) {}
    bool save() override;
};

class GMLSaverFactory : public ISaverFactory
{
public:
    QString name() const override { return QStringLiteral("GML"); }
    QString extension() const override { return QStringLiteral(".gml"); }
    virtual std::unique_ptr<ISaver> create(const QUrl& url, Document* document,
                                           const IPluginInstance* pluginInstance, const QByteArray& uiData,
                                           const QByteArray& pluginUiData);
};


#endif // GMLEXPORTER_H
