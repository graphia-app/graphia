#ifndef COMMAND_H
#define COMMAND_H

#include <QString>

#include <functional>
#include <type_traits>
class Command;

using FailableCommandFn = std::function<bool(Command&)>;
using CommandFn = std::function<void(Command&)>;
using ProgressFn = std::function<void(int)>;

// For simple operations the Command class can be used directly, by passing
// it lambdas to perform the execute and undo actions. For more complicated
// operations that require maintaining non-trivial state, Command should be
// subclassed and the execute and undo functions overridden.
class Command
{
    friend class CommandManager;

private:
    static FailableCommandFn defaultFailableCommandFn;
    static CommandFn defaultCommandFn;

    template<typename Fn, typename Result = typename std::result_of<Fn(Command&)>::type>
    struct ExecuteFnInitialisationHelper;

    template<typename Fn> struct ExecuteFnInitialisationHelper<Fn, void>
    {
        static void initialise(Command& command, Fn executeFn)
        {
            command._executeFn = executeFn;
        }
    };

    template<typename Fn> struct ExecuteFnInitialisationHelper<Fn, bool>
    {
        static void initialise(Command& command, Fn executeFn)
        {
            command._failableExecuteFn = executeFn;
        }
    };

    template<typename Fn> void initialiseExecuteFn(Fn executeFn)
    {
        ExecuteFnInitialisationHelper<Fn>::initialise(*this, executeFn);
    }

    void initialise();

public:
    template<typename ExecuteFn>
    Command(const QString& description, const QString& verb,
            const QString& pastParticiple,
            ExecuteFn executeFn,
            CommandFn undoFn = defaultCommandFn,
            bool asynchronous = true) :
        _description(description),
        _verb(verb),
        _pastParticiple(pastParticiple),
        _undoFn(undoFn),
        _asynchronous(asynchronous)
    {
        initialise();
        initialiseExecuteFn(executeFn);
    }

    template<typename ExecuteFn>
    Command(const QString& description, const QString& verb,
            ExecuteFn executeFn,
            CommandFn undoFn = defaultCommandFn,
            bool asynchronous = true) :
        _description(description),
        _verb(verb),
        _undoFn(undoFn),
        _asynchronous(asynchronous)
    {
        initialise();
        initialiseExecuteFn(executeFn);
    }

    template<typename ExecuteFn>
    Command(const QString& description,
            ExecuteFn executeFn,
            CommandFn undoFn = defaultCommandFn,
            bool asynchronous = true) :
        _description(description),
        _undoFn(undoFn),
        _asynchronous(asynchronous)
    {
        initialise();
        initialiseExecuteFn(executeFn);
    }

    Command(bool asynchronous = true);

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
    virtual void cancel();

    void setProgressFn(ProgressFn progressFn);

    QString _description;
    QString _undoDescription;
    QString _redoDescription;

    QString _verb;
    QString _undoVerb;
    QString _redoVerb;

    QString _pastParticiple;

    FailableCommandFn _failableExecuteFn;
    CommandFn _executeFn;
    CommandFn _undoFn;
    ProgressFn _progressFn;
    bool _asynchronous;
};

#endif // COMMAND_H
