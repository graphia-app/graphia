/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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
#include <QCoreApplication>
#include <QRegularExpression>
#include <QMetaType>
#include <QtGlobal>

using namespace Qt::Literals::StringLiterals;

namespace
{
    QSettings settings()
    {
#ifdef Q_OS_WASM
        const QSettings::Format format = QSettings::Format::NativeFormat;
#else
        const QSettings::Format format = QSettings::Format::IniFormat;
#endif

        return {format, QSettings::Scope::UserScope,
                QCoreApplication::organizationName(), QCoreApplication::applicationName()};
    }
} // namespace

void u::definePref(const QString& key, const QVariant& defaultValue)
{
    if(!u::prefExists(key))
        u::setPref(key, defaultValue);
}

QVariant u::pref(const QString& key)
{
    return PreferencesStore::instance()->value(key);
}

bool u::prefExists(const QString& key)
{
    return PreferencesStore::instance()->contains(key);
}

bool u::removePref(const QString& key)
{
    if(u::prefExists(key))
    {
        PreferencesStore::instance()->remove(key);
        return true;
    }

    return false;
}

void u::setPref(const QString& key, const QVariant& value)
{
    const bool changed = (value != u::pref(key));

    PreferencesStore::instance()->set(key, value);

    if(changed)
        PreferencesWatcher::setPref(key, value);
}

void u::savePendingPrefs()
{
    PreferencesStore::instance()->savePending();
}

QString u::settingsFileName()
{
    return settings().fileName();
}

PreferencesStore::PreferencesStore()
{
    const auto s = settings();
    const auto keys = s.allKeys();

    for(const auto& key : keys)
        _values.insert({key, s.value(key)});
}

PreferencesStore::~PreferencesStore()
{
    savePending();
}

QVariant PreferencesStore::value(const QString& key) const
{
    const std::unique_lock lock(_mutex);

    if(!_values.contains(key))
        return {};

    return _values.at(key);
}

bool PreferencesStore::contains(const QString& key) const
{
    const std::unique_lock lock(_mutex);
    return _values.contains(key);
}

void PreferencesStore::set(const QString& key, const QVariant& value)
{
    const std::unique_lock lock(_mutex);
    _values.insert_or_assign(key, value);
    markPending(key);
}

void PreferencesStore::remove(const QString& key)
{
    const std::unique_lock lock(_mutex);
    _values.erase(key);
    markPending(key);
}

void PreferencesStore::savePending()
{
    const std::unique_lock lock(_mutex);

    if(_pendingKeys.empty())
        return;

    auto s = settings();

    for(const auto& key : _pendingKeys)
    {
        if(_values.contains(key))
            s.setValue(key, _values.at(key));
        else
            s.remove(key);
    }

    _pendingKeys.clear();
}

void PreferencesStore::markPending(const QString& key)
{
    if(_pendingKeys.empty())
    {
        QMetaObject::invokeMethod(QCoreApplication::instance(),
            [this] { savePending(); }, Qt::QueuedConnection);
    }

    _pendingKeys.insert(key);
}

PreferencesWatcher::Watchers PreferencesWatcher::_watchers;

PreferencesWatcher::PreferencesWatcher()
{
    const std::unique_lock<std::recursive_mutex> lock(PreferencesWatcher::_watchers._mutex);
    PreferencesWatcher::_watchers._set.insert(this);
}

PreferencesWatcher::~PreferencesWatcher()
{
    const std::unique_lock<std::recursive_mutex> lock(PreferencesWatcher::_watchers._mutex);
    PreferencesWatcher::_watchers._set.erase(this);
}

void PreferencesWatcher::setPref(const QString& key, const QVariant& value)
{
    const std::unique_lock<std::recursive_mutex> lock(PreferencesWatcher::_watchers._mutex);

    for(auto* watcher : PreferencesWatcher::_watchers._set)
        emit watcher->preferenceChanged(key, value);
}

Preferences::Preferences(QObject* parent) :
    QObject(parent)
{
    connect(&_watcher, &PreferencesWatcher::preferenceChanged, this, &Preferences::onPreferenceChanged);
}

QString Preferences::section() const
{
    return _section;
}

void Preferences::setSection(const QString& section)
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

void Preferences::componentComplete()
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

QString Preferences::preferenceNameByPropertyName(const QString& propertyName)
{
    return u"%1/%2"_s.arg(_section, propertyName);
}

QMetaProperty Preferences::propertyByName(const QString& propertyName) const
{
    if(!propertyName.isEmpty())
    {
        auto index = metaObject()->indexOfProperty(propertyName.toLatin1().constData());
        if(index >= 0)
            return metaObject()->property(index);
    }

    return {};
}

static QString propertyNameFrom(const QString& preferenceName)
{
    static const QRegularExpression sepRegex(uR"(\/)"_s);
    if(preferenceName.contains(sepRegex))
    {
        auto stringList = preferenceName.split(sepRegex);
        if(stringList.size() == 2)
            return stringList.at(1);
    }

    return preferenceName;
}

QMetaProperty Preferences::propertyFrom(const QString& preferenceName)
{
    return propertyByName(propertyNameFrom(preferenceName));
}

void Preferences::setProperty(QMetaProperty property, const QVariant& value)
{
    if(!property.isValid())
        return;

    const QVariant previousValue = property.read(this);

    if((previousValue != value && QMetaType::canConvert(value.metaType(), previousValue.metaType())) ||
       !previousValue.isValid())
    {
        property.write(this, value); // clazy:exclude=unused-result-check
    }
}

void Preferences::load()
{
    forEachProperty(this,
    [this](const QMetaProperty& property)
    {
        setProperty(property, u::pref(preferenceNameByPropertyName(property.name())));
    });
}

void Preferences::onPreferenceChanged(const QString& key, const QVariant& value)
{
    setProperty(propertyFrom(key), value);
}

void Preferences::onPropertyChanged()
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
