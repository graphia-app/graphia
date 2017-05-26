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
                    const std::vector<std::shared_ptr<ICommand>>& commands) :
        _description(commandDescription._description),
        _verb(commandDescription._verb),
        _pastParticiple(commandDescription._pastParticiple),
        _commands(commands)
    {}

    CompoundCommand(const CompoundCommand&) = delete;
    CompoundCommand(CompoundCommand&&) = delete;
    CompoundCommand& operator=(const CompoundCommand&) = delete;
    CompoundCommand& operator=(CompoundCommand&&) = delete;

    virtual ~CompoundCommand() = default;

    QString description() const { return _description; }
    QString verb() const { return _verb; }

    QString pastParticiple() const { return _pastParticiple; }
    void setPastParticiple(const QString& pastParticiple)
    {
        _pastParticiple = pastParticiple;
    }

private:
    bool execute()
    {
        bool anyExecuted = false;
        for(auto it = _commands.begin(); it != _commands.end(); ++it)
            anyExecuted = (*it)->execute() || anyExecuted;

        return anyExecuted;
    }

    void undo()
    {
        for(auto it = _commands.rbegin(); it != _commands.rend(); ++it)
            (*it)->undo();
    }

    QString _description;
    QString _verb;
    QString _pastParticiple;

    std::vector<std::shared_ptr<ICommand>> _commands;
};

#endif // COMPOUNDCOMMAND_H
