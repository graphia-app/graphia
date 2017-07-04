#ifndef ICOMMAND_H
#define ICOMMAND_H

#include <QString>
#include <QDebug>
#include <QtGlobal>

#include <atomic>
#include <cassert>

class ICommand
{
public:
    virtual ~ICommand() = default;

    virtual QString description() const = 0;
    virtual QString verb() const { return description(); }
    virtual QString pastParticiple() const { return {}; }

    // Return false if the command failed, or did nothing
    virtual bool execute() = 0;
    virtual void undo() { Q_ASSERT(!"undo() not implmented for this ICommand"); }

    virtual void cancel() { _cancelled = true; }
    virtual bool cancelled() const { return _cancelled; }

    virtual void setProgress(int progress) { _progress = progress; }
    virtual int progress() const { return _progress; }

    virtual void initialise()
    {
        _progress = -1;
        _cancelled = false;
    }

private:
    std::atomic<int> _progress;
    std::atomic<bool> _cancelled;
};

#endif // ICOMMAND_H
