#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include "shared/commands/icommandmanager.h"

#include "shared/utils/utils.h"
#include "shared/utils/deferredexecutor.h"

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

enum class CommandAction
{
    None,
    Execute,
    Undo,
    Redo
};

class CommandManager : public QObject, public ICommandManager
{
    Q_OBJECT

    Q_PROPERTY(int commandProgress MEMBER _commandProgress NOTIFY commandProgressChanged)
    Q_PROPERTY(QString commandVerb MEMBER _commandVerb NOTIFY commandVerbChanged)

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
    CommandManager();
    virtual ~CommandManager();

    void clear();

    void execute(const std::shared_ptr<ICommand>& command);
    using ICommandManager::execute;
    void executeOnce(const std::shared_ptr<ICommand>& command);
    using ICommandManager::executeOnce;

    void undo();
    void redo();

    bool canUndo() const;
    bool canRedo() const;

    int commandProgress() const { return _commandProgress; }
    QString commandVerb() const { return _commandVerb; }

    const std::vector<QString> undoableCommandDescriptions() const;
    const std::vector<QString> redoableCommandDescriptions() const;

    QString nextUndoAction() const;
    QString nextRedoAction() const;

    bool busy() const;

    void clearCommandStack();

private:
    template<typename Fn> void doCommand(std::shared_ptr<ICommand> command,
                                         const QString& verb, const Fn& fn)
    {
        _currentCommand = command;
        _commandProgress = -1;
        _commandVerb = verb;
        emit commandProgressChanged();
        emit commandVerbChanged();
        emit commandWillExecute(command.get());

        _thread = std::thread(fn);

        command->setProgress(-1);
        _commandProgressTimerId = startTimer(200);
    }

    void timerEvent(QTimerEvent *event);

    bool canUndoNoLocking() const;
    bool canRedoNoLocking() const;

    void executeReal(std::shared_ptr<ICommand> command, bool irreversible);
    void undoReal();
    void redoReal();

    std::deque<std::shared_ptr<ICommand>> _stack;
    int _lastExecutedIndex = -1;

    std::thread _thread;
    mutable std::mutex _mutex;
    std::unique_lock<std::mutex> _lock;
    std::atomic<bool> _busy;

    std::shared_ptr<ICommand> _currentCommand;
    int _commandProgressTimerId = -1;
    int _commandProgress = 0;
    QString _commandVerb;

    DeferredExecutor _deferredExecutor;

    int _debug = 0;

private slots:
    void onCommandCompleted(ICommand* command, const QString& pastParticiple, CommandAction action);
    void update();

signals:
    void commandWillExecute(const ICommand* command) const;
    void commandProgressChanged() const;
    void commandVerbChanged() const;
    void commandQueued();
    void commandCompleted(ICommand* command, const QString& pastParticiple, CommandAction action) const;

    void busyChanged() const;
};

#endif // COMMANDMANAGER_H
