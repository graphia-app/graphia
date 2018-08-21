#ifndef ICOMMAND_H
#define ICOMMAND_H

#include <QString>
#include <QDebug>
#include <QtGlobal>

#include <atomic>
#include <cassert>

#include "shared/utils/progressable.h"
#include "shared/utils/cancellable.h"

class ICommand : public Progressable, public Cancellable
{
public:
    ~ICommand() override = default;

    virtual QString description() const = 0;
    virtual QString verb() const { return description(); }
    virtual QString pastParticiple() const { return {}; }

    // Return false if the command failed, or did nothing
    virtual bool execute() = 0;
    virtual void undo() { Q_ASSERT(!"undo() not implemented for this ICommand"); }

    void setProgress(int progress) override
    {
        Progressable::setProgress(progress);
        _progress = progress;
    }

    virtual int progress() const { return _progress; }

    virtual void initialise()
    {
        _progress = -1;
        uncancel();
    }

    virtual bool cancellable() const { return false; }

private:
    std::atomic<int> _progress{-1};
};

#endif // ICOMMAND_H
