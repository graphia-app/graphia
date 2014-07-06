#include "commandmanager.h"

#include "../utils/namethread.h"
#include "../utils/unique_lock_with_side_effects.h"

#include <QDebug>

#include <thread>

CommandManager::CommandManager() :
    _lastExecutedIndex(-1),
    _busy(false)
{}

void CommandManager::executeReal(std::shared_ptr<Command> command)
{
    unique_lock_with_side_effects<std::mutex> locker(_mutex);
    locker.setPostUnlockAction([this, command] { _busy = false; emit commandCompleted(command.get(), command->pastParticiple()); });
    command->setProgressFn([this, command](int progress) { emit commandProgress(command.get(), progress); });

    auto executeCommand = [this](unique_lock_with_side_effects<std::mutex> /*locker*/, std::shared_ptr<Command> command)
    {
        nameCurrentThread(command->description());

        if(!command->execute())
            return;

        // There are commands on the stack ahead of us; throw them away
        while(canRedoNoLocking())
            _stack.pop_back();

        _stack.push_back(std::move(command));
        _lastExecutedIndex = static_cast<int>(_stack.size()) - 1;
    };

    _busy = true;
    if(command->asynchronous())
    {
        emit commandWillExecuteAsynchronously(command.get(), command->verb());
        std::thread(executeCommand, std::move(locker), command).detach();
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
    locker.setPostUnlockAction([this, command] { _busy = false; emit commandCompleted(command.get(), QString()); });

    auto undoCommand = [this, command](unique_lock_with_side_effects<std::mutex> /*locker*/)
    {
        nameCurrentThread("(u) " + command->description());

        command->undo();
        _lastExecutedIndex--;
    };

    _busy = true;
    if(command->asynchronous())
    {
        emit commandWillExecuteAsynchronously(command.get(), command->undoVerb());
        std::thread(undoCommand, std::move(locker)).detach();
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
    locker.setPostUnlockAction([this, command] { _busy = false; emit commandCompleted(command.get(), command->pastParticiple()); });

    auto redoCommand = [this, command](unique_lock_with_side_effects<std::mutex> /*locker*/)
    {
        nameCurrentThread("(r) " + command->description());

        command->execute();
    };

    _busy = true;
    if(command->asynchronous())
    {
        emit commandWillExecuteAsynchronously(command.get(), command->redoVerb());
        std::thread(redoCommand, std::move(locker)).detach();
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
