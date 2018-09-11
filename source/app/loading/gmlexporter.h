#ifndef GMLEXPORTER_H
#define GMLEXPORTER_H

#include "graph/graphmodel.h"
#include "loading/iexporter.h"

class GMLExporter : public IExporter
{

private:
    QString indent(int level) { return QStringLiteral("    ").repeated(level); }

public:
    bool save(const QUrl& url, IGraphModel* graphModel) override;
    QString name() const override { return "GML"; }
    QString extension() const override { return ".gml"; }
};

#endif // GMLEXPORTER_H
