#ifndef LOADER_H
#define LOADER_H

#include "shared/loading/iparser.h"
#include "shared/plugins/iplugin.h"

#include <QString>

class Loader : public IParser
{
private:
    IPluginInstance *_pluginInstance = nullptr;

public:
    bool parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progressFn);
    void setPluginInstance(IPluginInstance* pluginInstance);

    static QString pluginNameFor(const QUrl& url);
    static bool canOpen(const QUrl& url);
};

#endif // LOADER_H
