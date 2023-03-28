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

#ifndef COMPOUNDCOMMAND_H
#define COMPOUNDCOMMAND_H

#include "icommand.h"
#include "command.h"

#include "shared/utils/container.h"

#include <QObject>

#include <memory>
#include <vector>

using namespace Qt::Literals::StringLiterals;

class CompoundCommand : public ICommand
{
public:
    CompoundCommand(const Command::CommandDescription& commandDescription,
                    ICommandPtrsVector&& commands) :
        _description(commandDescription._description),
        _verb(commandDescription._verb),
        _pastParticiple(commandDescription._pastParticiple),
        _commands(std::move(commands))
    {}

    CompoundCommand(const CompoundCommand&) = delete;
    CompoundCommand(CompoundCommand&&) = delete;
    CompoundCommand& operator=(const CompoundCommand&) = delete;
    CompoundCommand& operator=(CompoundCommand&&) = delete;

    ~CompoundCommand() override = default;

    QString description() const override { return _description; }
    QString verb() const override { return _verb; }

    QString pastParticiple() const override
    {
        if(_pastParticiple.isEmpty())
        {
            // If no past particple is set, fallback on the commands' one(s)
            QString compoundPastParticiple;

            for(const auto& command : _commands)
            {
                auto subCommandPastParticiple = command->pastParticiple();

                if(subCommandPastParticiple.isEmpty())
                    continue;

                if(!compoundPastParticiple.isEmpty())
                    compoundPastParticiple += QObject::tr(", ");

                compoundPastParticiple += subCommandPastParticiple;
            }

            return compoundPastParticiple;
        }

        return _pastParticiple;
    }

    void setPastParticiple(const QString& pastParticiple)

    {
        _pastParticiple = pastParticiple;
    }

    QString debugDescription() const override
    {
        auto text = _description;

        for(const auto& command : _commands)
        {
            auto indent = u"  "_s;
            auto commandDescription = indent + command->debugDescription();
            commandDescription.replace(u"\n"_s, u"\n"_s + indent);

            text.append(u"\n%1"_s.arg(commandDescription));
        }

        return text;
    }

    void cancel() override
    {
        ICommand::cancel();

        for(const auto& command : _commands)
            command->cancel();
    }

    bool cancellable() const override
    {
        return std::any_of(_commands.begin(), _commands.end(),
        [](const auto& command)
        {
            return command->cancellable();
        });
    }

    void initialise() override
    {
        ICommand::initialise();

        for(const auto& command : _commands)
            command->initialise();
    }

    void setProgress(int) override {}
    int progress() const override
    {
        if(_executing != nullptr)
            return _executing->progress();

        return -1;
    }

    void setPhase(const QString&) override {}
    QString phase() const override
    {
        if(_executing != nullptr)
            return _executing->phase();

        return {};
    }

private:
    bool execute() override
    {
        bool anyExecuted = false;
        for(const auto& command : _commands)
        {
            _executing = command.get();
            anyExecuted = command->execute() || anyExecuted;
        }

        _executing = nullptr;

        return anyExecuted;
    }

    void undo() override
    {
        for(const auto& command : u::reverse(_commands))
        {
            _executing = command.get();
            command->undo();
        }

        _executing = nullptr;
    }

    QString _description;
    QString _verb;
    QString _pastParticiple;

    ICommandPtrsVector _commands;

    const ICommand* _executing = nullptr;
};

#endif // COMPOUNDCOMMAND_H
