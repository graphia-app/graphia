#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include "shared/graph/grapharray.h"
#include "attributes/attribute.h"

#include "shared/graph/igraphmodel.h"

#include <QString>
#include <QStringList>
#include <QVariantMap>

#include <memory>
#include <map>
#include <vector>
#include <atomic>

class GraphModelImpl;
class Graph;
class MutableGraph;
class NodePositions;

class SelectionManager;
class SearchManager;

class ICommand;
class IPlugin;

struct ElementVisual;

class TransformInfo;
class VisualisationInfo;

class GraphTransformFactory;

class GraphModel : public QObject, public IGraphModel
{
    Q_OBJECT
public:
    GraphModel(QString name, IPlugin* plugin);
    ~GraphModel() override;

private:
    std::unique_ptr<GraphModelImpl> _;

    // While loading there may be lots of initial changes, and
    // we don't want to do many visual updates, so disable them
    bool _visualUpdatesEnabled = false;

    std::atomic_bool _transformedGraphIsChanging;
    QString _name;
    IPlugin* _plugin;

    void removeDynamicAttributes();
    QString normalisedAttributeName(QString attribute) const;

    IMutableGraph& mutableGraphImpl() override;
    const IGraph& graphImpl() const override;

    const IElementVisual& nodeVisualImpl(NodeId nodeId) const override;
    const IElementVisual& edgeVisualImpl(EdgeId edgeId) const override;

public:
    MutableGraph& mutableGraph();
    const Graph& graph() const;
    const ElementVisual& nodeVisual(NodeId nodeId) const;
    const ElementVisual& edgeVisual(EdgeId edgeId) const;

    NodePositions& nodePositions();
    const NodePositions& nodePositions() const;

    const NodeArray<QString>& nodeNames() const;

    QString nodeName(NodeId nodeId) const override;
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
                                    ValueType valueTypes = ValueType::All,
                                    AttributeFlag skipFlags = AttributeFlag::None) const;
    QStringList avaliableConditionFnOps(const QString& attributeName) const;
    bool hasTransformInfo() const;
    const TransformInfo& transformInfoAtIndex(int index) const;
    std::vector<QString> createdAttributeNamesAtTransformIndex(int index) const;

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

    Attribute& createAttribute(QString name) override;

    void addAttributes(const std::map<QString, Attribute>& attributes);
    void removeAttribute(const QString& name);

    const Attribute* attributeByName(const QString& name) const override;
    bool attributeExists(const QString& name) const override;
    Attribute attributeValueByName(const QString& name) const;

    void initialiseAttributeRanges();
    void initialiseUniqueAttributeValues();

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
    void onAttributeValuesChanged(QStringList attributeNames);

signals:
    void visualsWillChange();
    void visualsChanged();
    void attributesChanged(const QStringList& addedNames, const QStringList& removedNames);
    void attributeValuesChanged(const QString& name);
};

#endif // GRAPHMODEL_H
