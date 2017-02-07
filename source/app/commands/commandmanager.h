#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include "shared/utils/utils.h"
#include "shared/utils/deferredexecutor.h"
#include "command.h"

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
#include <type_traits>

class CommandManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int commandProgress MEMBER _commandProgress NOTIFY commandProgressChanged)
    Q_PROPERTY(QString commandVerb MEMBER _commandVerb NOTIFY commandVerbChanged)

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)

public:
    CommandManager();
    virtual ~CommandManager();

    void clear();

    template<typename T> typename std::enable_if<std::is_base_of<ICommand, T>::value, void>::type execute(std::shared_ptr<T> command)
    {
        _deferredExecutor.enqueue([this, command] { executeReal(command, false); });
        emit commandQueued();
    }

    // Execute only once, i.e. so that it can't be undone
    template<typename T> typename std::enable_if<std::is_base_of<ICommand, T>::value, void>::type executeOnce(std::shared_ptr<T> command)
    {
        _deferredExecutor.enqueue([this, command] { executeReal(command, true); });
        emit commandQueued();
    }

    template<typename... Args> void execute(Args&&... args)
    {
        execute(std::make_shared<Command>(std::forward<Args>(args)...));
    }

    template<typename... Args> void executeOnce(Args&&... args)
    {
        executeOnce(std::make_shared<Command>(std::forward<Args>(args)...));
    }

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
