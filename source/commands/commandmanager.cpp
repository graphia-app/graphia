#include "commandmanager.h"

#include "../utils/namethread.h"
#include "../utils/unique_lock_with_side_effects.h"

#include <QDebug>

#include <thread>

CommandManager::CommandManager() :
    _lastExecutedIndex(-1),
    _busy(false)
{
    connect(this, &CommandManager::commandCompleted, this, &CommandManager::onCommandCompleted);
}

void CommandManager::executeReal(std::shared_ptr<Command> command)
{
    unique_lock_with_side_effects<std::mutex> locker(_mutex);
    auto commandPtr = command.get();

    locker.setPostUnlockAction([this, commandPtr] { _busy = false; emit commandCompleted(commandPtr, commandPtr->pastParticiple()); });
    command->setProgressFn([this, commandPtr](int progress) { emit commandProgress(commandPtr, progress); });

    auto executeCommand = [this](const unique_lock_with_side_effects<std::mutex>& /*locker*/, std::shared_ptr<Command> command)
    {
        nameCurrentThread(command->description());

        if(!command->execute())
            return;

        // There are commands on the stack ahead of us; throw them away
        while(canRedoNoLocking())
            _stack.pop_back();

        _stack.push_back(command);
        _lastExecutedIndex = static_cast<int>(_stack.size()) - 1;
    };

    _busy = true;
    if(command->asynchronous())
    {
        emit commandWillExecuteAsynchronously(commandPtr, command->verb());
        _thread = std::thread(executeCommand, std::move(locker), command);
    }
    else
        executeCommand(std::move(locker), command);
}

void CommandManager::undo()
{
    unique_lock_with_side_effects<std::mutex> locker(_mutex);

    if(!canUndoNoLocking())
        return;

    auto command = _stack.at(_lastExecutedIndex);
    auto commandPtr = command.get();
    locker.setPostUnlockAction([this, commandPtr] { _busy = false; emit commandCompleted(commandPtr, QString()); });

    auto undoCommand = [this, command](const unique_lock_with_side_effects<std::mutex>& /*locker*/)
    {
        nameCurrentThread("(u) " + command->description());

        command->undo();
        _lastExecutedIndex--;
    };

    _busy = true;
    if(command->asynchronous())
    {
        emit commandWillExecuteAsynchronously(commandPtr, command->undoVerb());
        _thread = std::thread(undoCommand, std::move(locker));
    }
    else
        undoCommand(std::move(locker));
}

void CommandManager::redo()
{
    unique_lock_with_side_effects<std::mutex> locker(_mutex);

    if(!canRedoNoLocking())
        return;

    auto command = _stack.at(++_lastExecutedIndex);
    auto commandPtr = command.get();
    locker.setPostUnlockAction([this, commandPtr] { _busy = false; emit commandCompleted(commandPtr, commandPtr->pastParticiple()); });

    auto redoCommand = [this, command](const unique_lock_with_side_effects<std::mutex>& /*locker*/)
    {
        nameCurrentThread("(r) " + command->description());

        command->execute();
    };

    _busy = true;
    if(command->asynchronous())
    {
        emit commandWillExecuteAsynchronously(commandPtr, command->redoVerb());
        _thread = std::thread(redoCommand, std::move(locker));
    }
    else
        redoCommand(std::move(locker));
}

bool CommandManager::canUndo() const
{
    std::unique_lock<std::mutex> locker(_mutex, std::try_to_lock);

    if(locker.owns_lock())
        return canUndoNoLocking();

    return false;
}

bool CommandManager::canRedo() const
{
    std::unique_lock<std::mutex> locker(_mutex, std::try_to_lock);

    if(locker.owns_lock())
        return canRedoNoLocking();

    return false;
}

const std::vector<QString> CommandManager::undoableCommandDescriptions() const
{
    std::unique_lock<std::mutex> locker(_mutex, std::try_to_lock);
    std::vector<QString> commandDescriptions;

    if(locker.owns_lock() && canUndoNoLocking())
    {
        for(int index = _lastExecutedIndex; index >= 0; index--)
            commandDescriptions.push_back(_stack.at(index)->description());
    }

    return commandDescriptions;
}

const std::vector<QString> CommandManager::redoableCommandDescriptions() const
{
    std::unique_lock<std::mutex> locker(_mutex, std::try_to_lock);
    std::vector<QString> commandDescriptions;

    if(locker.owns_lock() && canRedoNoLocking())
    {
        for(int index = _lastExecutedIndex + 1; index < static_cast<int>(_stack.size()); index++)
            commandDescriptions.push_back(_stack.at(index)->description());
    }

    return commandDescriptions;
}

const QString CommandManager::nextUndoAction() const
{
    std::unique_lock<std::mutex> locker(_mutex, std::try_to_lock);

    if(locker.owns_lock() && canUndoNoLocking())
    {
        auto& command = _stack.at(_lastExecutedIndex);
        return command->undoDescription();
    }

    return QString();
}

const QString CommandManager::nextRedoAction() const
{
    std::unique_lock<std::mutex> locker(_mutex, std::try_to_lock);

    if(locker.owns_lock() && canRedoNoLocking())
    {
        auto& command = _stack.at(_lastExecutedIndex + 1);
        return command->redoDescription();
    }

    return QString();
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
}
