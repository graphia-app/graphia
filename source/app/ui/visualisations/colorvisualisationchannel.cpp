/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#include "colorvisualisationchannel.h"

#include "app/preferences.h"

#include <QObject>

using namespace Qt::Literals::StringLiterals;

void ColorVisualisationChannel::apply(double value, ElementVisual& elementVisual) const
{
    elementVisual._outerColor = _colorGradient.get(value);
}

void ColorVisualisationChannel::apply(const QString& value, ElementVisual& elementVisual) const
{
    if(value.isEmpty())
        return;

    elementVisual._outerColor = _colorPalette.get(value, indexOf(value));
}

QString ColorVisualisationChannel::description(ElementType elementType, ValueType valueType) const
{
    auto elementTypeString = elementTypeAsString(elementType).toLower();

    switch(valueType)
    {
    case ValueType::Int:
    case ValueType::Float:
        return QString(QObject::tr("The attribute will be visualised by "
                          "continuously varying the colour of the %1 "
                          "using a gradient.")).arg(elementTypeString);

    case ValueType::String:
        return QString(QObject::tr("This visualisation will be applied by "
                          "assigning a colour to the %1 which represents "
                          "unique values of the attribute.")).arg(elementTypeString);

    default:
        break;
    }

    return {};
}

void ColorVisualisationChannel::reset()
{
    VisualisationChannel::reset();

    _colorGradient = {};
    _colorPalette = {};
}

QVariantMap ColorVisualisationChannel::defaultParameters(ValueType valueType) const
{
    QVariantMap parameters;

    switch(valueType)
    {
    case ValueType::Int:
    case ValueType::Float:
        parameters.insert(u"gradient"_s,
            u::pref(u"visuals/defaultGradient"_s).toString());
        break;

    case ValueType::String:
        parameters.insert(u"palette"_s,
            u::pref(u"visuals/defaultPalette"_s).toString());
        break;

    default:
        break;
    }

    return parameters;
}

void ColorVisualisationChannel::setParameter(const QString& name, const QString& value)
{
    if(name == u"gradient"_s)
        _colorGradient = ColorGradient(value);
    else if(name == u"palette"_s)
        _colorPalette = ColorPalette(value);
}
