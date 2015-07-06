#include "debuglongrunningcommand.h"

#include <QObject>

DebugLongRunningCommand::DebugLongRunningCommand() :
    Command()
{
    setDescription(QObject::tr("Long Running Command"));
    setVerb(QObject::tr("Running Long Command"));
    setPastParticiple(QObject::tr("Command Ran Long"));
}

DebugLongRunningCommand* DebugLongRunningCommand::_activeLongRunningCommand = nullptr;

bool DebugLongRunningCommand::execute()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _activeLongRunningCommand = this;
    _blocker.wait(lock);

    return true;
}

void DebugLongRunningCommand::undo()
{
    execute();
}

void DebugLongRunningCommand::cancel()
{
    DebugLongRunningCommand::stop();
}

void DebugLongRunningCommand::stop()
{
    if(_activeLongRunningCommand != nullptr)
    {
        std::unique_lock<std::mutex> lock(_activeLongRunningCommand->_mutex);
        _activeLongRunningCommand->_blocker.notify_all();
        _activeLongRunningCommand = nullptr;
    }
}

bool DebugLongRunningCommand::running()
{
    return _activeLongRunningCommand != nullptr;
}
