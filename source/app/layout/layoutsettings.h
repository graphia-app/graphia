#ifndef LAYOUTSETTINGS_H
#define LAYOUTSETTINGS_H

#include <QObject>
#include <QString>

#include <vector>

class LayoutSetting : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString settingName MEMBER _displayName NOTIFY nameChanged)
    Q_PROPERTY(float settingValue MEMBER _value NOTIFY valueChanged)
    Q_PROPERTY(float settingMinimumValue MEMBER _minimumValue NOTIFY minimumChanged)
    Q_PROPERTY(float settingMaximumValue MEMBER _maximumValue NOTIFY maximumChanged)

public:
    LayoutSetting() {}
    LayoutSetting(QString name, QString displayName,
                  float minimumValue, float maximumValue, float defaultValue);

    LayoutSetting(const LayoutSetting& other);
    LayoutSetting& operator=(const LayoutSetting& other);

    float value() const { return _value; }
    const QString& name() const { return _name; }

private:
    QString _name;
    QString _displayName;
    float _minimumValue = 0.0f;
    float _maximumValue = 1.0f;
    float _value = 0.0f;

signals:
    void nameChanged();
    void valueChanged();
    void minimumChanged();
    void maximumChanged();
};

class LayoutSettings : public QObject
{
    Q_OBJECT

public:
    float valueOf(const QString& name) const;
    std::vector<LayoutSetting>& vector() { return _settings; }

    template<typename... Args>
    void registerSetting(Args&&... args)
    {
        _settings.emplace_back(args...);
    }

    void finishRegistration();

private:
    std::vector<LayoutSetting> _settings;

signals:
    void settingChanged();
};

#endif // LAYOUTSETTINGS_H
