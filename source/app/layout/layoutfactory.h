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

#ifndef LAYOUTFACTORY_H
#define LAYOUTFACTORY_H

#include "shared/graph/elementid.h"

#include "nodepositions.h"
#include "layout.h"
#include "layoutsettings.h"

#include <QString>

class GraphModel;

class LayoutFactory
{
protected:
    GraphModel* _graphModel = nullptr;
    LayoutSettings _layoutSettings;

public:
    explicit LayoutFactory(GraphModel* graphModel) :
        _graphModel(graphModel)
    {}

    virtual ~LayoutFactory() = default;

    LayoutSettings& settings()
    {
        return _layoutSettings;
    }

    const LayoutSetting* setting(const QString& name) const
    {
        return _layoutSettings.setting(name);
    }

    void setSettingValue(const QString& name, float value)
    {
        _layoutSettings.setValue(name, value);
    }

    void setSettingNormalisedValue(const QString& name, float normalisedValue)
    {
        _layoutSettings.setNormalisedValue(name, normalisedValue);
    }

    void resetSettingValue(const QString& name)
    {
        _layoutSettings.resetValue(name);
    }

    virtual QString name() const = 0;
    virtual QString displayName() const = 0;
    virtual std::unique_ptr<Layout> create(ComponentId componentId,
        NodeLayoutPositions& results, Layout::Dimensionality dimensionalityMode) = 0;
};

#endif // LAYOUTFACTORY_H
