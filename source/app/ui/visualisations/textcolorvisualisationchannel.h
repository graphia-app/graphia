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

#ifndef TEXTCOLORVISUALISATIONCHANNEL_H
#define TEXTCOLORVISUALISATIONCHANNEL_H

#include "colorvisualisationchannel.h"

class TextColorVisualisationChannel : public ColorVisualisationChannel
{
public:
    using ColorVisualisationChannel::ColorVisualisationChannel;
    using ColorVisualisationChannel::apply;

    void apply(double value, ElementVisual& elementVisual) const override;
    void apply(const QString& value, ElementVisual& elementVisual) const override;

    QString description(ElementType, ValueType) const override;
};

#endif // TEXTCOLORVISUALISATIONCHANNEL_H
