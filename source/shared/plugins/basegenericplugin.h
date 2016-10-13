#ifndef BASEGENERICPLUGIN_H
#define BASEGENERICPLUGIN_H

#include "baseplugin.h"

#include "shared/plugins/nodeattributes.h"
#include "shared/plugins/nodeattributestablemodel.h"

#include "shared/graph/grapharray.h"

#include <memory>

// This implements some basic functionality that will be common to plugins
// that load from generic graph file formats

class BaseGenericPluginInstance : public BasePluginInstance
{
    Q_OBJECT

    Q_PROPERTY(QString selectedNodeNames READ selectedNodeNames NOTIFY selectedNodeNamesChanged)
    Q_PROPERTY(QAbstractTableModel* nodeAttributes READ nodeAttributesTableModel CONSTANT)

private:
    std::unique_ptr<EdgeArray<float>> _edgeWeights;

    NodeAttributes _nodeAttributes;

    NodeAttributesTableModel _nodeAttributesTableModel;
    QAbstractTableModel* nodeAttributesTableModel() { return &_nodeAttributesTableModel; }

public:
    BaseGenericPluginInstance();

    std::unique_ptr<IParser> parserForUrlTypeName(const QString& urlTypeName);

    void setEdgeWeight(EdgeId edgeId, float weight);

private:
    void initialise(IGraphModel* graphModel, ISelectionManager* selectionManager, const IParserThread* parserThread);

    QString selectedNodeNames() const;

private slots:
    void onLoadComplete();

    void onGraphChanged();
    void onSelectionChanged(const ISelectionManager* selectionManager);

signals:
    void selectedNodeNamesChanged();
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
