/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

    template<void(ICommandManager::*Fn)(ICommandPtr), typename T, typename... Commands>
    void _execute(T&& command, Commands&&... commands)
    {
        if constexpr(sizeof...(commands) != 0)
            (this->*Fn)(makeCompoundCommand(std::forward<T>(command), std::forward<Commands>(commands)...));
        else
            (this->*Fn)(makeCommand(std::forward<T>(command)));
    }

    template<typename... Args>
    using EnableIfArgsAreAllCommands = typename std::enable_if_t<(... &&
        (std::is_convertible_v<Args, ICommandPtr> || std::is_convertible_v<Args, CommandFn&&>))
    >;

public:
    virtual void execute(ICommandPtr command) = 0;

    // cppcheck-suppress passedByValue
    void execute(ICommandPtrsVector commands, const Command::CommandDescription& commandDescription = {})
    {
        execute(std::make_unique<CompoundCommand>(commandDescription, std::move(commands)));
    }

    template<typename... Commands, typename = EnableIfArgsAreAllCommands<Commands...>>
    void execute(Commands&&... commands)
    {
        _execute<&ICommandManager::execute>(std::forward<Commands>(commands)...);
    }

    // Execute only once, i.e. so that it can't be undone
    virtual void executeOnce(ICommandPtr command) = 0;

    void executeOnce(CommandFn&& executeFn, const QString& commandDescription = {})
    {
        executeOnce(std::make_unique<Command>(Command::CommandDescription{commandDescription,
            commandDescription, commandDescription}, executeFn));
    }

    // cppcheck-suppress passedByValue
    void executeOnce(ICommandPtrsVector commands, const QString& commandDescription = {})
    {
        executeOnce(std::make_unique<CompoundCommand>(Command::CommandDescription{commandDescription,
            commandDescription, commandDescription}, std::move(commands)));
    }

    template<typename... Commands, typename = EnableIfArgsAreAllCommands<Commands...>>
    void executeOnce(Commands&&... commands)
    {
        _execute<&ICommandManager::executeOnce>(std::forward<Commands>(commands)...);
    }
};

#endif // ICOMMANDMANAGER_H
