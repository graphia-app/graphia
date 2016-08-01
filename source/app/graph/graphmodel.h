#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include "../graph/graph.h"
#include "../graph/mutablegraph.h"
#include "shared/graph/grapharray.h"

#include "../transform/transformedgraph.h"
#include "../transform/datafield.h"

#include "../ui/graphtransformconfiguration.h"

#include "../layout/nodepositions.h"

#include "shared/plugins/iplugin.h"
#include "shared/graph/igraphmodel.h"

#include <QQuickItem>
#include <QString>
#include <QStringList>
#include <QColor>

#include <memory>
#include <utility>
#include <map>

struct NodeVisual
{
    NodeVisual() noexcept {}
    NodeVisual(NodeVisual&& other) noexcept :
        _size(other._size),
        _color(other._color)
    {}

    float _size = 1.0f;
    QColor _color;
};

using NodeVisuals = NodeArray<NodeVisual>;

struct EdgeVisual
{
    EdgeVisual() noexcept {}
    EdgeVisual(EdgeVisual&& other) noexcept :
        _size(other._size),
        _color(other._color)
    {}

    float _size = 1.0f;
    QColor _color;
};

using EdgeVisuals = EdgeArray<EdgeVisual>;

class GraphModel : public QObject, public IGraphModel
{
    Q_OBJECT
public:
    GraphModel(const QString& name, IPlugin* plugin);

private:
    MutableGraph _graph;
    TransformedGraph _transformedGraph;
    NodePositions _nodePositions;
    NodeVisuals _nodeVisuals;
    EdgeVisuals _edgeVisuals;

    NodeArray<QString> _nodeNames;

    QString _name;
    IPlugin* _plugin;

    std::map<QString, DataField> _dataFields;
    std::map<QString, std::pair<DataFieldElementType, std::unique_ptr<GraphTransformFactory>>> _graphTransformFactories;

    void updateVisuals();

public:
    MutableGraph& mutableGraph() { return _graph; }
    const Graph& graph() const { return _transformedGraph; }
    NodePositions& nodePositions() { return _nodePositions; }
    const NodePositions& nodePositions() const { return _nodePositions; }

    const NodeVisuals& nodeVisuals() const { return _nodeVisuals; }
    const EdgeVisuals& edgeVisuals() const { return _edgeVisuals; }

    const NodeArray<QString>& nodeNames() const { return _nodeNames; }

    QString nodeName(NodeId nodeId) const { return _nodeNames[nodeId]; }
    void setNodeName(NodeId nodeId, const QString& name) { _nodeNames[nodeId] = name; }

    const QString& name() const { return _name; }

    bool editable() const { return _plugin->editable(); }
    QString pluginQmlPath() const { return _plugin->qmlPath(); }

    std::vector<NodeId> findNodes(const QString& regex, std::vector<QString> dataFieldNames = {}) const;

    void buildTransforms(const std::vector<GraphTransformConfiguration>& graphTransformConfigurations);

    QStringList availableTransformNames() const;
    QStringList availableDataFields(const QString& transformName) const;
    DataFieldType typeOfDataField(const QString& dataFieldName) const;
    const DataField& dataFieldByName(const QString& name) const;
    QStringList avaliableConditionFnOps(const QString& dataFieldName) const;

    IDataField& dataField(const QString& name);

private slots:
    void onGraphChanged(const Graph*);
    void onPreferenceChanged(const QString& key, const QVariant& value);
};

#endif // GRAPHMODEL_H
