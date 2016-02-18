#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "singleton.h"

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QString>
#include <QQmlParserStatus>

#include <map>

class Preferences : public QObject, public Singleton<Preferences>
{
    Q_OBJECT

private:
    QSettings _settings;

public:
    QVariant get(const QString& key, const QVariant& defaultValue = QVariant());
    void set(const QString& key, const QVariant& value);

    bool exists(const QString& key);

signals:
    void preferenceChanged(const QString& key, const QVariant& value);
};

class QmlPreferences : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString section READ section WRITE setSection FINAL)

public:
    explicit QmlPreferences(QObject* parent = nullptr);
    ~QmlPreferences();

    QString section() const;
    void setSection(const QString& section);

private:
    bool _initialised = false;
    QString _section;
    std::map<QString, QVariant> _pendingPreferenceChanges;
    int _timerId = 0;

    void timerEvent(QTimerEvent *);

    void classBegin() {}
    void componentComplete();

    QString preferenceNameFrom(const QString& propertyName);
    QMetaProperty propertyFrom(const QString& preferenceName);

    void setProperty(QMetaProperty property, const QVariant& value);

    void load();
    void save();
    void flush();

    Q_DISABLE_COPY(QmlPreferences)

private slots:
    void onPreferenceChanged(const QString& key, const QVariant& value);
    void onPropertyChanged();
};

namespace u
{
    QVariant pref(const QString& key, const QVariant& defaultValue = QVariant());
    void setPref(const QString& key, const QVariant& value);
    bool prefExists(const QString& key);
}

#endif // PREFERENCES_H

