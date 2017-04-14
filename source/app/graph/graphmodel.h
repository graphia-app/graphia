#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include "graph/graph.h"
#include "graph/mutablegraph.h"
#include "shared/graph/grapharray.h"

#include "transform/transformedgraph.h"
#include "transform/transforminfo.h"
#include "attributes/attribute.h"

#include "ui/visualisations/elementvisual.h"
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

    NameAttributeMap _attributes;
    std::map<QString, std::unique_ptr<GraphTransformFactory>> _graphTransformFactories;

    std::map<QString, std::unique_ptr<VisualisationChannel>> _visualisationChannels;

    void removeDynamicAttributes();

public:
    MutableGraph& mutableGraph() { return _graph; }
    const Graph& graph() const { return _transformedGraph; }
    NodePositions& nodePositions() { return _nodePositions; }
    const NodePositions& nodePositions() const { return _nodePositions; }

    const NodeVisuals& nodeVisuals() const { return _nodeVisuals; }
    const EdgeVisuals& edgeVisuals() const { return _edgeVisuals; }

    const NodeArray<QString>& nodeNames() const { return _nodeNames; }

    QString nodeName(NodeId nodeId) const { return _nodeNames[nodeId]; }
    void setNodeName(NodeId nodeId, const QString& name);

    const QString& name() const { return _name; }

    bool editable() const { return _plugin->editable(); }
    QString pluginQmlPath() const { return _plugin->qmlPath(); }

    bool graphTransformIsValid(const QString& transform) const;
    void buildTransforms(const QStringList& transforms);

    QStringList availableTransformNames() const;
    QStringList availableAttributes(ElementType elementTypes = ElementType::All,
                                    ValueType valueTypes = ValueType::All) const;
    QString descriptionFor(const QString& transformName) const;
    QStringList availableAttributesFor(const QString& transformName) const;
    QStringList avaliableConditionFnOps(const QString& attributeName) const;
    bool hasTransformInfo() const;
    const TransformInfo& transformInfoAtIndex(int index) const;

    bool visualisationIsValid(const QString& visualisation) const;
    void buildVisualisations(const QStringList& visualisations);

    QStringList availableVisualisationChannelNames(const QString& attributeName) const;
    QString visualisationDescription(const QString& attributeName, const QString& channelName) const;
    void clearVisualisationInfos();
    bool hasVisualisationInfo() const;
    const VisualisationInfo& visualisationInfoAtIndex(int index) const;
    QVariantMap visualisationDefaultParameters(const QString& attributeName,
                                               const QString& channelName) const;

    QStringList attributeNames(ElementType elementType) const;

    Attribute& createAttribute(const QString& name);
    const Attribute& attributeByName(const QString& name) const;

    const NameAttributeMap& attributes() const { return _attributes; }

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
};

#endif // GRAPHMODEL_H
