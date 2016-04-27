#include "layoutsettings.h"
#include "../utils/utils.h"

LayoutSetting::LayoutSetting(const QString& name, const QString& displayName,
                             float minimumValue, float maximumValue, float defaultValue) :
    _name(name),
    _displayName(displayName),
    _minimumValue(minimumValue),
    _maximumValue(maximumValue),
    _value(defaultValue)
{
}

LayoutSetting::LayoutSetting(const LayoutSetting& other) :
    QObject(),
    _name(other._name),
    _displayName(other._displayName),
    _minimumValue(other._minimumValue),
    _maximumValue(other._maximumValue),
    _value(other._value)
{
}

LayoutSetting& LayoutSetting::operator=(const LayoutSetting& other)
{
    if(this != &other)
    {
        _name = other._name;
        _displayName = other._displayName;
        _minimumValue = other._minimumValue;
        _maximumValue = other._maximumValue;
        _value = other._value;
    }

    return *this;
}

float LayoutSettings::valueOf(const QString& name) const
{
    for(auto& setting : _settings)
    {
        if(setting.name() == name)
            return setting.value();
    }

    Q_ASSERT(!"Setting not found");
    return 0.0f;
}

void LayoutSettings::finishRegistration()
{
    for(auto& setting : _settings)
        connect(&setting, &LayoutSetting::valueChanged, this, &LayoutSettings::settingChanged);
}
