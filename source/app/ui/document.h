#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "shared/plugins/iplugin.h"
#include "rendering/compute/gpucomputethread.h"
#include "graph/graph.h"
#include "loading/parserthread.h"
#include "commands/commandmanager.h"
#include "layout/layout.h"
#include "utils/qmlenum.h"
#include "utils/qmlcontainerwrapper.h"
#include "thirdparty/qt-qml-models/QQmlVariantListModel.h"

#include <QQuickItem>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariantMap>

#include <vector>
#include <memory>
#include <mutex>

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
    Q_PROPERTY(QString pluginQmlPath READ pluginQmlPath NOTIFY pluginQmlPathChanged)

    Q_PROPERTY(QColor contrastingColor READ contrastingColorForBackground NOTIFY contrastingColorChanged)

    Q_PROPERTY(QString title MEMBER _title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString status MEMBER _status WRITE setStatus NOTIFY statusChanged)

    Q_PROPERTY(bool idle READ idle NOTIFY idleChanged)
    Q_PROPERTY(bool canDelete READ canDelete NOTIFY canDeleteChanged)

    Q_PROPERTY(bool commandInProgress READ commandInProgress NOTIFY commandInProgressChanged)
    Q_PROPERTY(int commandProgress READ commandProgress NOTIFY commandProgressChanged)
    Q_PROPERTY(QString commandVerb READ commandVerb NOTIFY commandVerbChanged)

    Q_PROPERTY(QML_ENUM_PROPERTY(LayoutPauseState) layoutPauseState READ layoutPauseState NOTIFY layoutPauseStateChanged)

    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(QString nextUndoAction READ nextUndoAction NOTIFY nextUndoActionChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    Q_PROPERTY(QString nextRedoAction READ nextRedoAction NOTIFY nextRedoActionChanged)

    Q_PROPERTY(bool canResetView READ canResetView NOTIFY canResetViewChanged)
    Q_PROPERTY(bool canEnterOverviewMode READ canEnterOverviewMode NOTIFY canEnterOverviewModeChanged)

    Q_PROPERTY(QQmlVariantListModel* transforms READ transformsModel CONSTANT)

    Q_PROPERTY(bool debugPauserEnabled READ debugPauserEnabled NOTIFY debugPauserEnabledChanged)
    Q_PROPERTY(bool debugPaused READ debugPaused NOTIFY debugPausedChanged)
    Q_PROPERTY(QString debugResumeAction READ debugResumeAction NOTIFY debugResumeActionChanged)

    Q_PROPERTY(QmlContainerWrapperBase* layoutSettings READ layoutSettings CONSTANT)

    Q_PROPERTY(float fps READ fps NOTIFY fpsChanged)

    Q_PROPERTY(int foundIndex READ foundIndex NOTIFY foundIndexChanged)
    Q_PROPERTY(int numNodesFound READ numNodesFound NOTIFY numNodesFoundChanged)

public:
    explicit Document(QObject* parent = nullptr);
    ~Document();

    static QColor contrastingColorForBackground();

    bool commandInProgress() const;
    bool idle() const;
    bool canDelete() const;

    int commandProgress() const;
    QString commandVerb() const;

    void updateLayoutState();
    LayoutPauseState layoutPauseState();

    bool canUndo() const;
    QString nextUndoAction() const;
    bool canRedo() const;
    QString nextRedoAction() const;

    bool canResetView() const;
    bool canEnterOverviewMode() const;

    bool debugPauserEnabled() const;
    bool debugPaused() const;
    QString debugResumeAction() const;

    void setTitle(const QString& status);
    void setStatus(const QString& status);

    QQmlVariantListModel* transformsModel() { return &_graphTransformsModel; }
    void setTransforms(const QStringList& transforms);

    QmlContainerWrapper<LayoutSetting>* layoutSettings() { return &_layoutSettings; }

    float fps() const;

    QObject* pluginInstance();
    QString pluginQmlPath() const;

private:
    Application* _application = nullptr;
    GraphQuickItem* _graphQuickItem = nullptr;

    QString _title;
    QString _status;

    int _loadProgress = 0;
    bool _loadComplete = false;

    std::shared_ptr<GraphModel> _graphModel;
    std::shared_ptr<GPUComputeThread> _gpuComputeThread;
    std::unique_ptr<IPluginInstance> _pluginInstance;
    std::shared_ptr<SelectionManager> _selectionManager;
    std::shared_ptr<SearchManager> _searchManager;
    CommandManager _commandManager;
    std::unique_ptr<ParserThread> _graphFileParserThread;

    std::unique_ptr<LayoutThread> _layoutThread;
    QmlContainerWrapper<LayoutSetting> _layoutSettings;

    std::vector<NodeId> _foundNodeIds;
    bool _foundItValid = false;
    std::vector<NodeId>::const_iterator _foundIt = _foundNodeIds.begin();

    QQmlVariantListModel _graphTransformsModel;
    QStringList _graphTransforms;

    bool _userLayoutPaused = false; // true if the user wants the layout to pause

    bool _previousIdle = true;

    QStringList graphTransformConfigurationsFromUI() const;

    void maybeEmitIdleChanged();

    int foundIndex() const;
    int numNodesFound() const;
    void setFoundIt(std::vector<NodeId>::const_iterator it);
    void incrementFoundIt();
    void decrementFoundIt();

signals:
    void applicationChanged();
    void graphQuickItemChanged();
    void pluginInstanceChanged();
    void pluginQmlPathChanged();


    void titleChanged();
    void contrastingColorChanged();
    void statusChanged();

    void idleChanged();
    void canDeleteChanged();

    void commandInProgressChanged();
    void commandProgressChanged();
    void commandVerbChanged();

    void layoutPauseStateChanged();

    void canUndoChanged();
    void nextUndoActionChanged();
    void canRedoChanged();
    void nextRedoActionChanged();

    void canResetViewChanged();
    void canEnterOverviewModeChanged();

    void debugPauserEnabledChanged();
    void debugPausedChanged();
    void debugResumeActionChanged();

    void fpsChanged();

    void foundIndexChanged();
    void numNodesFoundChanged();

public slots:
    bool openFile(const QUrl& fileUrl,
                  const QString& fileType,
                  const QString& pluginName,
                  const QVariantMap& settings);

    void onPreferenceChanged(const QString& key, const QVariant& value);

    void toggleLayout();

    void selectAll();
    void selectNone();
    void invertSelection();

    void undo();
    void redo();

    void deleteSelectedNodes();

    void resetView();

    void switchToOverviewMode(bool doTransition = true);

    void gotoPrevComponent();
    void gotoNextComponent();

    void find(const QString& regex);
    void selectFirstFound();
    void selectNextFound();
    void selectPrevFound();
    void selectAllFound();
    void updateFoundIndex(bool reselectIfInvalidated);

    void toggleDebugPauser();
    void debugResume();

    QStringList availableTransformNames() const;
    QStringList availableDataFields(const QString& transformName) const;
    QStringList avaliableConditionFnOps(const QString& dataFieldName) const;

    QVariantMap findTransformParameter(const QString& transformName,
                                       const QString& parameterName) const;

    QVariantMap parseGraphTransform(const QString& transform) const;
    bool graphTransformIsValid(const QString& transform) const;
    void appendGraphTransform(const QString& transform);
    void removeGraphTransform(int index);
    void updateGraphTransforms();

    void dumpGraph();

private slots:
    void onLoadProgress(int percentage);
    void onLoadComplete(bool success);

    void onSelectionChanged(const SelectionManager* selectionManager);
    void onFoundNodeIdsChanged(const SearchManager* searchManager);
};

#endif // DOCUMENT_H
