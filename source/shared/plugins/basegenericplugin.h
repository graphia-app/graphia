#ifndef BASEGENERICPLUGIN_H
#define BASEGENERICPLUGIN_H

#include "baseplugin.h"

#include "shared/graph/grapharray.h"

#include <memory>

// This implements some basic functionality that will be common to plugins
// that load from generic graph file formats

class BaseGenericPluginInstance : public BasePluginInstance
{
    Q_OBJECT

    Q_PROPERTY(QString selectedNodeNames READ selectedNodeNames NOTIFY selectedNodeNamesChanged)
    Q_PROPERTY(float selectedNodeMeanDegree READ selectedNodeMeanDegree NOTIFY selectedNodeMeanDegreeChanged)

private:
    std::unique_ptr<EdgeArray<float>> _edgeWeights;

public:
    BaseGenericPluginInstance();

    std::unique_ptr<IParser> parserForUrlTypeName(const QString& urlTypeName);

    void setNodeName(NodeId nodeId, const QString& name);
    void setEdgeWeight(EdgeId edgeId, float weight);

private:
    QString selectedNodeNames() const;

    float selectedNodeMeanDegree() const;

private slots:
    void onGraphChanged();

    void onSelectionChanged(const ISelectionManager*);

signals:
    void selectedNodeNamesChanged();
    void selectedNodeMeanDegreeChanged();
};

class BaseGenericPlugin : public BasePlugin
{
    Q_OBJECT

public:
    BaseGenericPlugin();

    QStringList identifyUrl(const QUrl& url) const;

    bool editable() const { return true; }
};

#endif // BASEGENERICPLUGIN_H
