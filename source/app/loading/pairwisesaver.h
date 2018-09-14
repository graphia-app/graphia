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
    // For the factory template we must appease
    static QString name() { return QStringLiteral("Pairwise Text"); }
    static QString extension() { return QStringLiteral(".txt"); }
    PairwiseSaver(const QUrl& url, IGraphModel* graphModel) : _url(url), _graphModel(graphModel) {}
    bool save() override;
};

using PairwiseSaverFactory = SaverFactory<PairwiseSaver>;

#endif // PAIRWISEEXPORTER_H
