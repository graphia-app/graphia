/* Copyright © 2013-2024 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QObject>
#include <QQmlEngine>
#include <QString>
#include <QVariant>
#include <QQmlParserStatus>
#include <QMetaProperty>

#include <mutex>
#include <set>

class QString;

namespace u
{
    void definePref(const QString& key, const QVariant& defaultValue);
    QVariant pref(const QString& key);
    bool prefExists(const QString& key);

    bool removePref(const QString& key);
    void setPref(const QString& key, const QVariant& value);

    void updateOldPrefs();

    QString settingsFileName();
} // namespace u

class PreferencesWatcher : public QObject
{
    Q_OBJECT

    friend void u::setPref(const QString& key, const QVariant& value);

private:
    static struct Watchers
    {
        std::recursive_mutex _mutex;
        std::set<PreferencesWatcher*> _set;
    } _watchers;

    static void setPref(const QString& key, const QVariant& value);

public:
    PreferencesWatcher();
    ~PreferencesWatcher() override;

signals:
    void preferenceChanged(const QString& key, const QVariant& value);
};

class Preferences : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QString section READ section WRITE setSection NOTIFY sectionChanged)

public:
    explicit Preferences(QObject* parent = nullptr);

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

private slots:
    void onPreferenceChanged(const QString& key, const QVariant& value);

    void onPropertyChanged();

signals:
    void sectionChanged();
};

#endif // PREFERENCES_H
