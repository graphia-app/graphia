#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include "shared/commands/icommandmanager.h"

#include <QtGlobal>
#include <QObject>
#include <QString>

#include <functional>
#include <deque>
#include <vector>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>

class CommandManager : public QObject, public ICommandManager
{
    Q_OBJECT

    Q_PROPERTY(int commandProgress MEMBER _commandProgress NOTIFY commandProgressChanged)
    Q_PROPERTY(QString commandVerb MEMBER _commandVerb NOTIFY commandVerbChanged)
    Q_PROPERTY(bool commandIsCancellable READ commandIsCancellable NOTIFY commandIsCancellableChanged)

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
    CommandManager();
    ~CommandManager() override;

    void clear();

    void execute(std::unique_ptr<ICommand> command) override;
    using ICommandManager::execute;
    void executeOnce(std::unique_ptr<ICommand> command) override;
    using ICommandManager::executeOnce;

    void undo();
    void redo();

    bool canUndo() const;
    bool canRedo() const;

    int commandProgress() const { return _commandProgress; }
    QString commandVerb() const { return _commandVerb; }
    bool commandIsCancellable() const;

    const std::vector<QString> undoableCommandDescriptions() const;
    const std::vector<QString> redoableCommandDescriptions() const;

    QString nextUndoAction() const;
    QString nextRedoAction() const;

    bool busy() const;

    void clearCommandStack();

    void cancel();

private:
    template<typename Fn> void doCommand(ICommand* command,
                                         const QString& verb, Fn&& fn)
    {
        _currentCommand = command;
        _commandProgress = -1;
        _commandVerb = verb;
        emit commandProgressChanged();
        emit commandVerbChanged();
        emit commandIsCancellableChanged();
        emit commandWillExecute(command);

        _thread = std::thread(std::forward<Fn>(fn));

        command->initialise();
        _commandProgressTimerId = startTimer(200);
    }

    void clearCurrentCommand();

    void timerEvent(QTimerEvent *event) override;

    bool canUndoNoLocking() const;
    bool canRedoNoLocking() const;

    void executeReal(std::unique_ptr<ICommand> command, bool irreversible);
    void undoReal();
    void redoReal();

    void clearCommandStackNoLocking();

    enum class CommandAction
    {
        Execute,
        ExecuteOnce,
        Undo,
        Redo
    };

    struct PendingCommand
    {
        PendingCommand(CommandAction action, std::unique_ptr<ICommand>&& command) :
            _action(action), _command(std::move(command)) {}

        PendingCommand(CommandAction action) :
            _action(action), _command(nullptr) {}

        CommandAction _action;
        std::unique_ptr<ICommand> _command;
    };

    std::deque<PendingCommand> _pendingCommands;
    std::deque<std::unique_ptr<ICommand>> _stack;
    int _lastExecutedIndex = -1;

    std::thread _thread;
    mutable std::mutex _mutex;
    std::unique_lock<std::mutex> _lock;
    std::atomic<bool> _busy;
    std::atomic<bool> _graphChanged;

    mutable std::mutex _currentCommandMutex;
    ICommand* _currentCommand = nullptr;
    int _commandProgressTimerId = -1;
    int _commandProgress = 0;
    QString _commandVerb;

    int _debug = 0;

private slots:
    void onCommandCompleted(bool success, QString description, QString pastParticiple);
    void update();

public slots:
    void onGraphChanged() { _graphChanged = true; }

signals:
    void commandWillExecute(const ICommand* command) const;
    void commandProgressChanged() const;
    void commandVerbChanged() const;
    void commandIsCancellableChanged() const;
    void commandQueued();
    void commandCompleted(bool success, QString description, QString pastParticiple) const;
    void commandStackCleared();

    void busyChanged() const;
};

#endif // COMMANDMANAGER_H
