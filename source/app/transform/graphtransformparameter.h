/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#ifndef GRAPHTRANSFORMPARAMETER_H
#define GRAPHTRANSFORMPARAMETER_H

#include "shared/attributes/valuetype.h"

#include <QString>
#include <QVariant>

#include <limits>
#include <map>
#include <utility>

class GraphTransformParameter
{
public:
    GraphTransformParameter() = default;
    explicit GraphTransformParameter(const QString& name) : _name(name) {}

    QString name() const { return _name; }
    ValueType type() const { return _type; }
    QString description() const { return _description; }

    GraphTransformParameter& setName(const QString& name) { _name = name; return *this; }
    GraphTransformParameter& setType(ValueType type) { _type = type; return *this; }
    GraphTransformParameter& setDescription(const QString& description) { _description = description; return *this; }

    QVariant initialValue() const { return _initialValue; }
    GraphTransformParameter& setInitialValue(const QVariant& initialValue) { _initialValue = initialValue; return *this; }

    double min() const { return _min; }
    double max() const { return _max; }

    GraphTransformParameter& setMin(double min) { _min = min; return *this; }
    GraphTransformParameter& setMax(double max) { _max = max; return *this; }
    GraphTransformParameter& setRange(double min, double max) { _min = min; _max = max; return *this; }

    QString validatorRegex() const { return _validatorRegex; }
    GraphTransformParameter& setValidatorRegex(const QString& validatorRegex) { _validatorRegex = validatorRegex; return *this; }

    bool hasMin() const { return _min != std::numeric_limits<double>::max(); }
    bool hasMax() const { return _max != std::numeric_limits<double>::lowest(); }
    bool hasRange() const { return hasMin() && hasMax(); }

    static GraphTransformParameter create(const QString& name) { return GraphTransformParameter(name); }

private:
    QString _name;
    ValueType _type = ValueType::Unknown;
    QString _description;
    QVariant _initialValue;
    double _min = std::numeric_limits<double>::max();
    double _max = std::numeric_limits<double>::lowest();
    QString _validatorRegex;
};

using GraphTransformParameters = std::vector<GraphTransformParameter>;

#endif // GRAPHTRANSFORMPARAMETER_H
