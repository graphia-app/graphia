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

#include "preferences.h"

#include <QSettings>
#include <QMetaProperty>
#include <QRegularExpression>
#include <QCoreApplication>

namespace
{
QSettings settings()
{
    return {QSettings::Format::IniFormat, QSettings::Scope::UserScope,
            QCoreApplication::organizationName(), QCoreApplication::applicationName()};
}
} // namespace

void copyKajekaSettings()
{
    QSettings kajekaSettings(QStringLiteral("Kajeka"), QStringLiteral("Graphia"));
    auto targetSettings = settings();

    // No settings present
    if(kajekaSettings.allKeys().empty())
        return;

    // Already copied
    if(kajekaSettings.value(QStringLiteral("misc/ossCopied")).toBool())
        return;

    kajekaSettings.setValue(QStringLiteral("misc/ossCopied"), true);

    auto keys = kajekaSettings.allKeys();
    for(const auto& key : keys)
    {
        // The auth group is handled specially, see below
        if(key.startsWith(QStringLiteral("auth")))
            continue;

        // This system has changed significantly, and we don't want to carry it over anyway
        if(key == QStringLiteral("misc/update"))
            continue;

        targetSettings.setValue(key, kajekaSettings.value(key));
    }

    // Convert auth section to tracking section
    if(kajekaSettings.contains(QStringLiteral("auth/emailAddress")))
    {
        targetSettings.setValue(QStringLiteral("tracking/emailAddress"),
            kajekaSettings.value(QStringLiteral("auth/emailAddress")));

        targetSettings.setValue(QStringLiteral("tracking/permission"),
            QStringLiteral("given"));
    }
}

void u::definePref(const QString& key, const QVariant& defaultValue)
{
    if(!u::prefExists(key))
        u::setPref(key, defaultValue);
}

QVariant u::pref(const QString& key)
{
    u::definePref(key);

    return settings().value(key);
}

void u::setPref(const QString& key, const QVariant& value)
{
    auto s = settings();
    bool changed = (value != s.value(key));

    s.setValue(key, value);

    if(changed)
        PreferencesWatcher::setPref(key, value);
}

bool u::prefExists(const QString& key)
{
    return settings().contains(key);
}

QmlPreferences::QmlPreferences(QObject* parent) :
    QObject(parent)
{
    connect(&_watcher, &PreferencesWatcher::preferenceChanged, this, &QmlPreferences::onPreferenceChanged);
}

QString QmlPreferences::section() const
{
    return _section;
}

void QmlPreferences::setSection(const QString& section)
{
    if(_section != section)
    {
        _section = section;

        if(_initialised)
            load();

        emit sectionChanged();
    }
}

template<typename Fn> static void forEachProperty(const QObject* o, Fn fn)
{
    const QMetaObject *mo = o->metaObject();
    const int offset = mo->propertyOffset();
    const int count = mo->propertyCount();
    for(int i = offset; i < count; i++)
        fn(mo->property(i));
}

void QmlPreferences::componentComplete()
{
    if(!_initialised)
    {
        load();

        // Connect all the property notify signals so we know when they change
        forEachProperty(this,
        [this](const QMetaProperty& property)
        {
            if(property.hasNotifySignal())
            {
                auto index = metaObject()->indexOfSlot("onPropertyChanged()");
                auto slot = metaObject()->method(index);
                QObject::connect(this, property.notifySignal(), this, slot);
            }
        });

        _initialised = true;
    }
}

QString QmlPreferences::preferenceNameByPropertyName(const QString& propertyName)
{
    return QStringLiteral("%1/%2").arg(_section, propertyName);
}

QMetaProperty QmlPreferences::propertyByName(const QString& propertyName) const
{
    if(!propertyName.isEmpty())
    {
        auto index = metaObject()->indexOfProperty(propertyName.toLatin1());
        if(index >= 0)
            return metaObject()->property(index);
    }

    return {};
}

static QString propertyNameFrom(const QString& preferenceName)
{
    auto sepRegex = QRegularExpression(QStringLiteral(R"(\/)"));
    if(preferenceName.contains(sepRegex))
    {
        auto stringList = preferenceName.split(sepRegex);
        if(stringList.size() == 2)
            return stringList.at(1);
    }

    return preferenceName;
}

QMetaProperty QmlPreferences::propertyFrom(const QString& preferenceName)
{
    return propertyByName(propertyNameFrom(preferenceName));
}

void QmlPreferences::setProperty(QMetaProperty property, const QVariant& value)
{
    if(!property.isValid())
        return;

    const QVariant previousValue = property.read(this);

    if((previousValue != value && value.canConvert(previousValue.type())) ||
       !previousValue.isValid())
    {
        property.write(this, value);
    }
}

void QmlPreferences::load()
{
    forEachProperty(this,
    [this](const QMetaProperty& property)
    {
        setProperty(property, u::pref(preferenceNameByPropertyName(property.name())));
    });
}

void QmlPreferences::onPreferenceChanged(const QString& key, const QVariant& value)
{
    setProperty(propertyFrom(key), value);
}

void QmlPreferences::onPropertyChanged()
{
    auto metaMethodName = metaObject()->method(senderSignalIndex()).name();
    QMetaProperty changedProperty;

    forEachProperty(this,
    [&metaMethodName, &changedProperty](const QMetaProperty& property)
    {
        if(property.notifySignal().name() == metaMethodName)
            changedProperty = property;
    });

    if(changedProperty.isValid())
    {
        u::setPref(preferenceNameByPropertyName(changedProperty.name()),
            changedProperty.read(this));
    }
}

PreferencesWatcher::Watchers PreferencesWatcher::_watchers;

PreferencesWatcher::PreferencesWatcher()
{
    std::unique_lock<std::recursive_mutex> lock(PreferencesWatcher::_watchers._mutex);
    PreferencesWatcher::_watchers._set.insert(this);
}

PreferencesWatcher::~PreferencesWatcher()
{
    std::unique_lock<std::recursive_mutex> lock(PreferencesWatcher::_watchers._mutex);
    PreferencesWatcher::_watchers._set.erase(this);
}

void PreferencesWatcher::setPref(const QString& key, const QVariant& value)
{
    std::unique_lock<std::recursive_mutex> lock(PreferencesWatcher::_watchers._mutex);

    for(const auto* watcher : PreferencesWatcher::_watchers._set)
        emit watcher->preferenceChanged(key, value);
}
