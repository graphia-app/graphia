#include "commandmanager.h"

#include "../utils.h"

#include <QDebug>

CommandManager::CommandManager() :
    _lastExecutedIndex(-1)
{}

void CommandManager::execute(std::unique_ptr<Command> command)
{
    if(!command->execute())
        return;

    while(canRedo())
    {
        // There are commands on the stack ahead of us; throw them away
        _stack.pop_back();
    }

    _stack.push_back(std::move(command));
    _lastExecutedIndex = static_cast<int>(_stack.size()) - 1;

    emit commandStackChanged(*this);
}

void CommandManager::execute(const QString& description, std::function<bool()> executeFunction, std::function<void()> undoFunction)
{
    execute(std::make_unique<Command>(description, executeFunction, undoFunction));
}

void CommandManager::undo()
{
    if(!canUndo())
        return;

    _stack.at(_lastExecutedIndex)->undo();
    _lastExecutedIndex--;

    emit commandStackChanged(*this);
}

void CommandManager::redo()
{
    if(!canRedo())
        return;

    _lastExecutedIndex++;
    _stack.at(_lastExecutedIndex)->execute();

    emit commandStackChanged(*this);
}

bool CommandManager::canUndo() const
{
    return _lastExecutedIndex >= 0;
}

bool CommandManager::canRedo() const
{
    return _lastExecutedIndex < static_cast<int>(_stack.size()) - 1;
}

const std::vector<QString> CommandManager::undoableCommandDescriptions() const
{
    std::vector<QString> commandDescriptions;

    if(canUndo())
    {
        for(int index = _lastExecutedIndex; index >= 0; index--)
            commandDescriptions.push_back(_stack.at(index)->description());
    }

    return commandDescriptions;
}

const std::vector<QString> CommandManager::redoableCommandDescriptions() const
{
    std::vector<QString> commandDescriptions;

    if(canRedo())
    {
        for(int index = _lastExecutedIndex + 1; index < static_cast<int>(_stack.size()); index++)
            commandDescriptions.push_back(_stack.at(index)->description());
    }

    return commandDescriptions;
}
