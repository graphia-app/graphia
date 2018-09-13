#ifndef PAIRWISEEXPORTER_H
#define PAIRWISEEXPORTER_H

#include "loading/isaver.h"

#include <QString>

class PairwiseSaverFactory : public ISaverFactory
{
public:
    QString name() const override { return QStringLiteral("Pairwise Text"); }
    QString extension() const override { return QStringLiteral(".txt"); }
    std::unique_ptr<ISaver> create(const QUrl& url, Document* document,
                                   const IPluginInstance*,
                                   const QByteArray&, const QByteArray&) override;
};

class PairwiseSaver : public ISaver
{
private:
    const QUrl& _url;
    IGraphModel* _graphModel;
public:
    PairwiseSaver(const QUrl& url, IGraphModel* graphModel) : _url(url), _graphModel(graphModel) {}
    bool save() override;
};

#endif // PAIRWISEEXPORTER_H
