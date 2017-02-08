#include "command.h"

#include <QtGlobal>

Command::Command(const Command::CommandDescription& commandDescription,
                 CommandFn executeFn, CommandFn undoFn) :
    _description(commandDescription._description),
    _verb(commandDescription._verb),
    _pastParticiple(commandDescription._pastParticiple),
    _executeFn(executeFn),
    _undoFn(undoFn)
{}

void Command::setPastParticiple(const QString& pastParticiple)
{
    _pastParticiple = pastParticiple;
}

bool Command::execute() { return _executeFn(*this); }
void Command::undo() { _undoFn(*this); }

CommandFn Command::defaultCommandFn = [](Command&)
{
    Q_ASSERT(!"commandFn not implemented");
};
