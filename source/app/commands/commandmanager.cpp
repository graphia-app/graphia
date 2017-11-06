#include "commandmanager.h"

#include "shared/utils/thread.h"

#include <QDebug>

#include <thread>

CommandManager::CommandManager() :
    _lock(_mutex, std::defer_lock),
    _busy(false),
    _graphChanged(false)
{
    connect(this, &CommandManager::commandQueued, this, &CommandManager::update);
    connect(this, &CommandManager::commandCompleted, this, &CommandManager::onCommandCompleted);

    _debug = qEnvironmentVariableIntValue("COMMAND_DEBUG");
}

CommandManager::~CommandManager()
{
    if(_thread.joinable())
    {
        if(_currentCommand != nullptr)
            _currentCommand->cancel();

        _thread.join();
    }

    if(_lock.owns_lock())
        _lock.unlock();
}

void CommandManager::execute(std::unique_ptr<ICommand> command)
{
    _pendingCommands.emplace_back(CommandAction::Execute, std::move(command));
    emit commandQueued();
}

void CommandManager::executeOnce(std::unique_ptr<ICommand> command)
{
    _pendingCommands.emplace_back(CommandAction::ExecuteOnce, std::move(command));
    emit commandQueued();
}

void CommandManager::undo()
{
    _pendingCommands.emplace_back(CommandAction::Undo);
    emit commandQueued();
}

void CommandManager::redo()
{
    _pendingCommands.emplace_back(CommandAction::Redo);
    emit commandQueued();
}

void CommandManager::executeReal(std::unique_ptr<ICommand> command, bool irreversible)
{
    if(_lock.owns_lock())
    {
        if(_debug > 0)
            qDebug() << "enqueuing command" << command->description();

        // Something is already executing, do the new command once that is finished
        _pendingCommands.emplace_back(irreversible ? CommandAction::ExecuteOnce : CommandAction::Execute, std::move(command));
        return;
    }

    _lock.lock();

    _busy = true;
    emit busyChanged();
    if(_debug > 0)
        qDebug() << "Command started" << command->description();

    auto commandPtr = command.get();
    auto verb = command->verb();
    doCommand(commandPtr, verb, [this, command = std::move(command), irreversible]() mutable
    {
        u::setCurrentThreadName(command->description());

        _graphChanged = false;

        QString description;
        QString pastParticiple;
        bool success = false;

        if(command->execute() && !command->cancelled())
        {
            description = command->description();
            pastParticiple = command->pastParticiple();
            success = true;

            if(!irreversible)
            {
                // There are commands on the stack ahead of us; throw them away
                while(canRedoNoLocking())
                    _stack.pop_back();

                _stack.push_back(std::move(command));
                _lastExecutedIndex = static_cast<int>(_stack.size()) - 1;
            }
            else if(_graphChanged)
            {
                // The graph changed during an irreversible command, so throw
                // away our redo history as it is likely no longer coherent with
                // the current state
                clearCommandStackNoLocking();
            }
        }

        clearCurrentCommand();

        _busy = false;
        emit commandCompleted(success, description, pastParticiple);
    });
}

void CommandManager::undoReal()
{
    if(_lock.owns_lock())
    {
        _pendingCommands.emplace_back(CommandAction::Undo, nullptr);
        return;
    }

    _lock.lock();

    if(!canUndoNoLocking())
        return;

    auto command = _stack.at(_lastExecutedIndex).get();

    _busy = true;
    emit busyChanged();

    QString undoVerb = !command->description().isEmpty() ?
                QObject::tr("Undoing ") + command->description() :
                QObject::tr("Undoing");

    doCommand(command, undoVerb, [this, command]
    {
        u::setCurrentThreadName("(u) " + command->description());

        command->undo();
        _lastExecutedIndex--;

        clearCurrentCommand();

        _busy = false;
        emit commandCompleted(true, command->description(), QString());
    });
}

void CommandManager::redoReal()
{
    if(_lock.owns_lock())
    {
        _pendingCommands.emplace_back(CommandAction::Redo, nullptr);
        return;
    }

    _lock.lock();

    if(!canRedoNoLocking())
        return;

    auto command = _stack.at(++_lastExecutedIndex).get();

    _busy = true;
    emit busyChanged();

    QString redoVerb = !command->description().isEmpty() ?
                QObject::tr("Redoing ") + command->description() :
                QObject::tr("Redoing");

    doCommand(command, redoVerb, [this, command]
    {
        u::setCurrentThreadName("(r) " + command->description());

        command->execute();

        clearCurrentCommand();

        _busy = false;
        emit commandCompleted(true, command->description(), command->pastParticiple());
    });
}

bool CommandManager::canUndo() const
{
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);

    if(lock.owns_lock())
        return canUndoNoLocking();

    return false;
}

