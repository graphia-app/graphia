#ifndef WEBSEARCHPLUGIN_H
#define WEBSEARCHPLUGIN_H

#include "shared/interfaces/baseplugin.h"
#include "shared/interfaces/igenericplugininstance.h"

#include "shared/graph/grapharray.h"

#include <memory>

class WebSearchPluginInstance : public BasePluginInstance, public IGenericPluginInstance
{
    Q_OBJECT

    Q_PROPERTY(QString selectedNodeNames READ selectedNodeNames NOTIFY selectedNodeNamesChanged)

private:
    std::unique_ptr<EdgeArray<float>> _edgeWeights;

public:
    WebSearchPluginInstance();

    std::unique_ptr<IParser> parserForUrlTypeName(const QString& urlTypeName);

    void setNodeName(NodeId nodeId, const QString& name);
    void setEdgeWeight(EdgeId edgeId, float weight);

private:
    QString selectedNodeNames() const;

private slots:
    void onGraphChanged();
    void onSelectionChanged(const ISelectionManager*);

signals:
    void selectedNodeNamesChanged();
};

class WebSearchPlugin : public BasePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginIID FILE "websearchplugin.json")

public:
    WebSearchPlugin();

    QString name() const { return "WebSearch"; }
    QString description() const
    {
        return tr("An embedded web browser that searches for the "
                  "node selection using a URL template.");
    }
    QString imageSource() const { return "qrc:///globe.svg"; }

    QStringList identifyUrl(const QUrl& url) const;
    std::unique_ptr<IPluginInstance> createInstance();

    bool editable() const { return true; }
    QString qmlPath() const { return "qrc:///qml/websearchplugin.qml"; }
};

#endif // WEBSEARCHPLUGIN_H
