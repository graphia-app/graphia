#include "command.h"

#include <QObject>
#include <QDebug>

Command::Command() :
    _failableExecuteFn(defaultFailableCommandFn),
    _executeFn(defaultCommandFn),
    _undoFn(defaultCommandFn)
{}

void Command::setPastParticiple(const QString& pastParticiple)
{
    _pastParticiple = pastParticiple;
}

bool Command::execute()
{
    if(_failableExecuteFn != nullptr)
        return _failableExecuteFn(*this);

    _executeFn(*this);
    return true;
}

void Command::undo() { _undoFn(*this); }

FailableCommandFn Command::defaultFailableCommandFn = [](Command&)
{
    Q_ASSERT(!"failableCommandFn not implmented");
    return false;
};

CommandFn Command::defaultCommandFn = [](Command&)
{
    Q_ASSERT(!"commandFn not implemented");
};
