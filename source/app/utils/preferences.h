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
    std::map<QString, QVariant> _minimumValue;
    std::map<QString, QVariant> _maximumValue;

public:
    void define(const QString& key, const QVariant& defaultValue = QVariant(),
                const QVariant& minimumValue = QVariant(), const QVariant& maximumValue = QVariant());

    QVariant get(const QString& key);

    QVariant minimum(const QString& key) const;
    QVariant maximum(const QString& key) const;

    void set(const QString& key, QVariant value);

    bool exists(const QString& key);

signals:
    void preferenceChanged(const QString& key, const QVariant& value);
    void minimumChanged(const QString& key, const QVariant& value);
    void maximumChanged(const QString& key, const QVariant& value);
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

    QString preferenceNameByPropertyName(const QString& propertyName);
    QMetaProperty propertyByName(const QString& propertyName);

    QMetaProperty valuePropertyFrom(const QString& preferenceName);
    QMetaProperty minimumPropertyFrom(const QString& preferenceName);
    QMetaProperty maximumPropertyFrom(const QString& preferenceName);

    void setProperty(QMetaProperty property, const QVariant& value);

    void load();
    void save();
    void flush();

    Q_DISABLE_COPY(QmlPreferences)

private slots:
    void onPreferenceChanged(const QString& key, const QVariant& value);
    void onMinimumChanged(const QString& key, const QVariant& value);
    void onMaximumChanged(const QString& key, const QVariant& value);

    void onPropertyChanged();
};

namespace u
{
    template<typename... Args> void definePref(Args&&... args) { return S(Preferences)->define(std::forward<Args>(args)...); }
    template<typename... Args> QVariant pref(Args&&... args)   { return S(Preferences)->get(std::forward<Args>(args)...); }
    template<typename... Args> void setPref(Args&&... args)    { return S(Preferences)->set(std::forward<Args>(args)...); }
    template<typename... Args> bool prefExists(Args&&... args) { return S(Preferences)->exists(std::forward<Args>(args)...); }
}

#endif // PREFERENCES_H

