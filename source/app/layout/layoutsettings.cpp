#include "layoutsettings.h"

#include <cmath>

float LayoutSettings::value(const QString& name) const
{
    auto* v = setting(name);
    if(v != nullptr)
        return v->value();

    Q_ASSERT(!"Setting not found");
    return 0.0f;
}

float LayoutSettings::normalisedValue(const QString& name) const
{
    auto* v = setting(name);
    if(v != nullptr)
        return v->normalisedValue();

    Q_ASSERT(!"Setting not found");
    return 0.0f;
}

void LayoutSettings::setValue(const QString& name, float value)
{
    auto* v = setting(name);
    if(v != nullptr)
    {
        v->setValue(value);
        emit settingChanged();
        return;
    }

    Q_ASSERT(!"Setting not found");
}

void LayoutSettings::setNormalisedValue(const QString& name, float normalisedValue)
{
    auto* v = setting(name);
    if(v != nullptr)
    {
        v->setNormalisedValue(normalisedValue);
        emit settingChanged();
        return;
    }

    Q_ASSERT(!"Setting not found");
}

void LayoutSettings::resetValue(const QString& name)
{
    auto* v = setting(name);
    if(v != nullptr)
    {
        v->resetValue();
        emit settingChanged();
        return;
    }

    Q_ASSERT(!"Setting not found");
}

const LayoutSetting* LayoutSettings::setting(const QString& name) const
{
    auto* mutableThis = const_cast<LayoutSettings*>(this); // NOLINT
    return mutableThis->setting(name);
}

LayoutSetting* LayoutSettings::setting(const QString& name)
{
    auto setting = std::find_if(_settings.begin(), _settings.end(),
    [name](const auto& v)
    {
        return v.name() == name;
    });

    if(setting != _settings.end())
        return &(*setting);

    return nullptr;
}

float LayoutSetting::normalisedValue() const
{
    switch(_scaleType)
    {
    default:
    case LayoutSettingScaleType::Linear:
        return (_value - _minimumValue) / range();

    case LayoutSettingScaleType::Log:
    {
        auto logMin = std::log10(_minimumValue);
        auto logMax = std::log10(_maximumValue);
        auto logRange = logMax - logMin;

        return (std::log10(_value) - logMin) / logRange;
    }
    }
}

void LayoutSetting::setNormalisedValue(float normalisedValue)
{
    switch(_scaleType)
    {
    default:
    case LayoutSettingScaleType::Linear:
        _value = _minimumValue + (normalisedValue * range());
        break;

    case LayoutSettingScaleType::Log:
    {
        auto logMin = std::log10(_minimumValue);
        auto logMax = std::log10(_maximumValue);
        auto logRange = logMax - logMin;

        _value = std::pow(10.0f, logMin + (normalisedValue * logRange));
        break;
    }
    }
}
