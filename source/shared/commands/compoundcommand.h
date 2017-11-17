#ifndef COMPOUNDCOMMAND_H
#define COMPOUNDCOMMAND_H

#include "icommand.h"
#include "command.h"

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

    QString pastParticiple() const override { return _pastParticiple; }
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
        for(auto it = _commands.begin(); it != _commands.end(); ++it)
        {
            _executing = (*it).get();
            anyExecuted = (*it)->execute() || anyExecuted;
        }

        _executing = nullptr;

        return anyExecuted;
    }

    void undo() override
    {
        for(auto it = _commands.rbegin(); it != _commands.rend(); ++it)
        {
            _executing = (*it).get();
            (*it)->undo();
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
