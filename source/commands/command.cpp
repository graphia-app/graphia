#include "command.h"

#include <QObject>
#include <QDebug>

void Command::initialise()
{
    // To initialise the undo/redo strings
    setDescription(_description);

    _progressFn = [](int){};
}

Command::Command(bool asynchronous) :
    _failableExecuteFn(defaultFailableCommandFn),
    _executeFn(defaultCommandFn),
    _undoFn(defaultCommandFn),
    _asynchronous(asynchronous)
{
    initialise();
}

const QString& Command::description() const { return _description; }

void Command::setDescription(const QString &description)
{
    _description = description;

    if(!_description.isEmpty())
    {
        _undoDescription = QObject::tr("Undo ") + _description;
        _redoDescription = QObject::tr("Redo ") + _description;
        _undoVerb = QObject::tr("Undoing ") + _description;
        _redoVerb = QObject::tr("Redoing ") + _description;
    }
    else
    {
        _undoDescription = QObject::tr("Undo");
        _redoDescription = QObject::tr("Redo");
        _undoVerb = QObject::tr("Undoing");
        _redoVerb = QObject::tr("Redoing");
    }
}

const QString& Command::undoDescription() const { return _undoDescription; }
const QString& Command::redoDescription() const { return _redoDescription; }

const QString& Command::verb() const { return _verb; }

void Command::setVerb(const QString &verb)
{
    _verb = verb;
}
const QString& Command::undoVerb() const { return _undoVerb; }
const QString& Command::redoVerb() const { return _redoVerb; }

const QString& Command::pastParticiple() const { return _pastParticiple; }

void Command::setPastParticiple(const QString& pastParticiple)
{
    _pastParticiple = pastParticiple;
}

void Command::setProgress(int progress)
{
    _progressFn(progress);
}

bool Command::execute()
{
    if(_failableExecuteFn != nullptr)
        return _failableExecuteFn(*this);

    _executeFn(*this);
    return true;
}

void Command::undo() { _undoFn(*this); }

void Command::cancel()
{
    qWarning() << description() << "does not implement cancel(); now blocked until it completes";
}

void Command::setProgressFn(ProgressFn progressFn)
{
    _progressFn = progressFn;
}

FailableCommandFn Command::defaultFailableCommandFn = [](Command&)
{
    Q_ASSERT(!"failableCommandFn not implmented");
    return false;
};

CommandFn Command::defaultCommandFn = [](Command&)
{
    Q_ASSERT(!"commandFn not implemented");
};
