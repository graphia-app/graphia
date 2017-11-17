#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include "graph/graph.h"
#include "graph/mutablegraph.h"
#include "shared/graph/grapharray.h"

#include "transform/transformedgraph.h"
#include "transform/transforminfo.h"
#include "attributes/attribute.h"

#include "shared/ui/visualisations/ielementvisual.h"
#include "ui/visualisations/visualisationchannel.h"
#include "ui/visualisations/visualisationinfo.h"

#include "layout/nodepositions.h"

#include "shared/graph/igraphmodel.h"

#include <QString>
#include <QStringList>

#include <memory>
#include <utility>
#include <map>
#include <vector>
#include <atomic>

class SelectionManager;
class SearchManager;
class ICommand;
class IPlugin;

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
    std::vector<QString> _previousDynamicAttributeNames;
    std::map<QString, std::unique_ptr<GraphTransformFactory>> _graphTransformFactories;

    std::map<QString, std::unique_ptr<VisualisationChannel>> _visualisationChannels;

    void removeDynamicAttributes();
    QString normalisedAttributeName(QString attribute) const;

public:
    MutableGraph& mutableGraph() override { return _graph; }
    const Graph& graph() const override { return _transformedGraph; }
    NodePositions& nodePositions() { return _nodePositions; }
    const NodePositions& nodePositions() const { return _nodePositions; }

    const ElementVisual& nodeVisual(NodeId nodeId) const override { return _nodeVisuals.at(nodeId); }
    const ElementVisual& edgeVisual(EdgeId edgeId) const override { return _edgeVisuals.at(edgeId); }

    const NodeArray<QString>& nodeNames() const { return _nodeNames; }

    QString nodeName(NodeId nodeId) const override { return _nodeNames[nodeId]; }
    void setNodeName(NodeId nodeId, const QString& name) override;

    const QString& name() const { return _name; }

    bool editable() const;
    QString pluginName() const;
    int pluginDataVersion() const;
    QString pluginQmlPath() const;

    bool graphTransformIsValid(const QString& transform) const;
    void buildTransforms(const QStringList& transforms, ICommand* command = nullptr);
    void cancelTransformBuild();

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

    std::vector<QString> attributeNames(ElementType elementType = ElementType::All) const override;

    void patchAttributeNames(QStringList& transforms, QStringList& visualisations) const;
    Attribute& createAttribute(QString name) override;

    void addAttributes(const std::map<QString, Attribute>& attributes);
    void removeAttribute(const QString& name);

    const Attribute* attributeByName(const QString& name) const override;
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
    void attributeAdded(const QString& name);
    void attributeRemoved(const QString& name);
};

#endif // GRAPHMODEL_H
