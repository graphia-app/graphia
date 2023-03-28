/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#include "layoutsettings.h"

#include <cmath>

#include <QDebug>

using namespace Qt::Literals::StringLiterals;

float LayoutSettings::value(const QString& name) const
{
    const auto* v = setting(name);
    if(v != nullptr)
        return v->value();

    return 0.0f;
}

float LayoutSettings::normalisedValue(const QString& name) const
{
    const auto* v = setting(name);
    if(v != nullptr)
        return v->normalisedValue();

    return 0.0f;
}

void LayoutSettings::setValue(const QString& name, float value)
{
    auto* v = setting(name);
    if(v != nullptr)
    {
        if(v->setValue(value))
            emit settingChanged(name, v->value());
    }
}

void LayoutSettings::setNormalisedValue(const QString& name, float normalisedValue)
{
    auto* v = setting(name);
    if(v != nullptr)
    {
        if(v->setNormalisedValue(normalisedValue))
            emit settingChanged(name, v->value());
    }
}

void LayoutSettings::resetValue(const QString& name)
{
    auto* v = setting(name);
    if(v != nullptr)
    {
        if(v->resetValue())
            emit settingChanged(name, v->value());
    }
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

    qDebug() << u"Setting"_s << name << "not found!";
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

bool LayoutSetting::setValue(float value)
{
    auto newValue = std::clamp(value, _minimumValue, _maximumValue);

    if(newValue != _value)
    {
        _value = newValue;
        return true;
    }

    return false;
}

bool LayoutSetting::setNormalisedValue(float normalisedValue)
{
    float newValue;

    switch(_scaleType)
    {
    default:
    case LayoutSettingScaleType::Linear:
        newValue = _minimumValue + (normalisedValue * range());
        break;

    case LayoutSettingScaleType::Log:
    {
        auto logMin = std::log10(_minimumValue);
        auto logMax = std::log10(_maximumValue);
        auto logRange = logMax - logMin;

        newValue = std::pow(10.0f, logMin + (normalisedValue * logRange));
        break;
    }
    }

    if(newValue != _value)
    {
        _value = newValue;
        return true;
    }

    return false;
}
