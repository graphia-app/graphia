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

#include "baseplugin.h"

void BasePlugin::initialise(const IApplication* application)
{
    _application = application;
}

QString BasePlugin::imageSource() const
{
    // Default empty image
    return {};
}

QString BasePlugin::parametersQmlType(const QString&) const
{
    // Default to no parameters UI
    return {};
}

QString BasePlugin::qmlModule() const
{
    // Default to no UI
    return {};
}

bool BasePlugin::directed() const
{
    // Default to directed graphs
    return true;
}

const IApplication* BasePlugin::application() const
{
    return _application;
}

QObject* BasePlugin::ptr()
{
    return this;
}

QString BasePlugin::displayTextForTransform(const QString& transform) const
{
    return application()->displayTextForTransform(transform);
}

QString BasePlugin::displayTextForVisualisation(const QString& visualisation) const
{
    return application()->displayTextForVisualisation(visualisation);
}
