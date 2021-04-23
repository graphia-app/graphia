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

#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "shared/ui/idocument.h"

#include "attributes/availableattributesmodel.h"
#include "commands/commandmanager.h"
#include "graph/qmlelementid.h"
#include "layout/layout.h"
#include "loading/parserthread.h"
#include "rendering/compute/gpucomputethread.h"
#include "rendering/projection.h"
#include "rendering/shading.h"
#include "shared/plugins/iplugin.h"
#include "shared/utils/deferredexecutor.h"
#include "shared/utils/preferenceswatcher.h"
#include "shared/utils/qmlenum.h"
#include "shared/utils/failurereason.h"
#include "transform/availabletransformsmodel.h"
#include "ui/findoptions.h"

#include "attributes/enrichmenttablemodel.h"

#include <QQuickItem>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariantMap>
#include <QByteArray>

#include <vector>
#include <memory>
#include <mutex>
#include <atomic>

class Graph;
class Application;
class GraphQuickItem;
class GraphModel;
class SearchManager;
class SelectionManager;
class TabularData;

DEFINE_QML_ENUM(
    Q_GADGET, LayoutPauseState,
    Running, RunningFinished, Paused);

class Document : public QObject, public IDocument, public FailureReason
{
    Q_OBJECT

    Q_PROPERTY(Application* application MEMBER _application NOTIFY applicationChanged)
    Q_PROPERTY(GraphQuickItem* graph MEMBER _graphQuickItem NOTIFY graphQuickItemChanged)
    Q_PROPERTY(QObject* plugin READ pluginInstance NOTIFY pluginInstanceChanged)
    Q_PROPERTY(QString pluginName MEMBER _pluginName NOTIFY pluginNameChanged)
    Q_PROPERTY(QString pluginQmlPath READ pluginQmlPath NOTIFY pluginQmlPathChanged)

    Q_PROPERTY(QColor contrastingColor READ contrastingColorForBackground NOTIFY contrastingColorChanged)

    Q_PROPERTY(QString title MEMBER _title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString status MEMBER _status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(QString log MEMBER _log WRITE setLog NOTIFY logChanged)
    Q_PROPERTY(bool loadComplete MEMBER _loadComplete NOTIFY loadComplete)
    Q_PROPERTY(QString failureReason READ failureReason WRITE setFailureReason NOTIFY failureReasonChanged)

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool editable READ editable NOTIFY editableChanged)
    Q_PROPERTY(bool directed READ directed NOTIFY directedChanged)

    Q_PROPERTY(bool graphChanging READ graphChanging NOTIFY graphChangingChanged)

    Q_PROPERTY(QVariantList enrichmentTableModels READ enrichmentTableModels NOTIFY enrichmentTableModelsChanged)

    Q_PROPERTY(bool commandInProgress READ commandInProgress NOTIFY commandInProgressChanged)
    Q_PROPERTY(int commandProgress READ commandProgress NOTIFY commandProgressChanged)
    Q_PROPERTY(QString commandVerb READ commandVerb NOTIFY commandVerbChanged)
    Q_PROPERTY(bool commandIsCancellable READ commandIsCancellable NOTIFY commandIsCancellableChanged)
    Q_PROPERTY(bool commandIsCancelling READ commandIsCancelling NOTIFY commandIsCancellingChanged)

