#include "commandmanager.h"

CommandManager::CommandManager() :
    _lastExecutedIndex(-1)
{}

CommandManager::~CommandManager()
{
    while(!_stack.empty())
        delete _stack.pop();
}

void CommandManager::execute(Command* command)
{
    if(!command->execute())
    {
        delete command;
        return;
    }

    while(canRedo())
    {
        // There are commands on the stack ahead of us; throw them away
        delete _stack.pop();
    }

    _stack.push(command);
    _lastExecutedIndex = _stack.size() - 1;

    emit commandStackChanged(*this);
}

void CommandManager::execute(const QString& description, std::function<bool()> executeFunction, std::function<void()> undoFunction)
{
    Command* command = new Command(description, executeFunction, undoFunction);
    execute(command);
}

void CommandManager::undo()
{
    if(!canUndo())
        return;

    Command* command = _stack.at(_lastExecutedIndex);
    command->undo();
    _lastExecutedIndex--;

    emit commandStackChanged(*this);
}

void CommandManager::redo()
{
    if(!canRedo())
        return;

    _lastExecutedIndex++;
    Command* command = _stack.at(_lastExecutedIndex);
    command->execute();

    emit commandStackChanged(*this);
}

bool CommandManager::canUndo() const
{
    return _lastExecutedIndex >= 0;
}

bool CommandManager::canRedo() const
{
    return _lastExecutedIndex < _stack.size() - 1;
}

const QList<const Command*> CommandManager::undoableCommands() const
{
    QList<const Command*> commands;

    if(canUndo())
    {
        for(int index = _lastExecutedIndex; index >= 0; index--)
            commands.append(_stack.at(index));
    }

    return commands;
}

const QList<const Command*> CommandManager::redoableCommands() const
{
    QList<const Command*> commands;

    if(canRedo())
    {
        for(int index = _lastExecutedIndex + 1; index < _stack.size(); index++)
            commands.append(_stack.at(index));
    }

    return commands;
}
