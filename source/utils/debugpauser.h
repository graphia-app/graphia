#ifndef DEBUGPAUSER_H
#define DEBUGPAUSER_H

#include <QObject>
#include <QString>

#include <mutex>
#include <condition_variable>

class DebugPauser : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool enabled MEMBER _enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(QString resumeAction MEMBER _resumeAction NOTIFY resumeActionChanged)

public:
    bool enabled() const { return _enabled; }
    void setEnabled(bool enabled);
    void toggleEnabled() { setEnabled(!_enabled); }

    QString resumeAction() const { return _resumeAction; }
    void setResumeAction(const QString& resumeAction);

    void pause(const QString& resumeAction);
    void resume();
    bool paused() const;

private:
    bool _enabled = false;
    QString _resumeAction;
    mutable std::mutex _mutex;
    std::condition_variable _blocker;

signals:
    void enabledChanged() const;
    void pausedChanged() const;
    void resumeActionChanged() const;
};

#endif // DEBUGPAUSER_H
