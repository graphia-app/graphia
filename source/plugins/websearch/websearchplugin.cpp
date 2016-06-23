#include "websearchplugin.h"

std::unique_ptr<IPluginInstance> WebSearchPlugin::createInstance()
{
    return std::make_unique<WebSearchPluginInstance>();
}
