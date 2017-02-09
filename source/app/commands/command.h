#ifndef COMMAND_H
#define COMMAND_H

#include "shared/commands/icommand.h"
#include "shared/commands/commandfn.h"

#include <QString>

// For simple operations that don't warrant creating a full blown ICommand instance,
// the Command class can be used, usually by the CommandManager::execute* interface
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

    virtual ~Command() {}

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
