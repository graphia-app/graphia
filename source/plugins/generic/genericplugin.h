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
    GenericPlugin();

    QString name() const override { return u"Generic"_s; }
    QString description() const override
    {
        return tr("A plugin that loads generic graphs from a variety of file formats.");
    }
    QString imageSource() const override { return u"qrc:///tools.svg"_s; }
    int dataVersion() const override { return 3; }
    QString qmlPath() const override { return u"qrc:///qml/GenericPlugin.qml"_s; }
};

Q_DECLARE_INTERFACE(GenericPlugin, IPluginIID(Generic))

#endif // GENERICPLUGIN_H
