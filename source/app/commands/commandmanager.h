#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include "../utils/utils.h"
#include "../utils/deferredexecutor.h"
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

    template<typename T> typename std::enable_if<std::is_base_of<Command, T>::value, void>::type execute(std::shared_ptr<T> command)
    {
        QMetaObject::invokeMethod(this, "executeReal", Q_ARG(std::shared_ptr<Command>, command),
                                                       Q_ARG(bool, false));
    }

    template<typename... Args> void execute(Args&&... args)
    {
        QMetaObject::invokeMethod(this, "executeReal",
                                  Q_ARG(std::shared_ptr<Command>,
                                        std::make_shared<Command>(std::forward<Args>(args)...)),
                                  Q_ARG(bool, false));
    }

    // Execute only once, i.e. so that it can't be undone
    template<typename... Args> void executeOnce(Args&&... args)
    {
        QMetaObject::invokeMethod(this, "executeReal",
                                  Q_ARG(std::shared_ptr<Command>,
                                        std::make_shared<Command>(std::forward<Args>(args)...)),
                                  Q_ARG(bool, true));
    }

    template<typename... Args> void executeSynchronous(Args&&... args)
    {
        QMetaObject::invokeMethod(this, "executeReal",
                                  Q_ARG(std::shared_ptr<Command>,
                                        std::make_shared<Command>(std::forward<Args>(args)..., false)),
                                  Q_ARG(bool, false));
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

private:
    template<typename... Args> void executeAsynchronous(std::shared_ptr<Command> command, QString verb, Args&&... args)
    {
        _currentCommand = command;
        _commandProgress = -1;
        _commandVerb = verb;
        emit commandProgressChanged();
        emit commandVerbChanged();
        emit commandWillExecuteAsynchronously(command.get());
        _thread = std::thread(std::forward<Args>(args)...);
    }

    bool canUndoNoLocking() const;
    bool canRedoNoLocking() const;

    std::deque<std::shared_ptr<Command>> _stack;
    int _lastExecutedIndex = -1;

    std::thread _thread;
    mutable std::mutex _mutex;
    std::unique_lock<std::mutex> _lock;
    std::atomic<bool> _busy;

    std::shared_ptr<Command> _currentCommand;
    int _commandProgress = 0;
    QString _commandVerb;

    DeferredExecutor _deferredExecutor;

    int _debug = 0;

private slots:
    void executeReal(std::shared_ptr<Command> command, bool irreversible);

    void undoReal();
    void redoReal();

    void onCommandCompleted(Command* command, const QString& pastParticiple, CommandAction action);

signals:
    void commandWillExecuteAsynchronously(const Command* command) const;
    void commandProgressChanged() const;
    void commandVerbChanged() const;
    void commandCompleted(Command* command, const QString& pastParticiple, CommandAction action) const;

    void busyChanged() const;
};

#endif // COMMANDMANAGER_H
