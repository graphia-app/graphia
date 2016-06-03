#ifndef IPLUGIN_H
#define IPLUGIN_H

#include <QtPlugin>

class IPlugin
{
public:
    virtual ~IPlugin() = default;
};

#define IPluginIID "com.kajeka.IPlugin/" VERSION
Q_DECLARE_INTERFACE(IPlugin, IPluginIID)

#endif // IPLUGIN_H
