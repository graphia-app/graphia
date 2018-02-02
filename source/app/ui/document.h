#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "shared/ui/idocument.h"

#include "attributes/availableattributesmodel.h"
#include "commands/commandmanager.h"
#include "graph/qmlelementid.h"
#include "layout/layout.h"
#include "loading/parserthread.h"
#include "rendering/compute/gpucomputethread.h"
#include "shared/plugins/iplugin.h"
#include "shared/utils/deferredexecutor.h"
#include "shared/utils/qmlenum.h"
#include "shared/utils/semaphore.h"
#include "transform/availabletransformsmodel.h"
#include "ui/findoptions.h"

#include "thirdparty/qt-qml-models/QQmlVariantListModel.h"
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

DEFINE_QML_ENUM(Q_GADGET, LayoutPauseState,
                Running, RunningFinished, Paused);

class Document : public QObject, public IDocument
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
    Q_PROPERTY(bool loadComplete MEMBER _loadComplete NOTIFY loadComplete)

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool editable READ editable NOTIFY editableChanged)

    Q_PROPERTY(bool graphChanging READ graphChanging NOTIFY graphChangingChanged)

    Q_PROPERTY(QList<QObject*> listEnrichmentTableModels READ listEnrichmentTableModels NOTIFY enrichmentTableModelsChanged)

    Q_PROPERTY(bool commandInProgress READ commandInProgress NOTIFY commandInProgressChanged)
    Q_PROPERTY(int commandProgress READ commandProgress NOTIFY commandProgressChanged)
    Q_PROPERTY(QString commandVerb READ commandVerb NOTIFY commandVerbChanged)
    Q_PROPERTY(bool commandIsCancellable READ commandIsCancellable NOTIFY commandIsCancellableChanged)
    Q_PROPERTY(bool commandIsCancelling READ commandIsCancelling NOTIFY commandIsCancellingChanged)

    Q_PROPERTY(QML_ENUM_PROPERTY(LayoutPauseState) layoutPauseState READ layoutPauseState NOTIFY layoutPauseStateChanged)

    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(QString nextUndoAction READ nextUndoAction NOTIFY nextUndoActionChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    Q_PROPERTY(QString nextRedoAction READ nextRedoAction NOTIFY nextRedoActionChanged)

    Q_PROPERTY(bool canResetView READ canResetView NOTIFY canResetViewChanged)
    Q_PROPERTY(bool canEnterOverviewMode READ canEnterOverviewMode NOTIFY canEnterOverviewModeChanged)

    Q_PROPERTY(QQmlVariantListModel* transforms READ transformsModel CONSTANT)
    Q_PROPERTY(QQmlVariantListModel* visualisations READ visualisationsModel CONSTANT)
    Q_PROPERTY(QQmlVariantListModel* layoutSettings READ settingsModel CONSTANT)

    Q_PROPERTY(float fps READ fps NOTIFY fpsChanged)

    Q_PROPERTY(bool saveRequired MEMBER _saveRequired NOTIFY saveRequiredChanged)

    Q_PROPERTY(int foundIndex READ foundIndex NOTIFY foundIndexChanged)
    Q_PROPERTY(int numNodesFound READ numNodesFound NOTIFY numNodesFoundChanged)

    Q_PROPERTY(int numNodesSelected READ numNodesSelected NOTIFY numNodesSelectedChanged)
    Q_PROPERTY(QStringList attributeGroupNames READ attributeGroupNames NOTIFY attributeGroupNamesChanged)


    Q_PROPERTY(QStringList bookmarks READ bookmarks NOTIFY bookmarksChanged)

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

public:
    static QColor contrastingColorForBackground();

    bool commandInProgress() const;
    bool busy() const;
    bool editable() const;
    bool graphChanging() const;

    int commandProgress() const;
    QString commandVerb() const;
    bool commandIsCancellable() const;
    bool commandIsCancelling() const;

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

    QQmlVariantListModel* transformsModel() { return &_graphTransformsModel; }
    QStringList transforms() const { return _graphTransforms; }
    void setTransforms(const QStringList& transforms);

    QQmlVariantListModel* visualisationsModel() { return &_visualisationsModel; }
    QStringList visualisations() const { return _visualisations; }
    void setVisualisations(const QStringList& visualisations);

    QQmlVariantListModel* settingsModel() { return &_layoutSettingsModel; }

    float fps() const;

    QObject* pluginInstance();
    QString pluginQmlPath() const;

    QStringList bookmarks() const;
    NodeIdSet nodeIdsForBookmark(const QString& name) const;

    void executeOnMainThread(DeferredExecutor::TaskFn task,
                             const QString& description = QStringLiteral("GenericTask"));

    void executeOnMainThreadAndWait(DeferredExecutor::TaskFn task,
                                    const QString& description = QStringLiteral("GenericTask"));

private:
    Application* _application = nullptr;
    GraphQuickItem* _graphQuickItem = nullptr;

    QString _title;
    QString _status;

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
    QQmlVariantListModel _layoutSettingsModel;

    DeferredExecutor _deferredExecutor;
    semaphore _executed;

    std::vector<std::unique_ptr<EnrichmentTableModel>> enrichmentTableModels;

    std::vector<NodeId> _foundNodeIds;
    bool _foundItValid = false;
    std::vector<NodeId>::const_iterator _foundIt = _foundNodeIds.begin();

    // This is a mildly awkard hack to ensure than the numNodeFoundChanged signal
    // gets emitted /after/ the foundIndexChanged signal so that the number of
    // found nodes text in the UI gets updated minimally
    bool _numNodesFoundChanged = false;

    QQmlVariantListModel _graphTransformsModel;
    QStringList _graphTransforms;

    QQmlVariantListModel _visualisationsModel;
    QStringList _visualisations;

    bool _saveRequired = false;

    QByteArray _uiData;

    QByteArray _pluginUiData;
    int _pluginUiDataVersion = -1;

    std::unique_ptr<ExactNodePositions> _startingNodePositions;
    bool _userLayoutPaused = false; // true if the user wants the layout to pause

    bool _previousBusy = false;

    std::map<QString, NodeIdSet> _bookmarks;

    QStringList graphTransformConfigurationsFromUI() const;
    QStringList visualisationsFromUI() const;

    void maybeEmitBusyChanged();

    int foundIndex() const;
    int numNodesFound() const;
    void setFoundIt(std::vector<NodeId>::const_iterator foundIt);
    NodeId incrementFoundIt();
    NodeId decrementFoundIt();
    void selectAndFocusNode(NodeId nodeId);
    void selectAndFocusNodes(const std::vector<NodeId>& nodeIds);
    void selectAndFocusNodes(const NodeIdSet& nodeIds);
    void selectFoundNode(NodeId newFound);
    QList<QObject*> listEnrichmentTableModels();

    void setSaveRequired();

    int numNodesSelected() const;

    QStringList attributeGroupNames();

    void initialiseLayoutSettingsModel();

    QVariantMap transformParameter(const QString& transformName, const QString& parameterName) const;

signals:
    void applicationChanged();
    void graphQuickItemChanged();

    void uiDataChanged(const QByteArray& uiData);

    void pluginInstanceChanged();
    void pluginNameChanged();
    void pluginQmlPathChanged(const QByteArray& pluginUiData, int pluginUiDataVersion);

    void loadComplete(const QUrl& url, bool success);

    void titleChanged();
    void contrastingColorChanged();
    void statusChanged();

    void busyChanged();
    void editableChanged();

    void graphWillChange(const Graph* graph);
    void graphChanged(const Graph* graph, bool changeOccurred);
    void graphChangingChanged();

    void commandInProgressChanged();
    void commandProgressChanged();
    void commandVerbChanged();
    void commandIsCancellableChanged();
    void commandIsCancellingChanged();

    void layoutPauseStateChanged();

    void commandCompleted();
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

    void numNodesSelectedChanged();

    void bookmarksChanged();

    void taskAddedToExecutor();

    void enrichmentTableModelsChanged();
    void enrichmentAnalysisComplete();

    void saveComplete(bool success, QUrl fileUrl);
    void attributeGroupNamesChanged();

public:
    // Main QML interface
    Q_INVOKABLE bool openFile(const QUrl& fileUrl,
                              const QString& fileType,
                              QString pluginName,
                              const QVariantMap& parameters);

    Q_INVOKABLE void saveFile(const QUrl& fileUrl,
                              const QByteArray& uiData,
                              const QByteArray& pluginUiData);

    Q_INVOKABLE void onPreferenceChanged(const QString& key, const QVariant& value);

    Q_INVOKABLE void toggleLayout();

    Q_INVOKABLE bool nodeIsSelected(QmlNodeId nodeId) const;

    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void selectAllVisible();
    Q_INVOKABLE void selectNone();
    Q_INVOKABLE void selectSources();
    Q_INVOKABLE void selectSourcesOf(QmlNodeId nodeId);
    Q_INVOKABLE void selectTargets();
    Q_INVOKABLE void selectTargetsOf(QmlNodeId nodeId);
    Q_INVOKABLE void selectNeighbours();
    Q_INVOKABLE void selectNeighboursOf(QmlNodeId nodeId);
    Q_INVOKABLE void invertSelection();

    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();

    Q_INVOKABLE void deleteNode(QmlNodeId nodeId);
    Q_INVOKABLE void deleteSelectedNodes();

    Q_INVOKABLE void resetView();

    Q_INVOKABLE void switchToOverviewMode(bool doTransition = true);

    Q_INVOKABLE void gotoPrevComponent();
    Q_INVOKABLE void gotoNextComponent();

    Q_INVOKABLE void find(const QString& term, int options,
        const QStringList& attributeNames, int findSelectStyle);
    Q_INVOKABLE void resetFind();
    Q_INVOKABLE void selectFirstFound();
    Q_INVOKABLE void selectNextFound();
    Q_INVOKABLE void selectPrevFound();
    Q_INVOKABLE void updateFoundIndex(bool reselectIfInvalidated);

    Q_INVOKABLE void selectByAttributeValue(const QString& attributeName, const QString& value);

    Q_INVOKABLE QString nodeName(QmlNodeId nodeId) const;

    Q_INVOKABLE AvailableTransformsModel* availableTransforms() const;
    Q_INVOKABLE QVariantMap transform(const QString& transformName) const;
    Q_INVOKABLE QVariantMap findTransformParameter(const QString& transformName,
                                                   const QString& parameterName) const;
    Q_INVOKABLE bool hasTransformInfo() const;
    Q_INVOKABLE QVariantMap transformInfoAtIndex(int index) const;

    Q_INVOKABLE bool opIsUnary(const QString& op) const;

    Q_INVOKABLE AvailableAttributesModel* availableAttributes(int elementTypes = static_cast<int>(ElementType::All),
                                                              int valueTypes = static_cast<int>(ValueType::All)) const;
    Q_INVOKABLE QVariantMap attribute(const QString& attributeName) const;

    Q_INVOKABLE QVariantMap parseGraphTransform(const QString& transform) const;
    Q_INVOKABLE bool graphTransformIsValid(const QString& transform) const;
    Q_INVOKABLE void removeGraphTransform(int index);
    Q_INVOKABLE void moveGraphTransform(int from, int to);

    Q_INVOKABLE QStringList availableVisualisationChannelNames(int valueType) const;
    Q_INVOKABLE QString visualisationDescription(const QString& attributeName, const QString& channelName) const;
    Q_INVOKABLE bool hasVisualisationInfo() const;
    Q_INVOKABLE QVariantMap visualisationInfoAtIndex(int index) const;

    Q_INVOKABLE QVariantMap parseVisualisation(const QString& visualisation) const;
    Q_INVOKABLE QVariantMap visualisationDefaultParameters(int valueType,
                                                           const QString& channelName) const;
    Q_INVOKABLE bool visualisationIsValid(const QString& visualisation) const;
    Q_INVOKABLE void removeVisualisation(int index);
    Q_INVOKABLE void moveVisualisation(int from, int to);

    // Execute commands to apply transform and/or visualisation changes
    Q_INVOKABLE void update(QStringList newGraphTransforms = {},
                            QStringList newVisualisations = {});

    Q_INVOKABLE QVariantMap layoutSetting(const QString& name) const;
    Q_INVOKABLE void setLayoutSettingValue(const QString& name, float value);

    Q_INVOKABLE void cancelCommand();

    Q_INVOKABLE void writeTableViewToFile(QObject* tableView, const QUrl& fileUrl);

    Q_INVOKABLE void addBookmark(const QString& name);
    Q_INVOKABLE void removeBookmarks(const QStringList& names);
    Q_INVOKABLE void renameBookmark(const QString& from, const QString& to);
    Q_INVOKABLE void gotoBookmark(const QString& name);

    Q_INVOKABLE void dumpGraph();

    Q_INVOKABLE void performEnrichment(QStringList selectedAttributesAgainst, QString selectedAttribute);
private slots:
    void onLoadProgress(int percentage);
    void onLoadComplete(const QUrl& url, bool success);

    void onSelectionChanged(const SelectionManager* selectionManager);
    void onFoundNodeIdsChanged(const SearchManager* searchManager);

    void onGraphChanged(const Graph*, bool);
    void onMutableGraphChanged();

    void onPluginSaveRequired();

    void executeDeferred();
};

#endif // DOCUMENT_H
