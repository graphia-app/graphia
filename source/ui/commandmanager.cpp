#include "commandmanager.h"

#include "../utils/make_unique.h"
#include "../utils/namethread.h"
#include "../utils/unique_lock_with_side_effects.h"

#include <QDebug>

#include <thread>

CommandManager::CommandManager() :
    _lastExecutedIndex(-1)
{}

void CommandManager::execute(std::unique_ptr<Command> command)
{
    unique_lock_with_side_effects<std::mutex> locker(_mutex);
    locker.setPostUnlockAction([this, &command] { emit commandCompleted(this, command.get()); });

    auto executeCommand = [this](unique_lock_with_side_effects<std::mutex> /*locker*/, std::unique_ptr<Command> command)
    {
        nameCurrentThread(command->description());

        if(!command->execute([this, &command](int progress) { emit commandProgress(this, command.get(), progress); }))
            return;

        // There are commands on the stack ahead of us; throw them away
        while(canRedo())
            _stack.pop_back();

        _stack.push_back(std::move(command));
        _lastExecutedIndex = static_cast<int>(_stack.size()) - 1;
    };

    if(command->asynchronous())
    {
        emit commandWillExecuteAsynchronously(this, command.get());
        std::thread(executeCommand, std::move(locker), std::move(command)).detach();
    }
    else
        executeCommand(std::move(locker), std::move(command));
}

void CommandManager::execute(const QString& description,
                             std::function<bool(ProgressFn)> executeFunction,
                             std::function<void(ProgressFn)> undoFunction,
                             bool asynchronous)
{
    execute(std::make_unique<Command>(description, executeFunction, undoFunction, asynchronous));
}

void CommandManager::undo()
{
    if(!canUndo())
        return;

    unique_lock_with_side_effects<std::mutex> locker(_mutex);
    auto& command = _stack.at(_lastExecutedIndex);
    locker.setPostUnlockAction([this, &command] { emit commandCompleted(this, command.get()); });

    auto undoCommand = [this, &command](unique_lock_with_side_effects<std::mutex> /*locker*/)
    {
        nameCurrentThread("(u) " + command->description());

        command->undo([this, &command](int progress) { emit commandProgress(this, command.get(), progress); });
        _lastExecutedIndex--;
    };

    if(command->asynchronous())
    {
        emit commandWillExecuteAsynchronously(this, command.get());
        std::thread(undoCommand, std::move(locker)).detach();
    }
    else
        undoCommand(std::move(locker));
}

void CommandManager::redo()
{
    if(!canRedo())
        return;

    unique_lock_with_side_effects<std::mutex> locker(_mutex);
    auto& command = _stack.at(++_lastExecutedIndex);
    locker.setPostUnlockAction([this, &command] { emit commandCompleted(this, command.get()); });

    auto redoCommand = [this, &command](unique_lock_with_side_effects<std::mutex> /*locker*/)
    {
        nameCurrentThread("(r) " + command->description());

        command->execute([this, &command](int progress) { emit commandProgress(this, command.get(), progress); });
    };

    if(command->asynchronous())
    {
        emit commandWillExecuteAsynchronously(this, command.get());
        std::thread(redoCommand, std::move(locker)).detach();
    }
    else
        redoCommand(std::move(locker));
}

bool CommandManager::canUndo() const
{
    std::unique_lock<std::mutex> locker(_mutex, std::try_to_lock);

    if(locker.owns_lock())
        return _lastExecutedIndex >= 0;

    return false;
}

bool CommandManager::canRedo() const
{
    std::unique_lock<std::mutex> locker(_mutex, std::try_to_lock);

    if(locker.owns_lock())
        return _lastExecutedIndex < static_cast<int>(_stack.size()) - 1;

    return false;
}

const std::vector<QString> CommandManager::undoableCommandDescriptions() const
{
    bool commandsToUndo = canUndo();
    std::unique_lock<std::mutex> locker(_mutex, std::try_to_lock);
    std::vector<QString> commandDescriptions;

    if(locker.owns_lock() && commandsToUndo)
    {
        for(int index = _lastExecutedIndex; index >= 0; index--)
            commandDescriptions.push_back(_stack.at(index)->description());
    }

    return commandDescriptions;
}

const std::vector<QString> CommandManager::redoableCommandDescriptions() const
{
    bool commandsToRedo = canRedo();
    std::unique_lock<std::mutex> locker(_mutex, std::try_to_lock);
    std::vector<QString> commandDescriptions;

    if(locker.owns_lock() && commandsToRedo)
    {
        for(int index = _lastExecutedIndex + 1; index < static_cast<int>(_stack.size()); index++)
            commandDescriptions.push_back(_stack.at(index)->description());
    }

    return commandDescriptions;
}

bool CommandManager::busy() const
{
    if(_mutex.try_lock())
    {
        _mutex.unlock();
        return false;
    }

    return true;
}
