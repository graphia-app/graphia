/* Copyright © 2013-2022 Graphia Technologies Ltd.
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
    QString asString(bool forDisplay = false) const;

    QString parameterValue(const QString& name) const;

    bool equals(const VisualisationConfig& other) const;
    bool isFlagSet(const QString& flag) const;
};

#endif // VISUALISATIONCONFIG_H
