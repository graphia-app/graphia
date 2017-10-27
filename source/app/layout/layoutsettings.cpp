#include "layoutsettings.h"

float LayoutSettings::value(const QString& name) const
{
    auto* v = setting(name);
    if(v != nullptr)
        return v->value();

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

const LayoutSetting* LayoutSettings::setting(const QString& name) const
{
    auto* mutableThis = const_cast<LayoutSettings*>(this);
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