    Q_PROPERTY(QML_ENUM_PROPERTY(LayoutPauseState) layoutPauseState READ layoutPauseState NOTIFY layoutPauseStateChanged)
    Q_PROPERTY(QString layoutDisplayName READ layoutDisplayName NOTIFY layoutDisplayNameChanged)

    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(QString nextUndoAction READ nextUndoAction NOTIFY nextUndoActionChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    Q_PROPERTY(QString nextRedoAction READ nextRedoAction NOTIFY nextRedoActionChanged)

    Q_PROPERTY(bool canResetView READ canResetView NOTIFY canResetViewChanged)
    Q_PROPERTY(bool canEnterOverviewMode READ canEnterOverviewMode NOTIFY canEnterOverviewModeChanged)

    Q_PROPERTY(QStringList transforms MEMBER _graphTransformsFromUI NOTIFY transformsChanged)
    Q_PROPERTY(QStringList visualisations MEMBER _visualisationsFromUI NOTIFY visualisationsChanged)
    Q_PROPERTY(QStringList layoutSettingNames MEMBER _layoutSettingNames NOTIFY layoutSettingNamesChanged)

    Q_PROPERTY(float fps READ fps NOTIFY fpsChanged)

    Q_PROPERTY(bool saveRequired MEMBER _saveRequired NOTIFY saveRequiredChanged)

    Q_PROPERTY(int foundIndex READ foundIndex NOTIFY foundIndexChanged)
    Q_PROPERTY(int numNodesFound READ numNodesFound NOTIFY numNodesFoundChanged)

    Q_PROPERTY(int numNodesSelected READ numNodesSelected NOTIFY numNodesSelectedChanged)
    Q_PROPERTY(int numHeadNodesSelected READ numHeadNodesSelected NOTIFY numHeadNodesSelectedChanged)
    Q_PROPERTY(int numInvisibleNodesSelected READ numInvisibleNodesSelected NOTIFY numInvisibleNodesSelectedChanged)
    Q_PROPERTY(bool nodesMaskActive READ nodesMaskActive NOTIFY nodesMaskActiveChanged)

    Q_PROPERTY(QVariantList selectedNodeIds READ selectedNodeIds NOTIFY selectedNodeIdsChanged)
    Q_PROPERTY(QVariantList selectedHeadNodeIds READ selectedHeadNodeIds NOTIFY selectedHeadNodeIdsChanged)

    Q_PROPERTY(QStringList bookmarks READ bookmarks NOTIFY bookmarksChanged)

    Q_PROPERTY(bool hasValidEdgeTextVisualisation READ hasValidEdgeTextVisualisation
        NOTIFY hasValidEdgeTextVisualisationChanged)

public:
    explicit Document(QObject* parent = nullptr);
    ~Document() override;

public: // IDocument
    const IGraphModel* graphModel() const override;
    IGraphModel* graphModel() override;

    const ISelectionManager* selectionManager() const override;
    ISelectionManager* selectionManager() override;

    const ICommandManager* commandManager() const override { return &_commandManager; }
    ICommandManager* commandManager() override { return &_commandManager; }

    MessageBoxButton messageBox(MessageBoxIcon icon, const QString& title, const QString& text,
        Flags<MessageBoxButton> buttons = MessageBoxButton::Ok) override;

    void moveFocusToNode(NodeId nodeId) override;
    void moveFocusToNodes(const std::vector<NodeId>& nodeIds) override;

    void clearHighlightedNodes() override;
    void highlightNodes(const NodeIdSet& nodeIds) override;

    void reportProblem(const QString& description) const override;

    const QString& log() const override;
    void setLog(const QString& log) override;

public:
    static QColor contrastingColorForBackground();

    bool commandInProgress() const;
    bool busy() const;
    bool editable() const;
    bool directed() const;
    bool graphChanging() const;

    int commandProgress() const;
    QString commandVerb() const;
    bool commandIsCancellable() const;
    bool commandIsCancelling() const;

    QString layoutName() const;
    QString layoutDisplayName() const;
    std::vector<LayoutSetting>& layoutSettings() const;
    void updateLayoutDimensionality();
    void updateLayoutState();
    LayoutPauseState layoutPauseState();

    bool canUndo() const;
    QString nextUndoAction() const;
    bool canRedo() const;
    QString nextRedoAction() const;

    bool canResetView() const;
    bool canEnterOverviewMode() const;

    void setTitle(const QString& title);
    void setStatus(const QString& status);

    QStringList transforms() const { return _graphTransforms; }
    void setTransforms(const QStringList& transforms);

    QStringList visualisations() const { return _visualisations; }
    void setVisualisations(const QStringList& visualisations);

    const QVariantList& enrichmentTableModels() { return _enrichmentTableModels; }

    float fps() const;

    QObject* pluginInstance();
    QString pluginQmlPath() const;

    QStringList bookmarks() const;
    NodeIdSet nodeIdsForBookmark(const QString& name) const;

    size_t executeOnMainThread(DeferredExecutor::TaskFn task,
        const QString& description = QStringLiteral("GenericTask"));

    void executeOnMainThreadAndWait(DeferredExecutor::TaskFn task,
        const QString& description = QStringLiteral("GenericTask"));

private:
    Application* _application = nullptr;
    GraphQuickItem* _graphQuickItem = nullptr;

    PreferencesWatcher _preferencesWatcher;

    QString _title;
    QString _status;
    QString _log;

    int _loadProgress = 0;
    bool _loadComplete = false;

    std::atomic_bool _graphChanging;
    std::atomic_bool _layoutRequired;

    std::unique_ptr<GraphModel> _graphModel;
    std::unique_ptr<GPUComputeThread> _gpuComputeThread;

    std::unique_ptr<IPluginInstance> _pluginInstance;
    QString _pluginName;

    std::unique_ptr<SelectionManager> _selectionManager;
    std::unique_ptr<SearchManager> _searchManager;
    CommandManager _commandManager;
    std::unique_ptr<ParserThread> _graphFileParserThread;

    std::unique_ptr<LayoutThread> _layoutThread;
    QStringList _layoutSettingNames;

    DeferredExecutor _deferredExecutor;

    std::vector<NodeId> _foundNodeIds;
    bool _foundItValid = false;
    std::vector<NodeId>::const_iterator _foundIt = _foundNodeIds.begin();

    // This is a mildly awkward hack to ensure than the numNodesFoundChanged signal
    // gets emitted /after/ the foundIndexChanged signal so that the number of
    // found nodes text in the UI gets updated minimally
    bool _numNodesFoundChanged = false;

    QStringList _graphTransformsFromUI;
    QStringList _graphTransforms;

    QStringList _visualisationsFromUI;
    QStringList _visualisations;

    QVariantList _enrichmentTableModels;

    std::atomic_bool _saveRequired{false};

    QByteArray _uiData;

    QByteArray _pluginUiData;
    int _pluginUiDataVersion = -1;

    std::vector<LayoutSettingKeyValue> _loadedLayoutSettings;
    std::unique_ptr<ExactNodePositions> _startingNodePositions;
    bool _userLayoutPaused = false; // true if the user wants the layout to pause

    bool _previousBusy = true;

    std::map<QString, NodeIdSet> _bookmarks;

    QStringList graphTransformConfigurationsFromUI() const;

    bool hasValidEdgeTextVisualisation() const;

    void maybeEmitBusyChanged();

    int foundIndex() const;
    int numNodesFound() const;
    bool nodesMaskActive() const;
    void setFoundIt(std::vector<NodeId>::const_iterator foundIt);
    NodeId incrementFoundIt();
    NodeId decrementFoundIt();
    void selectAndFocusNode(NodeId nodeId);
    void selectAndFocusNodes(const std::vector<NodeId>& nodeIds);
    void selectAndFocusNodes(const NodeIdSet& nodeIds);
    void selectFoundNode(NodeId newFound);

    void setSaveRequired();

    int numNodesSelected() const;
    int numHeadNodesSelected() const;
    int numInvisibleNodesSelected() const;

    QVariantList selectedNodeIds() const;
    QVariantList selectedHeadNodeIds() const;

    void initialiseLayoutSettingsModel();

    QVariantMap transformParameter(const QString& transformName, const QString& parameterName) const;
    QVariantMap transformAttributeParameter(const QString& transformName, const QString& parameterName) const;

signals:
    void applicationChanged();
    void graphQuickItemChanged();

    void uiDataChanged(const QByteArray& uiData);

    void pluginInstanceChanged();
    void pluginNameChanged();
    void pluginQmlPathChanged(const QByteArray& pluginUiData, int pluginUiDataVersion);

    void loadComplete(const QUrl& url, bool success);
    void failureReasonChanged();

    void titleChanged();
    void contrastingColorChanged();
    void statusChanged();
    void logChanged();

    void busyChanged();
    void editableChanged();
    void directedChanged();

    void graphWillChange(const Graph* graph);
    void graphChanged(const Graph* graph, bool changeOccurred);
    void graphChangingChanged();

    void commandInProgressChanged();
    void commandProgressChanged();
    void commandVerbChanged();
    void commandIsCancellableChanged();
    void commandIsCancellingChanged();

    void layoutPauseStateChanged();
    void layoutDisplayNameChanged();

    void commandsFinished();
    void canUndoChanged();
    void nextUndoActionChanged();
    void canRedoChanged();
    void nextRedoActionChanged();

    void canResetViewChanged();
    void canEnterOverviewModeChanged();

    void fpsChanged();

    void saveRequiredChanged();

    void foundIndexChanged();
    void numNodesFoundChanged();
    void nodesMaskActiveChanged();

    void numNodesSelectedChanged();
    void numHeadNodesSelectedChanged();
    void numInvisibleNodesSelectedChanged();

    void selectedNodeIdsChanged();
    void selectedHeadNodeIdsChanged();

    void bookmarksChanged();

    void taskAddedToExecutor();

    void enrichmentTableModelsChanged();
    void enrichmentAnalysisComplete();

    void saveComplete(bool success, QUrl fileUrl, const QString& saverName);

    void attributesChanged(const QStringList& addedNames, const QStringList& removedNames,
        const QStringList& changedValuesNames);

    void hasValidEdgeTextVisualisationChanged();

    void transformsChanged();
    void visualisationsChanged();
    void layoutSettingNamesChanged();

public:
    // Main QML interface
    Q_INVOKABLE bool openFile(const QUrl& fileUrl,
                              const QString& fileType,
                              QString pluginName,
                              const QVariantMap& parameters);

    Q_INVOKABLE void saveFile(const QUrl& fileUrl,
                              const QString& saverName,
                              const QByteArray& uiData,
                              const QByteArray& pluginUiData);

    Q_INVOKABLE void onPreferenceChanged(const QString& key, const QVariant& value);

    Q_INVOKABLE void setUserLayoutPaused(bool userLayoutPaused);
    Q_INVOKABLE void resumeLayout();
    Q_INVOKABLE void toggleLayout();

    Q_INVOKABLE bool nodeIsSelected(QmlNodeId nodeId) const;

    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void selectAllFound();
    Q_INVOKABLE void selectAllVisible();
    Q_INVOKABLE void selectNone();
    Q_INVOKABLE void selectSources();
    Q_INVOKABLE void selectSourcesOf(QmlNodeId nodeId);
    Q_INVOKABLE void selectTargets();
    Q_INVOKABLE void selectTargetsOf(QmlNodeId nodeId);
    Q_INVOKABLE void selectNeighbours();
    Q_INVOKABLE void selectNeighboursOf(QmlNodeId nodeId);
    Q_INVOKABLE void selectBySharedAttributeValue(const QString& attributeName, QmlNodeId qmlNodeId = {});
    Q_INVOKABLE void invertSelection();

    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    Q_INVOKABLE void rollback();

    Q_INVOKABLE void deleteNode(QmlNodeId nodeId);
    Q_INVOKABLE void deleteSelectedNodes();

    Q_INVOKABLE void resetView();

    Q_INVOKABLE void switchToOverviewMode(bool doTransition = true);

    Q_INVOKABLE int projection() const;
    Q_INVOKABLE void setProjection(int projection);

    Q_INVOKABLE int shading2D() const;
    Q_INVOKABLE void setShading2D(int _shading2D);

    Q_INVOKABLE int shading3D() const;
    Q_INVOKABLE void setShading3D(int _shading3D);

    Q_INVOKABLE void gotoPrevComponent();
    Q_INVOKABLE void gotoNextComponent();

    Q_INVOKABLE void find(const QString& term, int options,
        const QStringList& attributeNames, int findSelectStyle);
    Q_INVOKABLE void resetFind();
    Q_INVOKABLE void selectFirstFound();
    Q_INVOKABLE void selectNextFound();
    Q_INVOKABLE void selectPrevFound();
    Q_INVOKABLE void updateFoundIndex(bool reselectIfInvalidated);

    Q_INVOKABLE void selectByAttributeValue(const QString& attributeName, const QString& term);

    Q_INVOKABLE QString nodeName(QmlNodeId nodeId) const;

    Q_INVOKABLE AvailableTransformsModel* availableTransforms() const;
    Q_INVOKABLE QVariantMap transform(const QString& transformName) const;
    Q_INVOKABLE QVariantMap findTransformParameter(const QString& transformName, const QString& parameterName) const;
    Q_INVOKABLE bool hasTransformInfo() const;
    Q_INVOKABLE QVariantMap transformInfoAtIndex(int index) const;

    Q_INVOKABLE bool opIsUnary(const QString& op) const;

    Q_INVOKABLE QStringList availableAttributeNames(int _elementTypes = static_cast<int>(ElementType::All),
        int _valueTypes = static_cast<int>(ValueType::All), int _skipFlags = static_cast<int>(AttributeFlag::None),
        const QStringList& skipAttributeNames = {}) const;
    Q_INVOKABLE AvailableAttributesModel* availableAttributesModel(int elementTypes = static_cast<int>(ElementType::All),
        int valueTypes = static_cast<int>(ValueType::All), int skipFlags = static_cast<int>(AttributeFlag::None),
        const QStringList& skipAttributeNames = {}) const;
    Q_INVOKABLE bool attributeExists(const QString& attributeName) const;
    Q_INVOKABLE QVariantMap attribute(const QString& attributeName) const;
    Q_INVOKABLE AvailableAttributesModel* attributesSimilarTo(const QString& attributeName,
        int skipFlags = static_cast<int>(AttributeFlag::None)) const;
    Q_INVOKABLE QStringList allAttributeValues(const QString& attributeName) const;

    Q_INVOKABLE QStringList createdAttributeNamesAtTransformIndexOrLater(int firstIndex) const;

    Q_INVOKABLE QVariantMap parseGraphTransform(const QString& transform) const;
    Q_INVOKABLE bool graphTransformIsValid(const QString& transform) const;
    Q_INVOKABLE void setGraphTransform(int index, const QString& transform);
    Q_INVOKABLE void removeGraphTransform(int index);
    Q_INVOKABLE void moveGraphTransform(int from, int to);

    Q_INVOKABLE QStringList availableVisualisationChannelNames(int valueType) const;
    Q_INVOKABLE bool visualisationChannelAllowsMapping(const QString& channelName) const;

    Q_INVOKABLE QStringList visualisationDescription(const QString& attributeName, const QStringList& channelNames) const;
    Q_INVOKABLE bool hasVisualisationInfo() const;
    Q_INVOKABLE QVariantMap visualisationInfoAtIndex(int index) const;

    Q_INVOKABLE QVariantMap parseVisualisation(const QString& visualisation) const;
    Q_INVOKABLE QVariantMap visualisationDefaultParameters(int valueType, const QString& channelName) const;
    Q_INVOKABLE bool visualisationIsValid(const QString& visualisation) const;
    Q_INVOKABLE void setVisualisation(int index, const QString& visualisation);
    Q_INVOKABLE void removeVisualisation(int index);
    Q_INVOKABLE void moveVisualisation(int from, int to);

    // Execute commands to apply transform and/or visualisation changes
    Q_INVOKABLE void update(QStringList newGraphTransforms = {},
        QStringList newVisualisations = {},
        bool replaceLatestCommand = false);

    Q_INVOKABLE QVariantMap layoutSetting(const QString& name) const;
    Q_INVOKABLE void setLayoutSettingValue(const QString& name, float value);
    Q_INVOKABLE void setLayoutSettingNormalisedValue(const QString& name, float normalisedValue);
    Q_INVOKABLE void resetLayoutSettingValue(const QString& name);

    Q_INVOKABLE void cancelCommand();

    Q_INVOKABLE void writeTableViewToFile(QObject* tableView, const QUrl& fileUrl,
        const QString& extension = "csv");

    Q_INVOKABLE void writeTableView2ToFile(QObject* tableView, const QUrl& fileUrl,
        const QString& extension = "csv");

    Q_INVOKABLE void copyTableViewCopyToClipboard(QObject* tableView, int column);

    Q_INVOKABLE void addBookmark(const QString& name);
    Q_INVOKABLE void removeBookmarks(const QStringList& names);
    Q_INVOKABLE void renameBookmark(const QString& from, const QString& to);
    Q_INVOKABLE void gotoBookmark(const QString& name);
    Q_INVOKABLE void gotoAllBookmarks();

    Q_INVOKABLE void dumpGraph();

    Q_INVOKABLE void performEnrichment(const QString& selectedAttributeA, const QString& selectedAttributeB);
    Q_INVOKABLE void removeEnrichmentResults(int index);

    Q_INVOKABLE void saveNodePositionsToFile(const QUrl& fileUrl);

    Q_INVOKABLE void importAttributesFromTable(const QString& keyAttributeName,
        std::shared_ptr<TabularData> data, int keyColumnIndex,
        std::vector<int> importColumnIndices, bool replace);

    Q_INVOKABLE QString graphSizeSummary() const;
    Q_INVOKABLE QString commandStackSummary() const;

private slots:
    void onLoadProgress(int percentage);
    void onLoadComplete(const QUrl& url, bool success);

    void onBusyChanged() const;

    void onSelectionChanged(const SelectionManager* selectionManager);
    void onFoundNodeIdsChanged(const SearchManager* searchManager);

    void onPluginSaveRequired();

    void executeDeferred();
};

#endif // DOCUMENT_H
