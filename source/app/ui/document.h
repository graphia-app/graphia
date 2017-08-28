#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "shared/plugins/iplugin.h"
#include "rendering/compute/gpucomputethread.h"
#include "graph/graph.h"
#include "loading/parserthread.h"
#include "commands/commandmanager.h"
#include "layout/layout.h"
#include "shared/utils/qmlenum.h"
#include "attributes/availableattributesmodel.h"
#include "transform/availabletransformsmodel.h"
#include "shared/utils/deferredexecutor.h"
#include "shared/utils/semaphore.h"
#include "thirdparty/qt-qml-models/QQmlVariantListModel.h"

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

class Application;
class GraphQuickItem;
class GraphModel;
class SelectionManager;
class SearchManager;

DEFINE_QML_ENUM(Q_GADGET, LayoutPauseState,
                Running, RunningFinished, Paused);

class Document : public QObject
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

    Q_PROPERTY(bool idle READ idle NOTIFY idleChanged)
    Q_PROPERTY(bool canDelete READ canDelete NOTIFY canDeleteChanged)

    Q_PROPERTY(bool commandInProgress READ commandInProgress NOTIFY commandInProgressChanged)
    Q_PROPERTY(int commandProgress READ commandProgress NOTIFY commandProgressChanged)
    Q_PROPERTY(QString commandVerb READ commandVerb NOTIFY commandVerbChanged)
    Q_PROPERTY(bool commandIsCancellable READ commandIsCancellable NOTIFY commandIsCancellableChanged)

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

public:
    explicit Document(QObject* parent = nullptr);
    ~Document();

    const GraphModel* graphModel() const { return _graphModel.get(); }
    GraphModel* graphModel() { return _graphModel.get(); }

    static QColor contrastingColorForBackground();

    bool commandInProgress() const;
    bool idle() const;
    bool canDelete() const;

    int commandProgress() const;
    QString commandVerb() const;
    bool commandIsCancellable() const;

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

    void executeOnMainThread(DeferredExecutor::TaskFn task,
                             const QString& description);

    void executeOnMainThreadAndWait(DeferredExecutor::TaskFn task,
                                    const QString& description);

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

    std::vector<NodeId> _foundNodeIds;
    bool _foundItValid = false;
    std::vector<NodeId>::const_iterator _foundIt = _foundNodeIds.begin();

    QQmlVariantListModel _graphTransformsModel;
    QStringList _graphTransforms;

    QQmlVariantListModel _visualisationsModel;
    QStringList _visualisations;

    bool _saveRequired = false;

    QByteArray _uiData;
    int _uiDataVersion = -1;

    std::unique_ptr<ExactNodePositions> _startingNodePositions;
    bool _userLayoutPaused = false; // true if the user wants the layout to pause

    bool _previousIdle = true;

    QStringList graphTransformConfigurationsFromUI() const;
    QStringList visualisationsFromUI() const;

    void maybeEmitIdleChanged();

    int foundIndex() const;
    int numNodesFound() const;
    void setFoundIt(std::vector<NodeId>::const_iterator it);
    NodeId incrementFoundIt();
    NodeId decrementFoundIt();
    void selectFoundNode(NodeId newFound);

    void setSaveRequired();

    void initialiseLayoutSettingsModel();

    QVariantMap transformParameter(const QString& transformName, const QString& parameterName) const;

signals:
    void applicationChanged();
    void graphQuickItemChanged();

    void pluginInstanceChanged();
    void pluginNameChanged();
    void pluginQmlPathChanged(const QByteArray& uiData, int uiDataVersion);

    void loadComplete(const QUrl& url, bool success);

    void titleChanged();
    void contrastingColorChanged();
    void statusChanged();

    void idleChanged();
    void canDeleteChanged();

    void commandInProgressChanged();
    void commandProgressChanged();
    void commandVerbChanged();
    void commandIsCancellableChanged();

    void layoutPauseStateChanged();

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

    void taskAddedToExecutor();

    void saveComplete(bool success, QUrl fileUrl);

public:
    // Main QML interface
    Q_INVOKABLE bool openFile(const QUrl& fileUrl,
                              const QString& fileType,
                              QString pluginName,
                              const QVariantMap& parameters);

    Q_INVOKABLE void saveFile(const QUrl& fileUrl, const QByteArray& uiData);

    Q_INVOKABLE void onPreferenceChanged(const QString& key, const QVariant& value);

    Q_INVOKABLE void toggleLayout();

    Q_INVOKABLE void selectAll();
    Q_INVOKABLE void selectAllVisible();
    Q_INVOKABLE void selectNone();
    Q_INVOKABLE void selectNeighbours();
    Q_INVOKABLE void invertSelection();

    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();

    Q_INVOKABLE void deleteSelectedNodes();

    Q_INVOKABLE void resetView();

    Q_INVOKABLE void switchToOverviewMode(bool doTransition = true);

    Q_INVOKABLE void gotoPrevComponent();
    Q_INVOKABLE void gotoNextComponent();

    Q_INVOKABLE void find(const QString& regex);
    Q_INVOKABLE void selectFirstFound();
    Q_INVOKABLE void selectNextFound();
    Q_INVOKABLE void selectPrevFound();
    Q_INVOKABLE void selectAllFound();
    Q_INVOKABLE void updateFoundIndex(bool reselectIfInvalidated);

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

    Q_INVOKABLE void dumpGraph();

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
