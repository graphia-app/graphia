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

#include "textvisualisationchannel.h"
#include "visualisationinfo.h"

#include "shared/utils/string.h"

#include "app/rendering/graphrenderer.h"

#include "app/preferences.h"

#include <QObject>

using namespace Qt::Literals::StringLiterals;

void TextVisualisationChannel::apply(double value, ElementVisual& elementVisual) const
{
    elementVisual._text = u::formatNumberScientific(value);
}

void TextVisualisationChannel::apply(const QString& value, ElementVisual& elementVisual) const
{
    elementVisual._text = value;
}

void TextVisualisationChannel::findErrors(ElementType elementType, VisualisationInfo& info) const
{
    if(elementType == ElementType::Edge && u::pref(u"visuals/showEdgeText"_s).toInt() == static_cast<int>(TextState::Off))
        info.addAlert(AlertType::Warning, QObject::tr("Edge Text Disabled"));
}

QString TextVisualisationChannel::description(ElementType, ValueType) const
{
    return QObject::tr("The attribute will be visualised as text.");
}
