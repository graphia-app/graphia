#include "debugpauser.h"

#include <QDebug>

void DebugPauser::setEnabled(bool enabled)
{
#ifdef _DEBUG
    if(_enabled != enabled)
    {
        if(!enabled && paused())
            resume();

        _enabled = enabled;
        emit enabledChanged();
    }
#else
    Q_UNUSED(enabled)
#endif
}

void DebugPauser::setResumeAction(const QString& description)
{
#ifdef _DEBUG
    if(_resumeAction != description)
    {
        _resumeAction = description;
        emit resumeActionChanged();
    }
#else
    Q_UNUSED(description)
#endif
}

void DebugPauser::pause(const QString& resumeAction)
{
#ifdef _DEBUG
    if(!_enabled)
        return;

    std::unique_lock<std::mutex> lock(_mutex);
    setResumeAction(resumeAction);
    qDebug() << "DebugPauser: paused before" << resumeAction;
    emit pausedChanged();
    _blocker.wait(lock);
#else
    Q_UNUSED(resumeAction)
#endif
}

void DebugPauser::resume()
{
#ifdef _DEBUG
    if(!_enabled)
        return;

    std::unique_lock<std::mutex> lock(_mutex);
    qDebug() << "DebugPauser: running from" << _resumeAction;
    setResumeAction(QString());
    emit pausedChanged();
    _blocker.notify_all();
#endif
}

bool DebugPauser::paused() const
{
#ifdef _DEBUG
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);
    return !_resumeAction.isEmpty();
#else
    return false;
#endif
}
