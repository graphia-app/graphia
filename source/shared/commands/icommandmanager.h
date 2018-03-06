#ifndef ICOMMANDMANAGER_H
#define ICOMMANDMANAGER_H

#include "icommand.h"
#include "command.h"
#include "compoundcommand.h"

#include <memory>
#include <vector>

class ICommandManager
{
public:
    virtual ~ICommandManager() = default;

    virtual void execute(std::unique_ptr<ICommand> command) = 0;

    void execute(const Command::CommandDescription& commandDescription,
                 CommandFn&& executeFn, CommandFn&& undoFn)
    {
        execute(std::make_unique<Command>(commandDescription, executeFn, undoFn));
    }

    void execute(const Command::CommandDescription& commandDescription,
                 std::vector<std::unique_ptr<ICommand>> commands)
    {
        execute(std::make_unique<CompoundCommand>(commandDescription, std::move(commands)));
    }

    void execute(std::vector<std::unique_ptr<ICommand>> commands)
    {
        execute(std::make_unique<CompoundCommand>(Command::CommandDescription(), std::move(commands)));
    }

    // Execute only once, i.e. so that it can't be undone
    virtual void executeOnce(std::unique_ptr<ICommand> command) = 0;

    void executeOnce(const Command::CommandDescription& commandDescription,
                     CommandFn&& executeFn)
    {
        executeOnce(std::make_unique<Command>(commandDescription, executeFn));
    }

    void executeOnce(const Command::CommandDescription& commandDescription,
                     std::vector<std::unique_ptr<ICommand>> commands)
    {
        executeOnce(std::make_unique<CompoundCommand>(commandDescription, std::move(commands)));
    }

    void executeOnce(std::vector<std::unique_ptr<ICommand>> commands)
    {
        executeOnce(std::make_unique<CompoundCommand>(Command::CommandDescription(), std::move(commands)));
    }

    void executeOnce(CommandFn&& executeFn)
    {
        executeOnce(std::make_unique<Command>(Command::CommandDescription(), executeFn));
    }
};

#endif // ICOMMANDMANAGER_H
