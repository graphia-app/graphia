#ifndef GENERICPLUGIN_H
#define GENERICPLUGIN_H

#include "shared/interfaces/baseplugin.h"
#include "shared/interfaces/igenericplugininstance.h"

#include "shared/graph/grapharray.h"

#include <memory>

class GenericPluginInstance : public BasePluginInstance, public IGenericPluginInstance
{
    Q_OBJECT

    Q_PROPERTY(QString selectedNodeNames READ selectedNodeNames NOTIFY selectedNodeNamesChanged)
    Q_PROPERTY(float selectedNodeMeanDegree READ selectedNodeMeanDegree NOTIFY selectedNodeMeanDegreeChanged)

private:
    std::unique_ptr<EdgeArray<float>> _edgeWeights;

public:
    GenericPluginInstance();

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

class GenericPlugin : public BasePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginIID FILE "genericplugin.json")

public:
    GenericPlugin();

    QString name() const { return "Generic"; }
    QString description() const
    {
        return tr("A plugin that loads generic graphs from a variety "
                  "of file formats.");
    }
    QString imageSource() const { return "qrc:///tools.svg"; }

    QStringList identifyUrl(const QUrl& url) const;
    std::unique_ptr<IPluginInstance> createInstance();

    bool editable() const { return true; }
    QString qmlPath() const { return "qrc:///qml/genericplugin.qml"; }
};

#endif // GENERICPLUGIN_H
