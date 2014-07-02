#include "commandmanager.h"

#include "../utils/namethread.h"
#include "../utils/unique_lock_with_side_effects.h"

#include <QDebug>

#include <thread>

CommandManager::CommandManager() :
    _lastExecutedIndex(-1),
    _busy(false)
{
    qRegisterMetaType<std::shared_ptr<const Command>>("std::shared_ptr<const Command>");
}

void CommandManager::execute(std::shared_ptr<Command> command)
{
    unique_lock_with_side_effects<std::mutex> locker(_mutex);
    locker.setPostUnlockAction([this, command] { _busy = false; emit commandCompleted(command); });

    auto executeCommand = [this](unique_lock_with_side_effects<std::mutex> /*locker*/, std::shared_ptr<Command> command)
    {
        nameCurrentThread(command->description());

        if(!command->execute([this, command](int progress) { emit commandProgress(command, progress); }))
            return;

        // There are commands on the stack ahead of us; throw them away
        while(canRedo())
            _stack.pop_back();

        _stack.push_back(std::move(command));
        _lastExecutedIndex = static_cast<int>(_stack.size()) - 1;
    };

    _busy = true;
    if(command->asynchronous())
    {
        emit commandWillExecuteAsynchronously(command);
        std::thread(executeCommand, std::move(locker), command).detach();
    }
    else
        executeCommand(std::move(locker), command);
}

void CommandManager::execute(const QString& description,
                             std::function<bool(ProgressFn)> executeFunction,
                             std::function<void(ProgressFn)> undoFunction,
                             bool asynchronous)
{
    execute(std::make_shared<Command>(description, executeFunction, undoFunction, asynchronous));
}

void CommandManager::undo()
{
    if(!canUndo())
        return;

    unique_lock_with_side_effects<std::mutex> locker(_mutex);
    auto& command = _stack.at(_lastExecutedIndex);
    locker.setPostUnlockAction([this, command] { _busy = false; emit commandCompleted(command); });

    auto undoCommand = [this, command](unique_lock_with_side_effects<std::mutex> /*locker*/)
    {
        nameCurrentThread("(u) " + command->description());

        command->undo([this, command](int progress) { emit commandProgress(command, progress); });
        _lastExecutedIndex--;
    };

    _busy = true;
    if(command->asynchronous())
    {
        emit commandWillExecuteAsynchronously(command);
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
    locker.setPostUnlockAction([this, command] { _busy = false; emit commandCompleted(command); });

    auto redoCommand = [this, command](unique_lock_with_side_effects<std::mutex> /*locker*/)
    {
        nameCurrentThread("(r) " + command->description());

        command->execute([this, command](int progress) { emit commandProgress(command, progress); });
    };

    _busy = true;
    if(command->asynchronous())
    {
        emit commandWillExecuteAsynchronously(command);
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
    return _busy;
}
