#ifndef PAIRWISEEXPORTER_H
#define PAIRWISEEXPORTER_H

#include "loading/saverfactory.h"

#include <QString>

class PairwiseSaver : public ISaver
{
private:
    const QUrl& _url;
    IGraphModel* _graphModel;
public:
    PairwiseSaver(const QUrl& url, IGraphModel* graphModel) : _url(url), _graphModel(graphModel) {}
    bool save() override;
};

class PairwiseSaverFactory : public SaverFactory<PairwiseSaver>
{
public:
    QString name() const override { return QStringLiteral("Pairwise Text"); }
    QString extension() const override { return QStringLiteral(".txt"); }
};

#endif // PAIRWISEEXPORTER_H
