#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QQuickItem>
#include <QString>
#include <QUrl>

#include "../graph/graph.h"
#include "../loading/graphfileparser.h"
#include "../commands/commandmanager.h"
#include "../layout/layout.h"

#include <memory>
#include <mutex>

class Application;
class GraphQuickItem;
class GraphModel;
class SelectionManager;

class Document : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Application* application MEMBER _application NOTIFY applicationChanged)
    Q_PROPERTY(GraphQuickItem* graph MEMBER _graphQuickItem NOTIFY graphQuickItemChanged)

    Q_PROPERTY(QString title MEMBER _title WRITE setTitle NOTIFY titleChanged)
    Q_PROPERTY(QString status MEMBER _status WRITE setStatus NOTIFY statusChanged)

    Q_PROPERTY(bool idle READ idle NOTIFY idleChanged)
    Q_PROPERTY(bool canDelete READ canDelete NOTIFY canDeleteChanged)

    Q_PROPERTY(bool commandInProgress READ commandInProgress NOTIFY commandInProgressChanged)
    Q_PROPERTY(int commandProgress READ commandProgress NOTIFY commandProgressChanged)
    Q_PROPERTY(QString commandVerb READ commandVerb NOTIFY commandVerbChanged)

    Q_PROPERTY(bool layoutIsPaused READ layoutIsPaused NOTIFY layoutIsPausedChanged)

    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged)
    Q_PROPERTY(QString nextUndoAction READ nextUndoAction NOTIFY nextUndoActionChanged)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged)
    Q_PROPERTY(QString nextRedoAction READ nextRedoAction NOTIFY nextRedoActionChanged)

    Q_PROPERTY(bool canResetView READ canResetView NOTIFY canResetViewChanged)
    Q_PROPERTY(bool canEnterOverviewMode READ canEnterOverviewMode NOTIFY canEnterOverviewModeChanged)

    Q_PROPERTY(bool debugPauserEnabled READ debugPauserEnabled NOTIFY debugPauserEnabledChanged)
    Q_PROPERTY(bool debugPaused READ debugPaused NOTIFY debugPausedChanged)
    Q_PROPERTY(QString debugResumeAction READ debugResumeAction NOTIFY debugResumeActionChanged)

public:
    explicit Document(QObject* parent = nullptr);

    bool commandInProgress() const;
    bool idle() const;
    bool canDelete() const;

    int commandProgress() const;
    QString commandVerb() const;

    void pauseLayout(bool autoResume = false);
    bool layoutIsPaused();
    void resumeLayout(bool autoResume = false);

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

private:
    Application* _application;
    GraphQuickItem* _graphQuickItem;

    QString _title;
    QString _status;

    int _loadProgress = 0;
    bool _loadComplete = false;

    std::shared_ptr<GraphModel> _graphModel;
    std::shared_ptr<SelectionManager> _selectionManager;
    CommandManager _commandManager;
    std::unique_ptr<GraphFileParserThread> _graphFileParserThread;
    std::unique_ptr<LayoutThread> _layoutThread;

    std::recursive_mutex _autoResumeMutex;
    int _autoResume = 0;

signals:
    void applicationChanged();
    void graphQuickItemChanged();

    void titleChanged();
    void statusChanged();

    void idleChanged();
    void canDeleteChanged();

    void commandInProgressChanged();
    void commandProgressChanged();
    void commandVerbChanged();

    void layoutIsPausedChanged();

    void canUndoChanged();
    void nextUndoActionChanged();
    void canRedoChanged();
    void nextRedoActionChanged();

    void canResetViewChanged();
    void canEnterOverviewModeChanged();

    void debugPauserEnabledChanged();
    void debugPausedChanged();
    void debugResumeActionChanged();

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

private slots:
    void onGraphWillChange(const Graph*);
    void onGraphChanged(const Graph* graph);

    void onLoadProgress(int percentage);
    void onLoadComplete(bool success);
};

#endif // DOCUMENT_H
