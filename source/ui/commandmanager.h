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
#include <atomic>

class Command;

using ExecuteFn = std::function<bool(Command&)>;
using UndoFn = std::function<void(Command&)>;
using ProgressFn = std::function<void(int)>;

// For simple operations the Command class can be used directly, by passing
// it lambdas to perform the execute and undo actions. For more complicated
// operations that require maintaining non-trivial state, Command should be
// subclassed and the execute and undo functions overridden.
class Command : public QObject
{
    friend class CommandManager;

    Q_OBJECT

private:
    static ExecuteFn defaultExecuteFn;
    static UndoFn defaultUndoFn;

    void initialise();

public:
    Command(const QString& description, const QString& verb,
            const QString &pastParticiple,
            ExecuteFn executeFn = defaultExecuteFn,
            UndoFn undoFn = defaultUndoFn,
            bool asynchronous = true);
    Command(const QString& description, const QString& verb,
            ExecuteFn executeFn = defaultExecuteFn,
            UndoFn undoFn = defaultUndoFn,
            bool asynchronous = true);
    Command(const QString& description,
            ExecuteFn executeFn = defaultExecuteFn,
            UndoFn undoFn = defaultUndoFn,
            bool asynchronous = true);
    Command(ExecuteFn executeFn = defaultExecuteFn,
            UndoFn undoFn = defaultUndoFn,
            bool asynchronous = true);

    const QString& description() const;
    const QString& undoDescription() const;
    const QString& redoDescription() const;

    const QString& verb() const;
    const QString& undoVerb() const;
    const QString& redoVerb() const;

    const QString& pastParticiple() const;
    void setPastParticiple(const QString& pastParticiple);

    void setProgress(int progress);

    bool asynchronous() const { return _asynchronous; }

private:
    // Return false if the command failed, or did nothing
    virtual bool execute(Command& command);
    virtual void undo(Command& command);

    void setProgressFn(ProgressFn progressFn);

    QString _description;
    QString _undoDescription;
    QString _redoDescription;

    QString _verb;
    QString _undoVerb;
    QString _redoVerb;

    QString _pastParticiple;

    ExecuteFn _executeFn;
    UndoFn _undoFn;
    ProgressFn _progressFn;
    bool _asynchronous;
};

class CommandManager : public QObject
{
    Q_OBJECT
public:
    CommandManager();

    void clear();

    void execute(std::shared_ptr<Command> command);

    template<typename... Args> void execute(Args&&... args)
    {
        execute(std::make_shared<Command>(std::forward<Args>(args)...));
    }

    template<typename... Args> void executeSynchronous(Args&&... args)
    {
        execute(std::make_shared<Command>(std::forward<Args>(args)..., false));
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
    bool canUndoNoLocking() const;
    bool canRedoNoLocking() const;

    std::deque<std::shared_ptr<Command>> _stack;
    int _lastExecutedIndex;

    mutable std::mutex _mutex;
    std::atomic<bool> _busy;

signals:
    void commandWillExecuteAsynchronously(std::shared_ptr<const Command> command, const QString& verb) const;
    void commandProgress(std::shared_ptr<const Command>, int progress) const;
    void commandCompleted(std::shared_ptr<const Command> command, const QString& pastParticiple) const;
};

#endif // COMMANDMANAGER_H