bool CommandManager::canRedo() const
{
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);

    if(lock.owns_lock())
        return canRedoNoLocking();

    return false;
}

bool CommandManager::commandIsCancellable() const
{
    std::unique_lock<std::mutex> lock(_currentCommandMutex);

    if(_currentCommand != nullptr)
        return _currentCommand->cancellable();

    return false;
}

const std::vector<QString> CommandManager::undoableCommandDescriptions() const
{
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);
    std::vector<QString> commandDescriptions;
    commandDescriptions.reserve(_lastExecutedIndex);

    if(lock.owns_lock() && canUndoNoLocking())
    {
        for(int index = _lastExecutedIndex; index >= 0; index--)
            commandDescriptions.push_back(_stack.at(index)->description());
    }

    return commandDescriptions;
}

const std::vector<QString> CommandManager::redoableCommandDescriptions() const
{
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);
    std::vector<QString> commandDescriptions;
    commandDescriptions.reserve(_stack.size());

    if(lock.owns_lock() && canRedoNoLocking())
    {
        for(int index = _lastExecutedIndex + 1; index < static_cast<int>(_stack.size()); index++)
            commandDescriptions.push_back(_stack.at(index)->description());
    }

    return commandDescriptions;
}

QString CommandManager::nextUndoAction() const
{
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);

    if(lock.owns_lock() && canUndoNoLocking())
    {
        auto& command = _stack.at(_lastExecutedIndex);
        if(!command->description().isEmpty())
            return QObject::tr("Undo ") + command->description();
    }

    return tr("Undo");
}

QString CommandManager::nextRedoAction() const
{
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);

    if(lock.owns_lock() && canRedoNoLocking())
    {
        auto& command = _stack.at(static_cast<size_t>(_lastExecutedIndex) + 1);
        if(!command->description().isEmpty())
            return QObject::tr("Redo ") + command->description();
    }

    return tr("Redo");
}

bool CommandManager::busy() const
{
    return _busy;
}

void CommandManager::clearCommandStack()
{
    std::unique_lock<std::mutex> lock(_mutex);

    // If a command is still in progress, wait until it's finished
    if(_thread.joinable())
        _thread.join();

    clearCommandStackNoLocking();
}

void CommandManager::clearCommandStackNoLocking()
{
    _stack.clear();
    _lastExecutedIndex = -1;

    // Force a UI update
    emit commandStackCleared();
}

void CommandManager::clearCurrentCommand()
{
    // _currentCommand is a raw pointer, so we must ensure it is reset to null
    // when the underlying unique_ptr is destroyed
    std::unique_lock<std::mutex> lock(_currentCommandMutex);
    _currentCommand = nullptr;
}


void CommandManager::cancel()
{
    std::unique_lock<std::mutex> lock(_currentCommandMutex);

    if(_currentCommand != nullptr)
    {
        _currentCommand->cancel();

        if(_debug > 0)
            qDebug() << "Command cancel request" << _currentCommand->description();
    }
}

void CommandManager::timerEvent(QTimerEvent*)
{
    std::unique_lock<std::mutex> lock(_currentCommandMutex);

    if(_currentCommand == nullptr)
        return;

    int newCommandProgress = _currentCommand->progress();

    if(newCommandProgress != _commandProgress)
    {
        _commandProgress = newCommandProgress;
        emit commandProgressChanged();
    }
}

bool CommandManager::canUndoNoLocking() const
{
    return _lastExecutedIndex >= 0;
}

bool CommandManager::canRedoNoLocking() const
{
    return _lastExecutedIndex < static_cast<int>(_stack.size()) - 1;
}

void CommandManager::onCommandCompleted(bool success, QString description, QString)
{
    killTimer(_commandProgressTimerId);
    _commandProgressTimerId = -1;

    if(_thread.joinable())
        _thread.join();

    Q_ASSERT(_lock.owns_lock());
    _lock.unlock();

    if(_debug > 0)
    {
        if(success)
        {
            if(!description.isEmpty())
                qDebug() << "Command completed" << description;
            else
                qDebug() << "Command completed";
        }
        else
            qDebug() << "Command failed/cancelled";
    }

    if(!_pendingCommands.empty())
        update();
    else
    {
        emit busyChanged();
        emit commandIsCancellableChanged();
    }
}

void CommandManager::update()
{
    if(!_pendingCommands.empty())
    {
        auto pendingCommand = std::move(_pendingCommands.front());
        _pendingCommands.pop_front();

        switch(pendingCommand._action)
        {
        case CommandAction::Execute:      executeReal(std::move(pendingCommand._command), false); break;
        case CommandAction::ExecuteOnce:  executeReal(std::move(pendingCommand._command), true); break;
        case CommandAction::Undo:         undoReal(); break;
        case CommandAction::Redo:         redoReal(); break;
        default: break;
        }
    }
}
