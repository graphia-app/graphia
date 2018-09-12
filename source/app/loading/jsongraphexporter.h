#ifndef JSONGRAPHEXPORTER_H
#define JSONGRAPHEXPORTER_H

#include "iexporter.h"

class JSONGraphExporter : public IExporter
{
public:
    bool save(const QUrl &url, IGraphModel *graphModel) override;
    QString name() const override;
    QString extension() const override;
};

#endif // JSONGRAPHEXPORTER_H
