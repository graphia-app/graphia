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

#ifndef TEXTSIZEVISUALISATIONCHANNEL_H
#define TEXTSIZEVISUALISATIONCHANNEL_H

#include "visualisationchannel.h"

#include <QObject>
#include <QString>

#include <memory>

using namespace Qt::Literals::StringLiterals;

class TextSizeVisualisationChannel : public VisualisationChannel
{
public:
    using VisualisationChannel::apply;

    void apply(double value, ElementVisual& elementVisual) const override;
};

class TextSizeVisualisationChannelFactory : public VisualisationChannelFactory
{
public:
    QString name() const override { return u"Text Size"_s; }
    QString displayName() const override { return QObject::tr("Text Size"); }

    bool supports(ValueType type) const override { return type == ValueType::Int || type == ValueType::Float; }

    QString description(ElementType, ValueType) const override;

    std::unique_ptr<VisualisationChannel> create(const VisualisationConfig&) const override;
};

#endif // TEXTSIZEVISUALISATIONCHANNEL_H
