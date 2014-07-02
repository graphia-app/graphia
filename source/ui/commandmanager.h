#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include "../utils/utils.h"

#include <QtGlobal>
#include <QObject>
#include <QString>

#include <functional>
#include <deque>
#include <vector>
#include <memory>
#include <mutex>

using ProgressFn = std::function<void(int)>;

// For simple operations the Command class can be used directly, by passing
// it lambdas to perform the execute and undo actions. For more complicated
// operations that require maintaining non-trivial state, Command should be
// subclassed and the execute and undo functions overridden.
class Command : public QObject
{
    friend class CommandManager;

    Q_OBJECT
public:
    Command(const QString& description,
            std::function<bool(ProgressFn)> executeFunction =
            [](ProgressFn)
            {
                Q_ASSERT(!"executeFunction not implmented");
                return false;
            },
            std::function<void(ProgressFn)> undoFunction =
            [](ProgressFn)
            {
                Q_ASSERT(!"undoFunction not implemented");
            }, bool asynchronous = false);

    const QString& description() const;
    const QString& undoDescription() const;
    const QString& redoDescription() const;
    bool asynchronous() const { return _asynchronous; }

private:
    // Return false if the command failed, or did nothing
    virtual bool execute(ProgressFn p);
    virtual void undo(ProgressFn p);

    QString _description;
    QString _undoDescription;
    QString _redoDescription;
    std::function<bool(ProgressFn)> _executeFunction;
    std::function<void(ProgressFn)> _undoFunction;
    bool _asynchronous;
};

class CommandManager : public QObject
{
    Q_OBJECT
public:
    CommandManager();

    void clear();
    void execute(std::shared_ptr<Command> command);
    void execute(const QString& description,
                 std::function<bool(ProgressFn)> executeFunction,
                 std::function<void(ProgressFn)> undoFunction,
                 bool asynchronous = false);

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
    bool canUndoNoLocking() const;
    bool canRedoNoLocking() const;

    std::deque<std::shared_ptr<Command>> _stack;
    int _lastExecutedIndex;

    mutable std::mutex _mutex;
    bool _busy;

signals:
    void commandWillExecuteAsynchronously(std::shared_ptr<const Command> command) const;
    void commandProgress(std::shared_ptr<const Command>, int progress) const;
    void commandCompleted(std::shared_ptr<const Command> command) const;
};

#endif // COMMANDMANAGER_H
