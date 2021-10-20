#ifndef QMLPREFERENCES_H
#define QMLPREFERENCES_H

#include "app/preferenceswatcher.h"

#include <QObject>
#include <QVariant>
#include <QString>
#include <QQmlParserStatus>
#include <QMetaProperty>

class QmlPreferences : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QString section READ section WRITE setSection NOTIFY sectionChanged)

public:
    explicit QmlPreferences(QObject* parent = nullptr);

    QString section() const;
    void setSection(const QString& section);

private:
    bool _initialised = false;
    QString _section;
    PreferencesWatcher _watcher;

    void classBegin() override {}
    void componentComplete() override;

    QString preferenceNameByPropertyName(const QString& propertyName);
    QMetaProperty propertyByName(const QString& propertyName) const;

    QMetaProperty propertyFrom(const QString& preferenceName);

    void setProperty(QMetaProperty property, const QVariant& value);

    void load();

    Q_DISABLE_COPY(QmlPreferences)

private slots:
    void onPreferenceChanged(const QString& key, const QVariant& value);

    void onPropertyChanged();

signals:
    void sectionChanged();
};

#endif // QMLPREFERENCES_H
