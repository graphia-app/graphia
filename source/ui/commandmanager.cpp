#include "commandmanager.h"

#include "../utils.h"

#include <QDebug>

#include <thread>

CommandManager::CommandManager() :
    _lastExecutedIndex(-1)
{}

void CommandManager::execute(std::unique_ptr<Command> command)
{
    std::unique_lock<std::mutex> locker(_lock);

    auto executeCommand = [this](std::unique_lock<std::mutex> locker, std::unique_ptr<Command> command)
    {
        if(!command->execute())
            return;

        // There are commands on the stack ahead of us; throw them away
        while(canRedo())
            _stack.pop_back();

        _stack.push_back(std::move(command));
        _lastExecutedIndex = static_cast<int>(_stack.size()) - 1;

        locker.unlock();
        emit commandCompleted(this);
    };

    if(command->asynchronous())
        std::thread(executeCommand, std::move(locker), std::move(command)).detach();
    else
        executeCommand(std::move(locker), std::move(command));
}

void CommandManager::execute(const QString& description,
                             std::function<bool()> executeFunction,
                             std::function<void()> undoFunction,
                             bool asynchronous)
{
    execute(std::make_unique<Command>(description, executeFunction, undoFunction, asynchronous));
}

void CommandManager::undo()
{
    if(!canUndo())
        return;

    std::unique_lock<std::mutex> locker(_lock);
    auto& command = _stack.at(_lastExecutedIndex);

    auto undoCommand = [this, &command](std::unique_lock<std::mutex> locker)
    {
        command->undo();
        _lastExecutedIndex--;
        locker.unlock();
        emit commandCompleted(this);
    };

    if(command->asynchronous())
        std::thread(undoCommand, std::move(locker)).detach();
    else
        undoCommand(std::move(locker));
}

void CommandManager::redo()
{
    if(!canRedo())
        return;

    std::unique_lock<std::mutex> locker(_lock);
    auto& command = _stack.at(++_lastExecutedIndex);

    auto redoCommand = [this, &command](std::unique_lock<std::mutex> locker)
    {
        command->execute();
        locker.unlock();
        emit commandCompleted(this);
    };

    if(command->asynchronous())
        std::thread(redoCommand, std::move(locker)).detach();
    else
        redoCommand(std::move(locker));
}

bool CommandManager::canUndo() const
{
    std::unique_lock<std::mutex> locker(_lock, std::try_to_lock);

    if(locker.owns_lock())
        return _lastExecutedIndex >= 0;

    return false;
}

bool CommandManager::canRedo() const
{
    std::unique_lock<std::mutex> locker(_lock, std::try_to_lock);

    if(locker.owns_lock())
        return _lastExecutedIndex < static_cast<int>(_stack.size()) - 1;

    return false;
}

const std::vector<QString> CommandManager::undoableCommandDescriptions() const
{
    std::unique_lock<std::mutex> locker(_lock, std::try_to_lock);
    std::vector<QString> commandDescriptions;

    if(locker.owns_lock())
    {
        if(canUndo())
        {
            for(int index = _lastExecutedIndex; index >= 0; index--)
                commandDescriptions.push_back(_stack.at(index)->description());
        }
    }

    return commandDescriptions;
}

const std::vector<QString> CommandManager::redoableCommandDescriptions() const
{
    std::unique_lock<std::mutex> locker(_lock, std::try_to_lock);
    std::vector<QString> commandDescriptions;

    if(locker.owns_lock())
    {
        if(canRedo())
        {
            for(int index = _lastExecutedIndex + 1; index < static_cast<int>(_stack.size()); index++)
                commandDescriptions.push_back(_stack.at(index)->description());
        }
    }

    return commandDescriptions;
}
