#ifndef COMMANDMANAGER_H
#define COMMANDMANAGER_H

#include <QtGlobal>
#include <QObject>
#include <QStack>
#include <QString>

#include <functional>
#include <vector>

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
            }) :
        _description(description),
        _executeFunction(executeFunction),
        _undoFunction(undoFunction)
    {}

    const QString& description() const { return _description; }

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
};

class CommandManager : public QObject
{
    Q_OBJECT
public:
    CommandManager();
    CommandManager(const CommandManager& c) : _stack(c._stack), _lastExecutedIndex(c._lastExecutedIndex) {}
    ~CommandManager();

    void clear();
    void execute(Command* command);
    void execute(const QString& description,
                 std::function<bool()> executeFunction,
                 std::function<void()> undoFunction);

    void undo();
    void redo();

    bool canUndo() const;
    bool canRedo() const;

    const std::vector<const Command*> undoableCommands() const;
    const std::vector<const Command*> redoableCommands() const;

private:
    QStack<Command*> _stack;
    int _lastExecutedIndex;

signals:
    void commandStackChanged(const CommandManager& commandManager) const;
};

#endif // COMMANDMANAGER_H
