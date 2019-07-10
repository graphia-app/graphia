#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QString>

#include <json_helper.h>

#include <atomic>

class QString;
class QByteArray;
class QStringList;
class QNetworkReply;

class Updater : public QObject
{
    Q_OBJECT

public:
    Updater();
    ~Updater() override;

    void enableAutoBackgroundCheck();
    void disableAutoBackgroundCheck();

    int progress() const { return _progress; }
    void cancelUpdateDownload();

    static bool updateAvailable();
    static bool showUpdatePrompt(const QStringList& arguments);

    QString updateStatus() const;
    void resetUpdateStatus();

public slots:
    void startBackgroundUpdateCheck();

private:
    QTimer _backgroundCheckTimer;

    QTimer _timeoutTimer;
    QNetworkAccessManager _networkManager;
    QNetworkReply* _reply = nullptr;
    QString _updateString;
    int _progress = -1;

    enum class State
    {
        Idle,
        Update,
        File
    } _state = State::Idle;

    QString _fileName;
    QString _checksum;

    void downloadUpdate(QNetworkReply* reply);
    void saveUpdate(QNetworkReply* reply);

    void onPreferenceChanged(const QString& key, const QVariant&);

private slots:
    void onReplyReceived(QNetworkReply* reply);
    void onTimeout();

signals:
    void noNewUpdateAvailable(bool existing);
    void updateDownloaded();
    void progressChanged();
};

#endif // UPDATER_H
