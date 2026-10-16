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

#ifndef COLORVISUALISATIONCHANNEL_H
#define COLORVISUALISATIONCHANNEL_H

#include "visualisationchannel.h"

#include "shared/ui/visualisations/colorgradient.h"
#include "shared/ui/visualisations/colorpalette.h"

#include <QObject>
#include <QString>
#include <QVariantMap>

#include <memory>

using namespace Qt::Literals::StringLiterals;

class ColorVisualisationChannel : public VisualisationChannel
{
public:
    explicit ColorVisualisationChannel(const VisualisationConfig& visualisationConfig);

    using VisualisationChannel::apply;

    void apply(double value, ElementVisual& elementVisual) const override;
    void apply(const QString& value, ElementVisual& elementVisual) const override;

protected:
    ColorGradient _colorGradient;
    ColorPalette _colorPalette;
};

class ColorVisualisationChannelFactory : public VisualisationChannelFactory
{
public:
    QString name() const override { return u"Colour"_s; }
    QString displayName() const override { return QObject::tr("Colour"); }

    bool supports(ValueType valueType) const override { return valueType != ValueType::Unknown; }

    QString description(ElementType, ValueType) const override;

    QVariantMap defaultParameters(ValueType valueType) const override;

    std::unique_ptr<VisualisationChannel> create(const VisualisationConfig& visualisationConfig) const override;
};

#endif // COLORVISUALISATIONCHANNEL_H
