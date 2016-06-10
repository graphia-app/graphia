#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "graphtransformconfiguration.h"

#include "../graph/graph.h"
#include "../loading/parserthread.h"
#include "../commands/commandmanager.h"
#include "../layout/layout.h"

#include "shared/interfaces/iplugin.h"

#include "../utils/qmlenum.h"
#include "../utils/qmlcontainerwrapper.h"

#include <QQuickItem>
#include <QString>
#include <QUrl>

#include <vector>
#include <memory>
#include <mutex>

class Application;
class GraphQuickItem;
class GraphModel;
class SelectionManager;

DEFINE_QML_ENUM(Application::uri(), Application::majorVersion(), Application::minorVersion(),
                LayoutPauseState,
                Running, RunningFinished, Paused);

class Document : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Application* application MEMBER _application NOTIFY applicationChanged)
    Q_PROPERTY(GraphQuickItem* graph MEMBER _graphQuickItem NOTIFY graphQuickItemChanged)
    Q_PROPERTY(QString contentQmlPath READ contentQmlPath NOTIFY contentQmlPathChanged)

    Q_PROPERTY(QString title MEMBER _title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString status MEMBER _status WRITE setStatus NOTIFY statusChanged)

    Q_PROPERTY(bool idle READ idle NOTIFY idleChanged)
    Q_PROPERTY(bool canDelete READ canDelete NOTIFY canDeleteChanged)

    Q_PROPERTY(bool commandInProgress READ commandInProgress NOTIFY commandInProgressChanged)
    Q_PROPERTY(int commandProgress READ commandProgress NOTIFY commandProgressChanged)
    Q_PROPERTY(QString commandVerb READ commandVerb NOTIFY commandVerbChanged)

    Q_PROPERTY(LayoutPauseState::Enum layoutPauseState READ layoutPauseState NOTIFY layoutPauseStateChanged)

    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(QString nextUndoAction READ nextUndoAction NOTIFY nextUndoActionChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    Q_PROPERTY(QString nextRedoAction READ nextRedoAction NOTIFY nextRedoActionChanged)

    Q_PROPERTY(bool canResetView READ canResetView NOTIFY canResetViewChanged)
    Q_PROPERTY(bool canEnterOverviewMode READ canEnterOverviewMode NOTIFY canEnterOverviewModeChanged)

    Q_PROPERTY(QmlContainerWrapperBase* transforms READ transforms CONSTANT)

    Q_PROPERTY(bool debugPauserEnabled READ debugPauserEnabled NOTIFY debugPauserEnabledChanged)
    Q_PROPERTY(bool debugPaused READ debugPaused NOTIFY debugPausedChanged)
    Q_PROPERTY(QString debugResumeAction READ debugResumeAction NOTIFY debugResumeActionChanged)

    Q_PROPERTY(QmlContainerWrapperBase* layoutSettings READ layoutSettings CONSTANT)

    Q_PROPERTY(float fps READ fps NOTIFY fpsChanged)

public:
    explicit Document(QObject* parent = nullptr);

    bool commandInProgress() const;
    bool idle() const;
    bool canDelete() const;

    int commandProgress() const;
    QString commandVerb() const;

    void updateLayoutState();
    LayoutPauseState::Enum layoutPauseState();

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

    QmlContainerWrapper<GraphTransformConfiguration>* transforms() { return &_graphTransformConfigurations; }

    QmlContainerWrapper<LayoutSetting>* layoutSettings() { return &_layoutSettings; }

    float fps() const;

    QString contentQmlPath() const;

private:
    Application* _application = nullptr;
    GraphQuickItem* _graphQuickItem = nullptr;

    QString _title;
    QString _status;

    int _loadProgress = 0;
    bool _loadComplete = false;

    std::shared_ptr<GraphModel> _graphModel;
    std::unique_ptr<IPluginInstance> _pluginInstance;
    std::shared_ptr<SelectionManager> _selectionManager;
    CommandManager _commandManager;
    std::unique_ptr<ParserThread> _graphFileParserThread;
    std::unique_ptr<LayoutThread> _layoutThread;
    QmlContainerWrapper<LayoutSetting> _layoutSettings;

    std::vector<GraphTransformConfiguration> _previousGraphTransformConfigurations;
    QmlContainerWrapper<GraphTransformConfiguration> _graphTransformConfigurations;

    bool _userLayoutPaused = false; // true if the user wants the layout to pause

    bool _previousIdle = true;

    template<typename... Args>
    void addGraphTransform(Args&&... args)
    {
        auto graphTransformConfigurations = _graphTransformConfigurations.vector();
        graphTransformConfigurations.emplace_back(this, std::forward<Args>(args)...);
        _graphTransformConfigurations.setVector(graphTransformConfigurations);
    }

    std::vector<GraphTransformConfiguration> transformsWithEmptyAppended(
            const std::vector<GraphTransformConfiguration>& graphTransformConfigurations);
    void applyTransforms();

    void maybeEmitIdleChanged();

private slots:
    void onGraphTransformsConfigurationDataChanged(const QModelIndex& index, const QModelIndex&,
                                                   const QVector<int>& roles);

signals:
    void applicationChanged();
    void graphQuickItemChanged();
    void contentQmlPathChanged();

    void titleChanged();
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

public slots:
    bool openFile(const QUrl& fileUrl, const QString& fileType);

    void toggleLayout();

    void selectAll();
    void selectNone();
    void invertSelection();

    void undo();
    void redo();

    void deleteSelectedNodes();

    void resetView();

    void switchToOverviewMode(bool doTransition = true);

    void toggleDebugPauser();
    void debugResume();

    QStringList availableTransformNames() const;
    QStringList availableDataFields(const QString& transformName) const;
    const DataField& dataFieldByName(const QString& dataFieldName) const;
    DataFieldType typeOfDataField(const QString& dataFieldName) const;
    QStringList avaliableConditionFnOps(const QString& dataFieldName) const;

    void removeGraphTransform(int index);

    void dumpGraph();

private slots:
    void onLoadProgress(int percentage);
    void onLoadComplete(bool success);
};

#endif // DOCUMENT_H
