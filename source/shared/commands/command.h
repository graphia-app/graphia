#ifndef COMMAND_H
#define COMMAND_H

#include "icommand.h"

#include <QString>

#include <functional>
#include <type_traits>

class Command;
class CommandFn
{
public:
    template<typename Fn, typename ReturnType>
    using EnableIfFnReturnTypeIs = typename std::enable_if_t<
        std::is_same<
            typename std::result_of_t<Fn(Command&)>,
            ReturnType
        >::value &&
        // Avoid enabling the universal move constructors if Fn is a CommandFn
        !std::is_same<std::remove_reference_t<Fn>, CommandFn>::value
    >;

    template<typename Fn, typename = EnableIfFnReturnTypeIs<Fn, bool>>
    CommandFn(Fn&& fn) : _boolFn(fn) {}

    template<typename Fn, typename = EnableIfFnReturnTypeIs<Fn, void>, typename = void>
    CommandFn(Fn&& fn) : _voidFn(fn) {}

    CommandFn() = default;
    CommandFn(const CommandFn& other) = default;
    CommandFn(CommandFn&& other) = default;
    CommandFn& operator=(const CommandFn& other) = default;
    CommandFn& operator=(CommandFn&& other) = default;

    bool operator()(Command& command) const
    {
        if(_boolFn != nullptr)
            return _boolFn(command);

        _voidFn(command);
        return true;
    }

private:
    std::function<bool(Command&)> _boolFn;
    std::function<void(Command&)> _voidFn;
};

// For simple operations that don't warrant creating a full blown ICommand instance,
// the Command class can be used, usually by the ICommandManager::execute* interface
class Command : public ICommand
{
public:
    struct CommandDescription
    {
        QString _description;
        QString _verb;
        QString _pastParticiple;

        CommandDescription(QString description = {},
                           QString verb = {},
                           QString pastParticiple = {}) :
            _description(description),
            _verb(verb),
            _pastParticiple(pastParticiple)
        {}
    };

    Command(const CommandDescription& commandDescription,
            CommandFn executeFn,
            CommandFn undoFn = [](Command&) { Q_ASSERT(!"undoFn not implemented"); }) :
        _description(commandDescription._description),
        _verb(commandDescription._verb),
        _pastParticiple(commandDescription._pastParticiple),
        _executeFn(executeFn),
        _undoFn(undoFn)
    {}

    Command(const Command&) = delete;
    Command(Command&&) = delete;
    Command& operator=(const Command&) = delete;
    Command& operator=(Command&&) = delete;

    virtual ~Command() = default;

    QString description() const { return _description; }
    QString verb() const { return _verb; }

    QString pastParticiple() const { return _pastParticiple; }
    void setPastParticiple(const QString& pastParticiple)
    {
        _pastParticiple = pastParticiple;
    }

private:
    bool execute() { return _executeFn(*this); }
    void undo() { _undoFn(*this); }

    QString _description;
    QString _verb;
    QString _pastParticiple;

    CommandFn _executeFn;
    CommandFn _undoFn;
};

#endif // COMMAND_H
