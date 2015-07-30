#include "commandmanager.h"

#include "../utils/namethread.h"

#include <thread>

CommandManager::CommandManager() :
    _lock(_mutex, std::defer_lock),
    _busy(false)
{
    qRegisterMetaType<std::shared_ptr<Command>>("std::shared_ptr<Command>");
    connect(this, &CommandManager::commandCompleted, this, &CommandManager::onCommandCompleted);
}

CommandManager::~CommandManager()
{
    if(_thread.joinable())
    {
        _currentCommand->cancel();
        _thread.join();
    }

    if(_lock.owns_lock())
        _lock.unlock();
}

void CommandManager::undo()
{
    QMetaObject::invokeMethod(this, "undoReal");
}

void CommandManager::redo()
{
    QMetaObject::invokeMethod(this, "redoReal");
}

void CommandManager::executeReal(std::shared_ptr<Command> command)
{
    _lock.lock();

    command->setProgressFn([this](int progress) { _commandProgress = progress; emit commandProgressChanged(); });

    auto executeCommand = [this, command]()
    {
        if(command->asynchronous())
            nameCurrentThread(command->description());

        if(!command->execute())
        {
            _busy = false;
            emit commandCompleted(nullptr, QString());
            return;
        }

        // There are commands on the stack ahead of us; throw them away
        while(canRedoNoLocking())
            _stack.pop_back();

        _stack.push_back(command);
        _lastExecutedIndex = static_cast<int>(_stack.size()) - 1;

        _busy = false;
        emit commandCompleted(command.get(), command->pastParticiple());
    };

    _busy = true;
    emit busyChanged();

    if(command->asynchronous())
        executeAsynchronous(command, command->verb(), executeCommand);
    else
        executeCommand();
}

void CommandManager::undoReal()
{
    _lock.lock();

    if(!canUndoNoLocking())
        return;

    auto command = _stack.at(_lastExecutedIndex);

    auto undoCommand = [this, command]()
    {
        if(command->asynchronous())
            nameCurrentThread("(u) " + command->description());

        command->undo();
        _lastExecutedIndex--;

        _busy = false;
        emit commandCompleted(command.get(), QString());
    };

    _busy = true;
    emit busyChanged();

    if(command->asynchronous())
        executeAsynchronous(command, command->undoVerb(), undoCommand);
    else
        undoCommand();
}

void CommandManager::redoReal()
{
    _lock.lock();

    if(!canRedoNoLocking())
        return;

    auto command = _stack.at(++_lastExecutedIndex);

    auto redoCommand = [this, command]()
    {
        if(command->asynchronous())
            nameCurrentThread("(r) " + command->description());

        command->execute();

        _busy = false;
        emit commandCompleted(command.get(), command->pastParticiple());
    };

    _busy = true;
    emit busyChanged();

    if(command->asynchronous())
        executeAsynchronous(command, command->redoVerb(), redoCommand);
    else
        redoCommand();
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

const std::vector<QString> CommandManager::undoableCommandDescriptions() const
{
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);
    std::vector<QString> commandDescriptions;

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
        return command->undoDescription();
    }

    return tr("Undo");
}

QString CommandManager::nextRedoAction() const
{
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);

    if(lock.owns_lock() && canRedoNoLocking())
    {
        auto& command = _stack.at(_lastExecutedIndex + 1);
        return command->redoDescription();
    }

    return tr("Redo");
}

bool CommandManager::busy() const
{
    return _busy;
}

bool CommandManager::canUndoNoLocking() const
{
    return _lastExecutedIndex >= 0;
}

bool CommandManager::canRedoNoLocking() const
{
    return _lastExecutedIndex < static_cast<int>(_stack.size()) - 1;
}

void CommandManager::onCommandCompleted(const Command*, const QString&)
{
    // If the command executed asynchronously, we need to join its thread
    if(_thread.joinable())
        _thread.join();

    Q_ASSERT(_lock.owns_lock());
    _lock.unlock();

    emit busyChanged();
}
