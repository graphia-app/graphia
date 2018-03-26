#ifndef COMPOUNDCOMMAND_H
#define COMPOUNDCOMMAND_H

#include "icommand.h"
#include "command.h"

#include "shared/utils/container.h"

#include <QObject>

#include <memory>
#include <vector>

class CompoundCommand : public ICommand
{
public:
    CompoundCommand(const Command::CommandDescription& commandDescription,
                    std::vector<std::unique_ptr<ICommand>>&& commands) :
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

    std::vector<std::unique_ptr<ICommand>> _commands;

    const ICommand* _executing = nullptr;
};

#endif // COMPOUNDCOMMAND_H
