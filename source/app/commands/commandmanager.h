#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include "shared/commands/icommandmanager.h"
#include "shared/utils/fatalerror.h"

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
    Q_PROPERTY(bool commandIsCancelling MEMBER _cancelling NOTIFY commandIsCancellingChanged)

public:
    CommandManager();
    ~CommandManager() override;

    void clear();

    void execute(ICommandPtr command) override;
    using ICommandManager::execute;
    void executeOnce(ICommandPtr command) override;
    using ICommandManager::executeOnce;

    void undo();
    void redo();

    bool canUndo() const;
    bool canRedo() const;

    int commandProgress() const { return _commandProgress; }
    QString commandVerb() const { return _commandVerb; }
    bool commandIsCancellable() const;
    bool commandIsCancelling() const { return _cancelling; }

    std::vector<QString> undoableCommandDescriptions() const;
    std::vector<QString> redoableCommandDescriptions() const;

    QString nextUndoAction() const;
    QString nextRedoAction() const;

    bool busy() const;

    void clearCommandStack();

    void cancel();

    void wait();

private:
    template<typename Fn> void doCommand(ICommand* command,
                                         const QString& verb, Fn&& fn)
    {
        command->initialise();

        _currentCommand = command;
        _commandProgress = -1;
        _commandVerb = verb;
        emit commandProgressChanged();
        emit commandVerbChanged();
        emit commandIsCancellableChanged();

        _commandProgressTimerId = startTimer(200);

        emit commandWillExecute(command);

        // If the command thread is still active, we shouldn't be here
        Q_ASSERT(!_thread.joinable());

        _thread = std::thread(std::forward<Fn>(fn));

        if(!_busy)
        {
            _busy = true;
            emit started();
        }
    }

    void clearCurrentCommand();

    void timerEvent(QTimerEvent *event) override;

    bool canUndoNoLocking() const;
    bool canRedoNoLocking() const;

    void executeReal(ICommandPtr command, bool irreversible);
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
        PendingCommand(CommandAction action, ICommandPtr command) :
            _action(action), _command(std::move(command))
        {
            if((action == CommandAction::Execute || action == CommandAction::ExecuteOnce) &&
                _command == nullptr)
            {
                FATAL_ERROR(NullPendingCommand);
            }
        }

        explicit PendingCommand(CommandAction action) :
            _action(action), _command(nullptr) {}

        PendingCommand() = default;

        CommandAction _action = CommandAction::Execute;
        ICommandPtr _command = nullptr;
    };

    bool commandsArePending() const;
    PendingCommand nextPendingCommand();

    std::deque<PendingCommand> _pendingCommands;
    std::deque<ICommandPtr> _stack;
    int _lastExecutedIndex = -1;

    std::thread _thread;
    mutable std::recursive_mutex _mutex;
    mutable std::recursive_mutex _queueMutex;

    bool _busy = false;
    std::atomic<bool> _graphChanged;

    mutable std::mutex _currentCommandMutex;
    ICommand* _currentCommand = nullptr;
    int _commandProgressTimerId = -1;
    int _commandProgress = 0;
    QString _commandVerb;
    bool _cancelling = false;

    int _debug = 0;

private slots:
    void onCommandCompleted(bool success, const QString& description, const QString& pastParticiple);
    void update();

public slots:
    void onGraphChanged() { _graphChanged = true; }

signals:
    void started();

    void commandWillExecute(const ICommand* command) const;
    void commandProgressChanged() const;
    void commandVerbChanged() const;
    void commandIsCancellableChanged() const;
    void commandIsCancellingChanged() const;
    void commandQueued();
    void commandCompleted(bool success, const QString& description, const QString& pastParticiple) const;
    void commandStackCleared();

    void finished();
};

#endif // COMMANDMANAGER_H
