/* Copyright © 2013-2020 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef GRAPHMODEL_H
#define GRAPHMODEL_H

#include "shared/graph/elementid_containers.h"
#include "shared/graph/grapharray.h"
#include "shared/graph/igraphmodel.h"

#include "shared/loading/userelementdata.h"

#include "shared/utils/preferenceswatcher.h"

#include "attributes/attribute.h"

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

class AttributeChangesTracker;

class GraphModel : public QObject, public IGraphModel
{
    friend class AttributeChangesTracker;

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

    PreferencesWatcher _preferencesWatcher;

    void removeDynamicAttributes();
    QString normalisedAttributeName(QString attribute) const;

    IMutableGraph& mutableGraphImpl() override;
    const IMutableGraph& mutableGraphImpl() const override;
    const IGraph& graphImpl() const override;

    const IElementVisual& nodeVisualImpl(NodeId nodeId) const override;
    const IElementVisual& edgeVisualImpl(EdgeId edgeId) const override;

public:
    MutableGraph& mutableGraph();
    const MutableGraph& mutableGraph() const;
    const Graph& graph() const;

    const ElementVisual& nodeVisual(NodeId nodeId) const;
    const ElementVisual& edgeVisual(EdgeId edgeId) const;
    std::vector<ElementVisual> nodeVisuals(const std::vector<NodeId>& nodeIds) const;
    std::vector<ElementVisual> edgeVisuals(const std::vector<EdgeId>& edgeIds) const;

    NodePositions& nodePositions();
    const NodePositions& nodePositions() const;

    const NodeArray<QString>& nodeNames() const;

    QString nodeName(NodeId nodeId) const override;
    void setNodeName(NodeId nodeId, const QString& name) override;

    const QString& name() const { return _name; }

    bool editable() const;
    bool directed() const;
    QString pluginName() const;
    int pluginDataVersion() const;
    QString pluginQmlPath() const;

    bool graphTransformIsValid(const QString& transform) const;
    QStringList transformsWithMissingParametersSetToDefault(const QStringList& transforms) const;
    void buildTransforms(const QStringList& transforms, ICommand* command = nullptr);
    void cancelTransformBuild();

    QStringList availableTransformNames() const;
    const GraphTransformFactory* transformFactory(const QString& transformName) const;
    QStringList availableAttributeNames(ElementType elementTypes = ElementType::All,
        ValueType valueTypes = ValueType::All, AttributeFlag skipFlags = AttributeFlag::None,
        const QStringList& skipAttributeNames = {}) const;
    QStringList avaliableConditionFnOps(const QString& attributeName) const;
    bool hasTransformInfo() const;
    const TransformInfo& transformInfoAtIndex(int index) const;
    std::vector<QString> createdAttributeNamesAtTransformIndex(int index) const;
    std::vector<QString> createdAttributeNamesAtTransformIndexOrLater(int firstIndex) const;

    static bool opIsUnary(const QString& op);

    bool visualisationIsValid(const QString& visualisation) const;
    void buildVisualisations(const QStringList& visualisations);
    bool hasValidEdgeTextVisualisation() const;

    QStringList availableVisualisationChannelNames(ValueType valueType) const;
    bool visualisationChannelAllowsMapping(const QString& channelName) const;

    QStringList visualisationDescription(const QString& attributeName, const QStringList& channelNames) const;
    void clearVisualisationInfos();
    bool hasVisualisationInfo() const;
    const VisualisationInfo& visualisationInfoAtIndex(int index) const;
    QVariantMap visualisationDefaultParameters(ValueType valueType,
                                               const QString& channelName) const;

    std::vector<QString> attributeNames(ElementType elementType = ElementType::All) const override;

    Attribute& createAttribute(QString name, QString* assignedName = nullptr) override;

    void addAttributes(const std::map<QString, Attribute>& attributes);
    void removeAttribute(const QString& name);

    std::unique_ptr<AttributeChangesTracker> attributeChangesTracker();

    const Attribute* attributeByName(const QString& name) const override;
    bool attributeExists(const QString& name) const override;
    bool attributeIsValid(const QString& name) const;
    Attribute attributeValueByName(const QString& name) const;

    static bool attributeNameIsValid(const QString& attributeName);

    static void calculateAttributeRange(const IGraph* graph, Attribute& attribute);
    void calculateAttributeRange(Attribute& attribute);
    void calculateAttributeRange(const QString& name);

    void updateSharedAttributeValues(Attribute& attribute);
    void updateSharedAttributeValues(const QString& name);

    void initialiseAttributeRanges();
    void initialiseSharedAttributeValues();

    UserNodeData& userNodeData() override;
    UserEdgeData& userEdgeData() override;

    void clearHighlightedNodes();
    void highlightNodes(const NodeIdSet& nodeIds);

    void enableVisualUpdates();
    void updateVisuals();

public slots:
    void onSelectionChanged(const SelectionManager* selectionManager);
    void onFoundNodeIdsChanged(const SearchManager* searchManager);
    void onPreferenceChanged(const QString&, const QVariant&);

private slots:
    void onMutableGraphChanged(const Graph* graph);
    void onTransformedGraphWillChange(const Graph* graph);
    void onTransformedGraphChanged(const Graph* graph);

    void onAttributesChanged(const QStringList& addedNames, const QStringList& removedNames,
        const QStringList& changedValuesNames);

signals:
    void visualsWillChange();
    void visualsChanged();
    void attributesChanged(const QStringList& addedNames, const QStringList& removedNames,
        const QStringList& changedValuesNames);

    void rebuildRequired(bool transforms, bool visualisations);
};

class AttributeChangesTracker
{
private:
    GraphModel* _graphModel;
    QStringList _attributesWithChangedValues;

public:
    AttributeChangesTracker(GraphModel& graphModel);
    ~AttributeChangesTracker();

    void setAttributeValuesChanged(const QString& attributeName);
};

#endif // GRAPHMODEL_H
