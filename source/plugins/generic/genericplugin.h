#ifndef GENERICPLUGIN_H
#define GENERICPLUGIN_H

#include "../iplugin.h"

#include <QObject>

class GenericPlugin : public QObject, public IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginIID FILE "genericplugin.json")
    Q_INTERFACES(IPlugin)

public:
    GenericPlugin();
};

#endif // GENERICPLUGIN_H
