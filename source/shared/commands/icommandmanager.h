/* Copyright © 2013-2023 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ICOMMANDMANAGER_H
#define ICOMMANDMANAGER_H

#include "icommand.h"
#include "command.h"
#include "compoundcommand.h"

#include <memory>
#include <vector>
#include <type_traits>

class QString;

enum class ExecutePolicy
{
    Add,        // Add to the execution stack
    Replace,    // Replace the top of the execution stack
    Once,       // Don't add to the execution stack...
    OnceMutate  // ...but allow state change (not for general use)
};

class ICommandManager
{
public:
    virtual ~ICommandManager() = default;

private:
    template<typename T>
    ICommandPtr makeCommand(T command)
    {
        if constexpr(std::is_convertible_v<T, ICommandPtr>)
        {
            // Already is a command
            return command;
        }
        else
            return std::make_unique<Command>(Command::CommandDescription(), command);
    }

    void makeCommandsVector(ICommandPtrsVector&) {}

    template<typename T, typename... Commands>
    void makeCommandsVector(ICommandPtrsVector& commands, T&& command, Commands&&... args)
    {
        commands.emplace_back(makeCommand(std::forward<T>(command)));
        makeCommandsVector(commands, std::forward<Commands>(args)...);
    }

    template<typename... Commands>
    auto makeCompoundCommand(Commands&&... args)
    {
        ICommandPtrsVector commands;
        makeCommandsVector(commands, std::forward<Commands>(args)...);
        return std::make_unique<CompoundCommand>(Command::CommandDescription(), std::move(commands));
    }

    template<typename... Args>
    using EnableIfArgsAreAllCommands = typename std::enable_if_t<(... &&
        (std::is_convertible_v<Args, ICommandPtr> || std::is_convertible_v<Args, CommandFn&&>))
    >;

public:
    virtual void execute(ExecutePolicy policy, ICommandPtr command) = 0;

    // cppcheck-suppress passedByValue
    void execute(ExecutePolicy policy, ICommandPtrsVector commands,
        const Command::CommandDescription& commandDescription = {})
    {
        execute(policy, std::make_unique<CompoundCommand>(commandDescription, std::move(commands)));
    }

    template<typename Command, typename... Commands, typename = EnableIfArgsAreAllCommands<Command, Commands...>>
    void execute(ExecutePolicy policy, Command&& command, Commands&&... commands)
    {
        if constexpr(sizeof...(commands) != 0)
            execute(policy, makeCompoundCommand(std::forward<Command>(command), std::forward<Commands>(commands)...));
        else
            execute(policy, makeCommand(std::forward<Command>(command)));
    }

    void executeOnce(CommandFn&& executeFn, const QString& commandDescription)
    {
        execute(ExecutePolicy::Once, std::make_unique<Command>(Command::CommandDescription{commandDescription,
            commandDescription, commandDescription}, std::move(executeFn)));
    }

    void executeOnce(CommandFn&& executeFn, const Command::CommandDescription& commandDescription = {})
    {
        execute(ExecutePolicy::Once, std::make_unique<Command>(commandDescription, std::move(executeFn)));
    }

    virtual void setPhase(const QString& phase) = 0;
    void clearPhase() { setPhase({}); }
};

#endif // ICOMMANDMANAGER_H
