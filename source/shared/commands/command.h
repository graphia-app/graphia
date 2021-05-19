/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#ifndef COMMAND_H
#define COMMAND_H

#include "icommand.h"

#include <QString>

#include <functional>
#include <type_traits>
#include <utility>

class Command;
class CommandFn
{
public:
    template<typename Fn, typename ReturnType>
    using EnableIfFnReturnTypeIs = typename std::enable_if_t<
        std::is_same_v<
            std::invoke_result_t<Fn, Command&>,
            ReturnType
        > &&
        // Avoid enabling the universal move constructors if Fn is a CommandFn
        !std::is_same_v<std::remove_reference_t<Fn>, CommandFn>
    >;

    template<typename Fn, typename = EnableIfFnReturnTypeIs<Fn, bool>>
    // cppcheck-suppress noExplicitConstructor
    CommandFn(Fn&& fn) : // NOLINT
        _boolFn(fn) {}

    template<typename Fn, typename = EnableIfFnReturnTypeIs<Fn, void>, typename = void>
    // cppcheck-suppress noExplicitConstructor
    CommandFn(Fn&& fn) : // NOLINT
        _voidFn(fn) {}

    CommandFn() = default;
    CommandFn(const CommandFn& other) = default;
    CommandFn(CommandFn&& other) = default; // NOLINT
    CommandFn& operator=(const CommandFn& other) = default;
    CommandFn& operator=(CommandFn&& other) = default; // NOLINT

    bool operator()(Command& command) const
    {
        if(_boolFn != nullptr)
            return _boolFn(command);

        _voidFn(command);
        return true;
    }

private:
    std::function<bool(Command&)> _boolFn;
    std::function<void(Command&)> _voidFn;
};

// For simple operations that don't warrant creating a full blown ICommand instance,
// the Command class can be used, usually by the ICommandManager::execute interface
class Command : public ICommand
{
public:
    struct CommandDescription
    {
        QString _description;
        QString _verb;
        QString _pastParticiple;

        CommandDescription(QString description = {}, // NOLINT
                           QString verb = {},
                           QString pastParticiple = {}) :
            _description(std::move(description)),
            _verb(std::move(verb)),
            _pastParticiple(std::move(pastParticiple))
        {}
    };

    Command(CommandDescription commandDescription,
            // cppcheck-suppress passedByValue
            CommandFn executeFn,
            // cppcheck-suppress passedByValue
            CommandFn undoFn = [](Command&) { Q_ASSERT(!"undoFn not implemented"); }) :
        _description(std::move(commandDescription._description)),
        _verb(commandDescription._verb),
        _pastParticiple(commandDescription._pastParticiple),
        _executeFn(std::move(executeFn)),
        _undoFn(std::move(undoFn))
    {}

    Command(const Command&) = delete;
    Command(Command&&) = delete;
    Command& operator=(const Command&) = delete;
    Command& operator=(Command&&) = delete;

    ~Command() override = default;

    QString description() const override { return _description; }
    QString verb() const override { return _verb; }

    QString pastParticiple() const override { return _pastParticiple; }
    void setPastParticiple(const QString& pastParticiple)
    {
        _pastParticiple = pastParticiple;
    }

private:
    bool execute() override { return _executeFn(*this); }
    void undo() override { _undoFn(*this); }

    QString _description;
    QString _verb;
    QString _pastParticiple;

    CommandFn _executeFn;
    CommandFn _undoFn;
};

#endif // COMMAND_H
