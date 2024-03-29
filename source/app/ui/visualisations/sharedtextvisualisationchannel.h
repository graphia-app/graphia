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

#ifndef SHAREDTEXTVISUALISATIONCHANNEL_H
#define SHAREDTEXTVISUALISATIONCHANNEL_H

#include "visualisationchannel.h"

class SharedTextVisualisationChannel : public VisualisationChannel
{
public:
    using VisualisationChannel::VisualisationChannel;

    // This visualisation is not applied to individual elements
    void apply(double, ElementVisual&) const override {}
    void apply(const QString&, ElementVisual&) const override {}

    TextVisuals textVisuals(const QString& attributeName,
        const GraphModel& graphModel, const TransformedGraph&) override;

    bool appliesTo(ElementType elementType) const override { return elementType & ElementType::Node; }
    bool supports(ValueType valueType) const override { return valueType == ValueType::String; }
    bool requiresRange() const override { return false; }
    bool allowsMapping() const override { return false; }

    QString description(ElementType, ValueType) const override;
};

#endif // SHAREDTEXTVISUALISATIONCHANNEL_H
