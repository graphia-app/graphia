#ifndef ICOMMANDMANAGER_H
#define ICOMMANDMANAGER_H

#include "icommand.h"
#include "command.h"

#include <memory>

class ICommandManager
{
public:
    virtual ~ICommandManager() = default;

    virtual void execute(const std::shared_ptr<ICommand>& command) = 0;

    void execute(const Command::CommandDescription& commandDescription,
                 CommandFn&& executeFn, CommandFn&& undoFn)
    {
        execute(std::make_shared<Command>(commandDescription, executeFn, undoFn));
    }

    // Execute only once, i.e. so that it can't be undone
    virtual void executeOnce(const std::shared_ptr<ICommand>& command) = 0;

    void executeOnce(const Command::CommandDescription& commandDescription,
                     CommandFn&& executeFn)
    {
        executeOnce(std::make_shared<Command>(commandDescription, executeFn));
    }

    void executeOnce(CommandFn&& executeFn)
    {
        executeOnce(std::make_shared<Command>(Command::CommandDescription(), executeFn));
    }
};

#endif // ICOMMANDMANAGER_H
