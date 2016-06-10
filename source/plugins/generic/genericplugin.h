#ifndef GENERICPLUGIN_H
#define GENERICPLUGIN_H

#include "shared/interfaces/baseplugin.h"

#include "shared/graph/grapharray.h"

#include <memory>

class GenericPluginInstance : public BasePluginInstance
{
    Q_OBJECT

private:
    std::unique_ptr<EdgeArray<float>> _edgeWeights;

public:
    GenericPluginInstance();

    std::unique_ptr<IParser> parserForUrlTypeName(const QString& urlTypeName);

    void setNodeName(NodeId nodeId, const QString& name);
    void setEdgeWeight(EdgeId edgeId, float weight);

private slots:
    void onGraphChanged();
};

class GenericPlugin : public BasePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginIID FILE "genericplugin.json")

public:
    GenericPlugin();

    QStringList identifyUrl(const QUrl& url) const;
    std::unique_ptr<IPluginInstance> createInstance(IGraphModel* graphModel);

    bool editable() const { return true; }
    QString contentQmlPath() const { return {}; }
};

#endif // GENERICPLUGIN_H
