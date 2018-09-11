#ifndef GRAPHMLEXPORTER_H
#define GRAPHMLEXPORTER_H

#include "loading/iexporter.h"

class GraphMLExporter : public IExporter
{
public:
    GraphMLExporter();

    void uncancel() override;
    void cancel() override;
    bool cancelled() const override;
    bool save(const QUrl &url, IGraphModel *graphModel) override;
    QString name() const override;
    QString extension() const override;
};

#endif // GRAPHMLEXPORTER_H
