/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#ifndef WEBSEARCHPLUGIN_H
#define WEBSEARCHPLUGIN_H

#include "shared/plugins/basegenericplugin.h"

using namespace Qt::Literals::StringLiterals;

class WebSearchPluginInstance : public BaseGenericPluginInstance
{
    Q_OBJECT
};

class WebSearchPlugin : public BaseGenericPlugin, PluginInstanceProvider<WebSearchPluginInstance>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginIID(WebSearch) FILE "WebsearchPlugin.json")

public:
    WebSearchPlugin();

    QString name() const override { return u"WebSearch"_s; }
    QString description() const override
    {
        return tr("An embedded web browser that searches for the node selection using a URL template.");
    }
    QString imageSource() const override { return u"qrc:///globe.svg"_s; }
    int dataVersion() const override { return 1; }
    QString qmlPath() const override { return u"qrc:///qml/WebsearchPlugin.qml"_s; }
};

Q_DECLARE_INTERFACE(WebSearchPlugin, IPluginIID(WebSearch)) // NOLINT cppcoreguidelines-pro-type-const-cast

#endif // WEBSEARCHPLUGIN_H
