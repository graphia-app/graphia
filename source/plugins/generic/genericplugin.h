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

#ifndef GENERICPLUGIN_H
#define GENERICPLUGIN_H

#include "shared/plugins/basegenericplugin.h"

using namespace Qt::Literals::StringLiterals;

class GenericPluginInstance : public BaseGenericPluginInstance
{
    Q_OBJECT
};

class GenericPlugin : public BaseGenericPlugin, public PluginInstanceProvider<GenericPluginInstance>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginIID(Generic) FILE "GenericPlugin.json")

public:
    QString name() const override { return u"Generic"_s; }
    QString description() const override
    {
        return tr("A plugin that loads generic graphs from a variety of file formats.");
    }
    QString imageSource() const override { return u"qrc:///qt/qml/Graphia/Plugins/Generic/tools.svg"_s; }
    int dataVersion() const override { return 3; }
    QString qmlModule() const override { return u"Graphia.Plugins.Generic"_s; }
};

Q_DECLARE_INTERFACE(GenericPlugin, IPluginIID(Generic))

#endif // GENERICPLUGIN_H
