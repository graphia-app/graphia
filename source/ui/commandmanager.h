#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include "../utils.h"

#include <QtGlobal>
#include <QObject>
#include <QString>

#include <functional>
#include <deque>
#include <vector>
#include <memory>
#include <mutex>

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
            std::function<bool()> executeFunction =
            []
            {
                Q_ASSERT(!"executeFunction not implmented");
                return false;
            },
            std::function<void()> undoFunction =
            []
            {
                Q_ASSERT(!"undoFunction not implemented");
            }, bool asynchronous = false) :
        _description(description),
        _executeFunction(executeFunction),
        _undoFunction(undoFunction),
        _asynchronous(asynchronous)
    {}

    const QString& description() const { return _description; }
    bool asynchronous() const { return _asynchronous; }

private:
    // Return false if the command failed, or did nothing
    virtual bool execute()
    {
        return _executeFunction();
    }

    virtual void undo()
    {
        _undoFunction();
    }

    QString _description;
    std::function<bool()> _executeFunction;
    std::function<void()> _undoFunction;
    bool _asynchronous;
};

class CommandManager : public QObject
{
    Q_OBJECT
public:
    CommandManager();

    void clear();
    void execute(std::unique_ptr<Command> command);
    void execute(const QString& description,
                 std::function<bool()> executeFunction,
                 std::function<void()> undoFunction,
                 bool asynchronous = false);

    void undo();
    void redo();

    bool canUndo() const;
    bool canRedo() const;

    const std::vector<QString> undoableCommandDescriptions() const;
    const std::vector<QString> redoableCommandDescriptions() const;

private:
    std::deque<std::unique_ptr<Command>> _stack;
    int _lastExecutedIndex;

    mutable std::mutex _lock;

signals:
    void commandCompleted(const CommandManager* commandManager) const;
};

#endif // COMMANDMANAGER_H
