#ifndef COMMAND_H
#define COMMAND_H

#include <QString>

#include <functional>
class Command;

using ExecuteFn = std::function<bool(Command&)>;
using UndoFn = std::function<void(Command&)>;
using ProgressFn = std::function<void(int)>;

// For simple operations the Command class can be used directly, by passing
// it lambdas to perform the execute and undo actions. For more complicated
// operations that require maintaining non-trivial state, Command should be
// subclassed and the execute and undo functions overridden.
class Command
{
    friend class CommandManager;

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

    virtual ~Command() {}

    const QString& description() const;
    void setDescription(const QString& description);
    const QString& undoDescription() const;
    const QString& redoDescription() const;

    const QString& verb() const;
    void setVerb(const QString& verb);
    const QString& undoVerb() const;
    const QString& redoVerb() const;

    const QString& pastParticiple() const;
    void setPastParticiple(const QString& pastParticiple);

    void setProgress(int progress);

    bool asynchronous() const { return _asynchronous; }

private:
    // Return false if the command failed, or did nothing
    virtual bool execute();
    virtual void undo();

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

#endif // COMMAND_H
