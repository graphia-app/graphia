#include "visualisationconfig.h"

#include "shared/utils/utils.h"

bool VisualisationConfig::Parameter::operator==(const VisualisationConfig::Parameter& other) const
{
    return _name == other._name &&
            _value == other._value;
}

QString VisualisationConfig::Parameter::valueAsString() const
{
    struct Visitor
    {
        QString operator()(double d) const { return QString::number(d); }
        QString operator()(const QString& s) const { return s; }
    };

    return boost::apply_visitor(Visitor(), _value);
}

QVariantMap VisualisationConfig::asVariantMap() const
{
    QVariantMap map;

    QVariantList flags;
    for(const auto& flag : _flags)
        flags.append(flag);
    map.insert("flags", flags);

    map.insert("attribute", _attributeName);
    map.insert("channel", _channelName);

    QVariantList parameters;
    for(const auto& parameter : _parameters)
    {
        QVariantMap parameterObject;
        parameterObject.insert(parameter._name, parameter.valueAsString());
        parameters.append(parameterObject);
    }
    map.insert("parameters", parameters);

    return map;
}

bool VisualisationConfig::operator==(const VisualisationConfig& other) const
{
    return _attributeName == other._attributeName &&
            _channelName == other._channelName;
}

bool VisualisationConfig::operator!=(const VisualisationConfig& other) const
{
    return !operator==(other);
}

bool VisualisationConfig::isFlagSet(const QString& flag) const
{
    return u::contains(_flags, flag);
}
