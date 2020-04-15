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

#include "shared/utils/container.h"

static void copyKajekaSettings(QSettings& targetSettings)
{
    QSettings kajekaSettings(QStringLiteral("Kajeka"), QStringLiteral("Graphia"));

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

Preferences::Preferences() :
    _settings(QSettings::Format::IniFormat, QSettings::Scope::UserScope,
              QCoreApplication::organizationName(), QCoreApplication::applicationName())
{
    copyKajekaSettings(_settings);
}

void Preferences::define(const QString& key, const QVariant& defaultValue,
                         const QVariant& minimumValue, const QVariant& maximumValue)
{
    if(defaultValue.isValid() && _defaultValue[key] != defaultValue)
        _defaultValue[key] = defaultValue;

    if(minimumValue.isValid() && _minimumValue[key] != minimumValue)
    {
        _minimumValue[key] = minimumValue;
        emit minimumChanged(key, minimumValue);
    }

    if(maximumValue.isValid() && _maximumValue[key] != maximumValue)
    {
        _maximumValue[key] = maximumValue;
        emit maximumChanged(key, maximumValue);
    }

    if(!exists(key))
        set(key, defaultValue);
}

QVariant Preferences::get(const QString& key)
{
    define(key);

    return _settings.value(key);
}

QVariant Preferences::minimum(const QString& key) const
{
    if(u::contains(_minimumValue, key))
        return _minimumValue.at(key);

    return {};
}

QVariant Preferences::maximum(const QString& key) const
{
    if(u::contains(_maximumValue, key))
        return _maximumValue.at(key);

    return {};
}

void Preferences::set(const QString& key, QVariant value, bool notify)
{
    bool changed = (value != _settings.value(key));

    auto minimumValue = minimum(key);
    if(minimumValue.canConvert(value.type()))
    {
        minimumValue.convert(value.type());
        value = std::max(value, minimumValue);
    }

    auto maximumValue = maximum(key);
    if(maximumValue.canConvert(value.type()))
    {
        maximumValue.convert(value.type());
        value = std::min(value, maximumValue);
    }

    _settings.setValue(key, value);

    if(changed && notify)
        emit preferenceChanged(key, value);
}

void Preferences::reset(const QString& key)
{
    set(key, _defaultValue[key]);
}

bool Preferences::exists(const QString& key)
{
    return _settings.contains(key);
}

QmlPreferences::QmlPreferences(QObject* parent) :
    QObject(parent)
{
    connect(S(Preferences), &Preferences::preferenceChanged, this, &QmlPreferences::onPreferenceChanged);
    connect(S(Preferences), &Preferences::minimumChanged, this, &QmlPreferences::onMinimumChanged);
    connect(S(Preferences), &Preferences::maximumChanged, this, &QmlPreferences::onMaximumChanged);
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

void QmlPreferences::reset(const QString& key)
{
    S(Preferences)->reset(QStringLiteral("%1/%2").arg(_section, key));
}

enum class PropertyType
{
    Value,
    Minimum,
    Maximum
};

template<typename Fn> static void forEachProperty(const QObject* o, Fn fn)
{
    const QMetaObject *mo = o->metaObject();
    const int offset = mo->propertyOffset();
    const int count = mo->propertyCount();
    for(int i = offset; i < count; i++)
    {
        QString propertyName(mo->property(i).name());
        PropertyType type;

        if(propertyName.endsWith(QLatin1String("MinimumValue")))
            type = PropertyType::Minimum;
        else if(propertyName.endsWith(QLatin1String("MaximumValue")))
            type = PropertyType::Maximum;
        else
            type = PropertyType::Value;

        fn(mo->property(i), type);
    }
}

void QmlPreferences::componentComplete()
{
    if(!_initialised)
    {
        load();

        // Connect all the property notify signals so we know when they change
        forEachProperty(this,
        [this](const QMetaProperty& property, PropertyType propertyType)
        {
            if(property.hasNotifySignal() && propertyType == PropertyType::Value)
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
    QString canoncicalPropertyName(propertyName);
    canoncicalPropertyName.replace(QRegularExpression(QStringLiteral("M(ax|in)imumValue")), QLatin1String(""));

    return QStringLiteral("%1/%2").arg(_section, canoncicalPropertyName);
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

QMetaProperty QmlPreferences::valuePropertyFrom(const QString& preferenceName)
{
    return propertyByName(propertyNameFrom(preferenceName));
}

QMetaProperty QmlPreferences::minimumPropertyFrom(const QString& preferenceName)
{
    return propertyByName(QStringLiteral("%1MinimumValue").arg(propertyNameFrom(preferenceName)));
}

QMetaProperty QmlPreferences::maximumPropertyFrom(const QString& preferenceName)
{
    return propertyByName(QStringLiteral("%1MaximumValue").arg(propertyNameFrom(preferenceName)));
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
    // Set the minimum and maximum first, if they exist...
    forEachProperty(this,
    [this](const QMetaProperty& property, PropertyType propertyType)
    {
        auto preferenceName = preferenceNameByPropertyName(property.name());
        switch(propertyType)
        {
        case PropertyType::Minimum:
            if(S(Preferences)->exists(preferenceName))
                setProperty(property, S(Preferences)->minimum(preferenceName));
            break;
        case PropertyType::Maximum:
            if(S(Preferences)->exists(preferenceName))
                setProperty(property, S(Preferences)->maximum(preferenceName));
            break;
        default: break;
        }
    });

    // ...then the value
    forEachProperty(this,
    [this](const QMetaProperty& property, PropertyType propertyType)
    {
        if(propertyType == PropertyType::Value)
        {
            setProperty(property, S(Preferences)->get(
                        preferenceNameByPropertyName(property.name())));
        }
    });
}

void QmlPreferences::onPreferenceChanged(const QString& key, const QVariant& value)
{
    setProperty(valuePropertyFrom(key), value);
}

void QmlPreferences::onMinimumChanged(const QString& key, const QVariant& value)
{
    setProperty(minimumPropertyFrom(key), value);
}

void QmlPreferences::onMaximumChanged(const QString& key, const QVariant& value)
{
    setProperty(maximumPropertyFrom(key), value);
}

void QmlPreferences::onPropertyChanged()
{
    auto metaMethodName = metaObject()->method(senderSignalIndex()).name();
    QMetaProperty changedProperty;

    forEachProperty(this,
    [&metaMethodName, &changedProperty](const QMetaProperty& property, PropertyType)
    {
        if(property.notifySignal().name() == metaMethodName)
            changedProperty = property;
    });

    if(changedProperty.isValid())
    {
        S(Preferences)->set(preferenceNameByPropertyName(changedProperty.name()),
            changedProperty.read(this), true);
    }
}
