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

#ifndef GRAPHTRANSFORMATTRIBUTEPARAMETER_H
#define GRAPHTRANSFORMATTRIBUTEPARAMETER_H

#include "shared/graph/elementtype.h"
#include "shared/attributes/valuetype.h"

#include <QString>

#include <map>

class GraphTransformAttributeParameter
{
public:
    GraphTransformAttributeParameter() = default;

    GraphTransformAttributeParameter(const QString& name, ElementType elementType,
        ValueType valueType, const QString& description) :
        _name(name), _elementType(elementType),
        _valueType(valueType), _description(description)
    {}

    QString name() const { return _name; }
    ElementType elementType() const { return _elementType; }
    ValueType valueType() const { return _valueType; }
    QString description() const { return _description; }

private:
    QString _name;
    ElementType _elementType = ElementType::None;
    ValueType _valueType = ValueType::Unknown;
    QString _description;
};

using GraphTransformAttributeParameters = std::vector<GraphTransformAttributeParameter>;

#endif // GRAPHTRANSFORMATTRIBUTEPARAMETER_H
