#ifndef GENERICPLUGIN_H
#define GENERICPLUGIN_H

#include "shared/interfaces/baseplugin.h"

#include "loading/gmlfileparser.h"
#include "loading/pairwisetxtfileparser.h"

#include "shared/graph/grapharray.h"

#include <memory>

class GenericPlugin : public BasePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginIID FILE "genericplugin.json")

private:
    GmlFileParser _gmlFileParser;
    PairwiseTxtFileParser _pairwiseTxtFileParser;

    std::unique_ptr<EdgeArray<float>> _edgeWeights;

public:
    GenericPlugin();

    QStringList identifyUrl(const QUrl& url) const;
    IParser* parserForUrlTypeName(const QString& urlTypeName);

    bool editable() const { return true; }
    QString contentQmlPath() const { return {}; }

    void setNodeName(NodeId nodeId, const QString& name);
    void setEdgeWeight(EdgeId edgeId, float weight);

private slots:
    void onGraphChanged();
};

#endif // GENERICPLUGIN_H
