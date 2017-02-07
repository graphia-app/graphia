#ifndef COMMAND_H
#define COMMAND_H

#include "shared/commands/icommand.h"

#include <QString>

#include <functional>
#include <type_traits>

class Command;
enum class CommandAction
{
    None,
    Execute,
    Undo,
    Redo
};

using FailableCommandFn = std::function<bool(Command&)>;
using CommandFn = std::function<void(Command&)>;

// For simple operations the Command class can be used directly, by passing
// it lambdas to perform the execute and undo actions. For more complicated
// operations that require maintaining non-trivial state, Command should be
// subclassed and the execute and undo functions overridden.
class Command : public ICommand
{
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

public:
    template<typename ExecuteFn>
    Command(const QString& description, const QString& verb,
            const QString& pastParticiple,
            ExecuteFn executeFn,
            CommandFn undoFn = defaultCommandFn) :
        _description(description),
        _verb(verb),
        _pastParticiple(pastParticiple),
        _undoFn(undoFn)
    {
        initialiseExecuteFn(executeFn);
    }

    template<typename ExecuteFn>
    Command(const QString& description, const QString& verb,
            ExecuteFn executeFn,
            CommandFn undoFn = defaultCommandFn) :
        _description(description),
        _verb(verb),
        _undoFn(undoFn)
    {
        initialiseExecuteFn(executeFn);
    }

    template<typename ExecuteFn>
    Command(const QString& description,
            ExecuteFn executeFn,
            CommandFn undoFn = defaultCommandFn) :
        _description(description),
        _verb(description),
        _undoFn(undoFn)
    {
        initialiseExecuteFn(executeFn);
    }

    template<typename ExecuteFn>
    Command(ExecuteFn executeFn)
    {
        initialiseExecuteFn(executeFn);
    }

    Command();

    Command(const Command&) = delete;
    Command(Command&&) = delete;
    Command& operator=(const Command&) = delete;
    Command& operator=(Command&&) = delete;

    virtual ~Command() {}

    QString description() const { return _description; }
    QString verb() const { return _verb; }

    QString pastParticiple() const { return _pastParticiple; }
    void setPastParticiple(const QString& pastParticiple);

private:
    bool execute();
    void undo();

    QString _description;
    QString _verb;
    QString _pastParticiple;

    FailableCommandFn _failableExecuteFn;
    CommandFn _executeFn;
    CommandFn _undoFn;
};

#endif // COMMAND_H
