#ifndef LAYOUTSETTINGS_H
#define LAYOUTSETTINGS_H

#include "shared/utils/utils.h"

#include <QObject>
#include <QString>

#include <vector>
#include <utility>

class LayoutSetting
{
public:
    LayoutSetting(QString name, QString displayName,
                  float minimumValue, float maximumValue, float defaultValue) :
        _name(std::move(name)),
        _displayName(std::move(displayName)),
        _minimumValue(minimumValue),
        _maximumValue(maximumValue),
        _value(defaultValue)
    {}

    float value() const { return _value; }
    float minimumValue() const { return _minimumValue; }
    float maximumValue() const { return _maximumValue; }
    void setValue(float value) { _value = u::clamp(_minimumValue, _maximumValue, value); }
    const QString& name() const { return _name; }
    const QString& displayName() const { return _displayName; }

private:
    QString _name;
    QString _displayName;
    float _minimumValue = 0.0f;
    float _maximumValue = 1.0f;
    float _value = 0.0f;
};

class LayoutSettings : public QObject
{
    Q_OBJECT

private:
    std::vector<LayoutSetting> _settings;

public:
    float value(const QString& name) const;
    void setValue(const QString& name, float value);

    const LayoutSetting* setting(const QString& name) const;
    LayoutSetting* setting(const QString& name);

    std::vector<LayoutSetting>& vector() { return _settings; }

    template<typename... Args>
    void registerSetting(Args&&... args)
    {
        _settings.emplace_back(args...);
    }

signals:
    void settingChanged();
};

#endif // LAYOUTSETTINGS_H
