#ifndef PAIRWISEEXPORTER_H
#define PAIRWISEEXPORTER_H

#include "loading/iexporter.h"

class PairwiseExporter : public IExporter
{
public:
    bool save(const QUrl &url, IGraphModel *graphModel) override;
    QString name() const override;
    QString extension() const override;
};

#endif // PAIRWISEEXPORTER_H
