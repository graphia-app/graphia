#ifndef INSTALLER_H
#define INSTALLER_H

#include <json_helper.h>

#include <QObject>
#include <QString>
#include <QProcess>

class Installer : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool complete READ complete NOTIFY completeChanged)
    Q_PROPERTY(bool success READ success NOTIFY successChanged)
    Q_PROPERTY(QString error READ error NOTIFY errorChanged)

private:
    json _details;
    QString _version;

    QString _installerFileName;
    QString _existingInstallation;

    QProcess _process;
    QString _error;

    bool _busy = false;
    bool _complete = false;
    bool _success = false;

    void signalComplete();

public:
    Installer(json details, QString version,
        QString existingInstallation);

    Q_INVOKABLE void start();
    Q_INVOKABLE void retry();

    Q_INVOKABLE void setStatus(const QString& status);

    bool busy() const { return _busy; }
    bool complete() const { return _complete; }
    bool success() const { return _success; }
    QString error() const { return _error; }

signals:
    void busyChanged();
    void completeChanged();
    void successChanged();
    void errorChanged();
};

#endif // INSTALLER_H
