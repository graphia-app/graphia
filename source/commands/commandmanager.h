#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include "../utils/utils.h"
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

    void clear();

    template<typename T> typename std::enable_if<std::is_base_of<Command, T>::value, void>::type execute(std::shared_ptr<T> command)
    {
        executeReal(command);
    }

    template<typename... Args> void execute(Args&&... args)
    {
        executeReal(std::make_shared<Command>(std::forward<Args>(args)...));
    }

    template<typename... Args> void executeSynchronous(Args&&... args)
    {
        executeReal(std::make_shared<Command>(std::forward<Args>(args)..., false));
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
    void executeReal(std::shared_ptr<Command> command);

    bool canUndoNoLocking() const;
    bool canRedoNoLocking() const;

    std::deque<std::shared_ptr<Command>> _stack;
    int _lastExecutedIndex;

    std::thread _thread;
    mutable std::mutex _mutex;
    std::atomic<bool> _busy;

    int _commandProgress;
    QString _commandVerb;

    void onCommandCompleted(const Command* command, const QString& pastParticiple);

signals:
    void commandWillExecuteAsynchronously() const;
    void commandProgressChanged() const;
    void commandVerbChanged() const;
    void commandCompleted(const Command* command, const QString& pastParticiple) const;

    void busyChanged() const;
};

#endif // COMMANDMANAGER_H
