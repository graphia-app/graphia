#ifndef VISUALISATIONCONFIG_H
#define VISUALISATIONCONFIG_H

#include <QString>
#include <QVariantMap>

#include <vector>
#include <variant>

struct VisualisationConfig
{
    using ParameterValue = std::variant<double, QString>;
    struct Parameter
    {
        QString _name;
        ParameterValue _value;

        bool operator==(const Parameter& other) const;
        QString valueAsString(bool addQuotes = false) const;
    };

    std::vector<QString> _flags;
    QString _attributeName;
    QString _channelName;
    std::vector<Parameter> _parameters;

    QVariantMap asVariantMap() const;
    QString asString() const;

    bool operator==(const VisualisationConfig& other) const;
    bool operator!=(const VisualisationConfig& other) const;
    bool isFlagSet(const QString& flag) const;
};

#endif // VISUALISATIONCONFIG_H
