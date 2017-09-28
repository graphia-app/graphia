#include "visualisationconfig.h"

#include "shared/utils/utils.h"

bool VisualisationConfig::Parameter::operator==(const VisualisationConfig::Parameter& other) const
{
    return _name == other._name &&
            _value == other._value;
}

QString VisualisationConfig::Parameter::valueAsString(bool addQuotes) const
{
    struct Visitor
    {
        bool _addQuotes;

        explicit Visitor(bool addQuotes_) : _addQuotes(addQuotes_) {}

        QString operator()(double d) const { return QString::number(d); }
        QString operator()(const QString& s) const
        {
            if(_addQuotes)
            {
                QString escapedString = s;
                escapedString.replace(R"(")", R"(\")");

                return QString(R"("%1")").arg(escapedString);
            }

            return s;
        }
    };

    return boost::apply_visitor(Visitor(addQuotes), _value);
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

    QVariantMap parameters;
    for(const auto& parameter : _parameters)
        parameters.insert(parameter._name, parameter.valueAsString(true));
    map.insert("parameters", parameters);

    return map;
}

QString VisualisationConfig::asString() const
{
    QString s;

    if(!_flags.empty())
    {
        s += "[";
        for(const auto& flag : _flags)
        {
            if(s[s.length() - 1] != '[')
                s += ", ";

            s += flag;
        }
        s += "] ";
    }

    s += QString("\"%1\" \"%2\"").arg(_attributeName).arg(_channelName);

    if(!_parameters.empty())
    {
        s += " with ";

        for(const auto& parameter : _parameters)
            s += QString(" %1 = %2").arg(parameter._name).arg(parameter.valueAsString(true));
    }

    return s;
}

bool VisualisationConfig::operator==(const VisualisationConfig& other) const
{
    return _attributeName == other._attributeName &&
           _channelName == other._channelName &&
           !u::setsDiffer(_parameters, other._parameters) &&
           !u::setsDiffer(_flags, other._flags);
}

bool VisualisationConfig::operator!=(const VisualisationConfig& other) const
{
    return !operator==(other);
}

bool VisualisationConfig::isFlagSet(const QString& flag) const
{
    return u::contains(_flags, flag);
}
