#include "genericplugin.h"

std::unique_ptr<IPluginInstance> GenericPlugin::createInstance()
{
    return std::make_unique<GenericPluginInstance>();
}
