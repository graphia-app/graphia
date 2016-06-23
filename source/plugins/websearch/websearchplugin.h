#ifndef WEBSEARCHPLUGIN_H
#define WEBSEARCHPLUGIN_H

#include "shared/interfaces/basegenericplugin.h"

class WebSearchPluginInstance : public BaseGenericPluginInstance
{
    Q_OBJECT
};

class WebSearchPlugin : public BaseGenericPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginIID FILE "websearchplugin.json")

public:
    QString name() const { return "WebSearch"; }
    QString description() const
    {
        return tr("An embedded web browser that searches for the "
                  "node selection using a URL template.");
    }
    QString imageSource() const { return "qrc:///globe.svg"; }

    std::unique_ptr<IPluginInstance> createInstance();

    QString qmlPath() const { return "qrc:///qml/websearchplugin.qml"; }
};

#endif // WEBSEARCHPLUGIN_H
