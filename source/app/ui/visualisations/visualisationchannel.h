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

#ifndef VISUALISATIONCHANNEL_H
#define VISUALISATIONCHANNEL_H

#include "ui/visualisations/elementvisual.h"
#include "ui/visualisations/textvisual.h"
#include "shared/attributes/valuetype.h"
#include "shared/graph/elementtype.h"

#include <QString>
#include <QVariantMap>

#include <vector>
#include <map>

class VisualisationInfo;
class GraphModel;
class TransformedGraph;

class VisualisationChannel
{
public:
    virtual ~VisualisationChannel() = default;

    virtual void apply(double, ElementVisual&) const { qFatal("apply not implemented"); }
    virtual void apply(const QString&, ElementVisual&) const { qFatal("apply not implemented"); }

    virtual TextVisuals textVisuals(const QString& /*attributeName*/,
        const GraphModel&, const TransformedGraph&) { return {}; }

    virtual bool appliesTo(ElementType) const { return true; }
    virtual bool supports(ValueType) const = 0;
    virtual bool requiresRange() const { return true; }
    virtual bool allowsMapping() const { return true; }

    virtual void findErrors(ElementType, VisualisationInfo&) const {}

    virtual QString description(ElementType, ValueType) const { return {}; }

    virtual void reset();
    virtual QVariantMap defaultParameters(ValueType) const { return {}; }
    virtual void setParameter(const QString& /*name*/, const QString& /*value*/) {}

    const std::vector<QString>& values() const { return _values; }
    void addValue(const QString& value);
    int indexOf(const QString& value) const;

private:
    std::vector<QString> _values;
    std::map<QString, size_t> _valueIndexMap;
};

#endif // VISUALISATIONCHANNEL_H
