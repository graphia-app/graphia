#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include "graph/graph.h"
#include "graph/mutablegraph.h"
#include "shared/graph/grapharray.h"

#include "transform/transformedgraph.h"
#include "transform/transforminfo.h"
#include "attributes/attribute.h"

#include "shared/ui/visualisations/elementvisual.h"
#include "ui/visualisations/visualisationchannel.h"
#include "ui/visualisations/visualisationinfo.h"

#include "layout/nodepositions.h"

#include "shared/plugins/iplugin.h"
#include "shared/graph/igraphmodel.h"

#include <QQuickItem>
#include <QString>
#include <QStringList>

#include <memory>
#include <utility>
#include <map>
#include <atomic>

class SelectionManager;
class SearchManager;
class ICommand;

using NodeVisuals = NodeArray<ElementVisual>;
using EdgeVisuals = EdgeArray<ElementVisual>;

class GraphModel : public QObject, public IGraphModel
{
    Q_OBJECT
public:
    GraphModel(QString name, IPlugin* plugin);

private:
    MutableGraph _graph;
    TransformedGraph _transformedGraph;
    TransformInfosMap _transformInfos;
    NodePositions _nodePositions;

    NodeVisuals _nodeVisuals;
    EdgeVisuals _edgeVisuals;
    NodeVisuals _mappedNodeVisuals;
    EdgeVisuals _mappedEdgeVisuals;
    VisualisationInfosMap _visualisationInfos;

    // While loading there may be lots of initial changes, and
    // we don't want to do many visual updates, so disable them
    bool _visualUpdatesEnabled = false;

    std::atomic_bool _transformedGraphIsChanging;

    NodeArray<QString> _nodeNames;

    QString _name;
    IPlugin* _plugin;

    std::map<QString, Attribute> _attributes;
    QStringList _previousDynamicAttributeNames;
    std::map<QString, std::unique_ptr<GraphTransformFactory>> _graphTransformFactories;

    std::map<QString, std::unique_ptr<VisualisationChannel>> _visualisationChannels;

    void removeDynamicAttributes();

public:
    MutableGraph& mutableGraph() { return _graph; }
    const Graph& graph() const { return _transformedGraph; }
    NodePositions& nodePositions() { return _nodePositions; }
    const NodePositions& nodePositions() const { return _nodePositions; }

    const ElementVisual& nodeVisual(NodeId nodeId) const { return _nodeVisuals.at(nodeId); }
    const ElementVisual& edgeVisual(EdgeId edgeId) const { return _edgeVisuals.at(edgeId); }

    const NodeArray<QString>& nodeNames() const { return _nodeNames; }

    QString nodeName(NodeId nodeId) const { return _nodeNames[nodeId]; }
    void setNodeName(NodeId nodeId, const QString& name);

    const QString& name() const { return _name; }

    bool editable() const { return _plugin->editable(); }
    QString pluginQmlPath() const { return _plugin->qmlPath(); }

    bool graphTransformIsValid(const QString& transform) const;
    void buildTransforms(const QStringList& transforms, ICommand* command = nullptr);

    QStringList availableTransformNames() const;
    const GraphTransformFactory* transformFactory(const QString& transformName) const;
    QStringList availableAttributes(ElementType elementTypes = ElementType::All,
                                    ValueType valueTypes = ValueType::All) const;
    QStringList avaliableConditionFnOps(const QString& attributeName) const;
    bool hasTransformInfo() const;
    const TransformInfo& transformInfoAtIndex(int index) const;

    bool opIsUnary(const QString& op) const;

    bool visualisationIsValid(const QString& visualisation) const;
    void buildVisualisations(const QStringList& visualisations);

    QStringList availableVisualisationChannelNames(ValueType valueType) const;
    QString visualisationDescription(const QString& attributeName, const QString& channelName) const;
    void clearVisualisationInfos();
    bool hasVisualisationInfo() const;
    const VisualisationInfo& visualisationInfoAtIndex(int index) const;
    QVariantMap visualisationDefaultParameters(ValueType valueType,
                                               const QString& channelName) const;

    std::vector<QString> attributeNames(ElementType elementType = ElementType::All) const;
    std::vector<QString> nodeAttributeNames() const;

    Attribute& createAttribute(const QString& name);

    void addAttribute(const QString& name, const Attribute& attribute);
    void addAttributes(const std::map<QString, Attribute>& attributes);

    const Attribute* attributeByName(const QString& name) const;
    Attribute attributeValueByName(const QString& name) const;

    void enableVisualUpdates();
    void updateVisuals(const SelectionManager* selectionManager = nullptr, const SearchManager* searchManager = nullptr);

public slots:
    void onSelectionChanged(const SelectionManager* selectionManager);
    void onFoundNodeIdsChanged(const SearchManager* searchManager);
    void onPreferenceChanged(const QString&, const QVariant&);

private slots:
    void onMutableGraphChanged(const Graph* graph);
    void onTransformedGraphWillChange(const Graph* graph);
    void onTransformedGraphChanged(const Graph* graph);

signals:
    void visualsWillChange();
    void visualsChanged();
    void attributeAdded(QString name);
    void attributeRemoved(QString name);
};

#endif // GRAPHMODEL_H
