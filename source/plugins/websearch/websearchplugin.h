#ifndef WEBSEARCHPLUGIN_H
#define WEBSEARCHPLUGIN_H

#include "shared/plugins/basegenericplugin.h"

class WebSearchPluginInstance : public BaseGenericPluginInstance
{
    Q_OBJECT
};

class WebSearchPlugin : public BaseGenericPlugin, PluginInstanceProvider<WebSearchPluginInstance>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginIID FILE "WebsearchPlugin.json")

public:
    QString name() const { return "WebSearch"; }
    QString description() const
    {
        return tr("An embedded web browser that searches for the "
                  "node selection using a URL template.");
    }
    QString imageSource() const { return "qrc:///globe.svg"; }
    int dataVersion() const { return 1; }
    QString qmlPath() const { return "qrc:///qml/WebsearchPlugin.qml"; }
};

#endif // WEBSEARCHPLUGIN_H
