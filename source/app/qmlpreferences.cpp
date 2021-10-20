#include "qmlpreferences.h"

#include "app/preferences.h"

#include "shared/utils/static_block.h"

#include <QRegularExpression>
#include <QQmlEngine>

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

static_block
{
    qmlRegisterType<QmlPreferences>(
        APP_URI, APP_MAJOR_VERSION, APP_MINOR_VERSION, "Preferences");
}
