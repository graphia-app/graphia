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

#ifndef LAYOUTSETTINGS_H
#define LAYOUTSETTINGS_H

#include "shared/utils/utils.h"

#include <QObject>
#include <QString>

#include <vector>
#include <utility>
#include <algorithm>

enum class LayoutSettingScaleType { Linear, Log };

class LayoutSetting
{
public:
    LayoutSetting(const QString& name, const QString& displayName, const QString& description,
                  float minimumValue, float maximumValue, float defaultValue,
                  LayoutSettingScaleType scaleType = LayoutSettingScaleType::Linear) :
        _name(name),
        _displayName(displayName),
        _description(description),
        _minimumValue(minimumValue),
        _maximumValue(maximumValue),
        _defaultValue(defaultValue),
        _value(defaultValue),
        _scaleType(scaleType)
    {}

    float value() const { return _value; }
    float normalisedValue() const;
    float minimumValue() const { return _minimumValue; }
    float maximumValue() const { return _maximumValue; }
    float defaultValue() const { return _defaultValue; }
    float range() const { return _maximumValue - _minimumValue; }
    bool setValue(float value);
    bool setNormalisedValue(float normalisedValue);
    bool resetValue() { return setValue(_defaultValue); }
    const QString& name() const { return _name; }
    const QString& displayName() const { return _displayName; }
    const QString& description() const { return _description; }

private:
    QString _name;
    QString _displayName;
    QString _description;

    float _minimumValue = 0.0f;
    float _maximumValue = 1.0f;
    float _defaultValue = 0.0f;
    float _value = 0.0f;

    LayoutSettingScaleType _scaleType = LayoutSettingScaleType::Linear;
};

class LayoutSettings : public QObject
{
    Q_OBJECT

private:
    std::vector<LayoutSetting> _settings;

public:
    float value(const QString& name) const;
    float normalisedValue(const QString& name) const;
    void setValue(const QString& name, float value);
    void setNormalisedValue(const QString& name, float normalisedValue);
    void resetValue(const QString& name);

    const LayoutSetting* setting(const QString& name) const;
    LayoutSetting* setting(const QString& name);

    std::vector<LayoutSetting>& vector() { return _settings; }

    template<typename... Args>
    void registerSetting(Args... args)
    {
        _settings.emplace_back(args...);
    }

signals:
    void settingChanged(const QString& name, float value);
};

#endif // LAYOUTSETTINGS_H
