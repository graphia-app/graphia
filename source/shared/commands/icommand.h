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
    virtual void cancel()
    {
        qWarning() << description() << "does not implement cancel(); now blocked until it completes";
    }

    virtual void setProgress(int progress) { _progress = progress; }
    virtual int progress() const { return _progress; }

private:
    std::atomic<int> _progress;
};

#endif // ICOMMAND_H
