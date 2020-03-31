/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

#include "singleton.h"

#include <QObject>
#include <QSettings>
#include <QVariant>
#include <QString>
#include <QQmlParserStatus>
#include <QCoreApplication>

#include <map>

class Preferences : public QObject, public Singleton<Preferences>
{
    Q_OBJECT

private:
    QSettings _settings;
    std::map<QString, QVariant> _defaultValue;
    std::map<QString, QVariant> _minimumValue;
    std::map<QString, QVariant> _maximumValue;

public:
    Preferences() :
        _settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope,
            QCoreApplication::organizationName(), QCoreApplication::applicationName())
    {}

    void define(const QString& key, const QVariant& defaultValue = QVariant(),
                const QVariant& minimumValue = QVariant(), const QVariant& maximumValue = QVariant());

    QVariant get(const QString& key);

    QVariant minimum(const QString& key) const;
    QVariant maximum(const QString& key) const;

    void set(const QString& key, QVariant value, bool notify = true);
    void reset(const QString& key);

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
    Q_PROPERTY(QString section READ section WRITE setSection NOTIFY sectionChanged)

public:
    explicit QmlPreferences(QObject* parent = nullptr);

    QString section() const;
    void setSection(const QString& section);

    Q_INVOKABLE void reset(const QString& key);

private:
    bool _initialised = false;
    QString _section;

    void classBegin() override {}
    void componentComplete() override;

    QString preferenceNameByPropertyName(const QString& propertyName);
    QMetaProperty propertyByName(const QString& propertyName) const;

    QMetaProperty valuePropertyFrom(const QString& preferenceName);
    QMetaProperty minimumPropertyFrom(const QString& preferenceName);
    QMetaProperty maximumPropertyFrom(const QString& preferenceName);

    void setProperty(QMetaProperty property, const QVariant& value);

    void load();

    Q_DISABLE_COPY(QmlPreferences)

private slots:
    void onPreferenceChanged(const QString& key, const QVariant& value);
    void onMinimumChanged(const QString& key, const QVariant& value);
    void onMaximumChanged(const QString& key, const QVariant& value);

    void onPropertyChanged();

signals:
    void sectionChanged();
};

namespace u
{
    template<typename... Args> void definePref(Args&&... args)  { return S(Preferences)->define(std::forward<Args>(args)...); }
    template<typename... Args> QVariant pref(Args&&... args)    { return S(Preferences)->get(std::forward<Args>(args)...); }
    template<typename... Args> QVariant minPref(Args&&... args) { return S(Preferences)->minimum(std::forward<Args>(args)...); }
    template<typename... Args> QVariant maxPref(Args&&... args) { return S(Preferences)->maximum(std::forward<Args>(args)...); }
    template<typename... Args> void setPref(Args&&... args)     { return S(Preferences)->set(std::forward<Args>(args)...); }
    template<typename... Args> void resetPref(Args&&... args)   { return S(Preferences)->reset(std::forward<Args>(args)...); }
    template<typename... Args> bool prefExists(Args&&... args)  { return S(Preferences)->exists(std::forward<Args>(args)...); }
} // namespace u

#endif // PREFERENCES_H

