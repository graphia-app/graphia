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

#ifndef FILTERTRANSFORM_H
#define FILTERTRANSFORM_H

#include "app/transform/graphtransform.h"

#include <QObject>
#include <QString>

#include <memory>

using namespace Qt::Literals::StringLiterals;

class FilterTransform : public GraphTransform
{
public:
    explicit FilterTransform(ElementType elementType,
                             const GraphModel& graphModel,
                             bool invert) :
        _elementType(elementType),
        _graphModel(&graphModel),
        _invert(invert)
    {}

    void apply(TransformedGraph& target) override;

private:
    ElementType _elementType;
    const GraphModel* _graphModel;
    bool _invert = false;
};

class FilterTransformFactory : public GraphTransformFactory
{
private:
    ElementType _elementType = ElementType::None;
    bool _invert = false;

public:
    FilterTransformFactory(GraphModel* graphModel, ElementType elementType, bool invert) :
        GraphTransformFactory(graphModel),
        _elementType(elementType), _invert(invert)
    {}

    QString name() const override
    {
        switch(_elementType)
        {
        case ElementType::Node:         return _invert ? u"Keep Nodes"_s : u"Remove Nodes"_s;
        case ElementType::Edge:         return _invert ? u"Keep Edges"_s : u"Remove Edges"_s;
        case ElementType::Component:    return _invert ? u"Keep Components"_s : u"Remove Components"_s;
        default: break;
        }

        return {};
    }

    QString displayName() const override
    {
        switch(_elementType)
        {
        case ElementType::Node:         return _invert ? QObject::tr("Keep Nodes") : QObject::tr("Remove Nodes");
        case ElementType::Edge:         return _invert ? QObject::tr("Keep Edges") : QObject::tr("Remove Edges");
        case ElementType::Component:    return _invert ? QObject::tr("Keep Components") : QObject::tr("Remove Components");
        default: break;
        }

        return {};
    }

    QString description() const override
    {
        return QObject::tr("%1 all the %2s which match the specified condition.")
                .arg(_invert ? QObject::tr("Keep") : QObject::tr("Remove"),
                     elementTypeAsString(_elementType).toLower());
    }
    QString category() const override { return QObject::tr("Filters"); }
    ElementType elementType() const override { return _elementType; }
    bool requiresCondition() const override { return true; }

    std::unique_ptr<GraphTransform> create(const GraphTransformConfig& graphTransformConfig) const override;
};

#endif // FILTERTRANSFORM_H
