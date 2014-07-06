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
#include <mutex>
#include <atomic>
#include <type_traits>

class CommandManager : public QObject
{
    Q_OBJECT
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

    const std::vector<QString> undoableCommandDescriptions() const;
    const std::vector<QString> redoableCommandDescriptions() const;

    const QString nextUndoAction() const;
    const QString nextRedoAction() const;

    bool busy() const;

private:
    void executeReal(std::shared_ptr<Command> command);

    bool canUndoNoLocking() const;
    bool canRedoNoLocking() const;

    std::deque<std::shared_ptr<Command>> _stack;
    int _lastExecutedIndex;

    mutable std::mutex _mutex;
    std::atomic<bool> _busy;

signals:
    void commandWillExecuteAsynchronously(const Command* command, const QString& verb) const;
    void commandProgress(const Command*, int progress) const;
    void commandCompleted(const Command* command, const QString& pastParticiple) const;
};

#endif // COMMANDMANAGER_H
