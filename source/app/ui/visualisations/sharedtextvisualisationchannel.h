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

#ifndef SHAREDTEXTVISUALISATIONCHANNEL_H
#define SHAREDTEXTVISUALISATIONCHANNEL_H

#include "visualisationchannel.h"

#include <QObject>
#include <QString>

#include <memory>

using namespace Qt::Literals::StringLiterals;

class SharedTextVisualisationChannel : public VisualisationChannel
{
public:
    // This visualisation is not applied to individual elements
    void apply(double, ElementVisual&) const override {}
    void apply(const QString&, ElementVisual&) const override {}

    TextVisuals textVisuals(const QString& attributeName,
        const GraphModel& graphModel, const TransformedGraph&) override;
};

class SharedTextVisualisationChannelFactory : public VisualisationChannelFactory
{
public:
    QString name() const override { return u"SharedText"_s; }
    QString displayName() const override { return QObject::tr("Shared Text"); }

    bool appliesTo(ElementType elementType) const override { return elementType & ElementType::Node; }
    bool showsValuesAsText() const override { return true; }
    bool supports(ValueType valueType) const override { return valueType == ValueType::String; }
    bool requiresRange() const override { return false; }
    bool allowsMapping() const override { return false; }

    QString description(ElementType, ValueType) const override;

    std::unique_ptr<VisualisationChannel> create(const VisualisationConfig&) const override;
};

#endif // SHAREDTEXTVISUALISATIONCHANNEL_H
