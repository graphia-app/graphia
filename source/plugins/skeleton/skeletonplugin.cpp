#include "skeletonplugin.h"

SkeletonPluginInstance::SkeletonPluginInstance()
{
}

std::unique_ptr<IParser> SkeletonPluginInstance::parserForUrlTypeName(const QString& /*urlTypeName*/)
{
    return nullptr;
}

SkeletonPlugin::SkeletonPlugin()
{
    //registerUrlType("Frob", QObject::tr("Frob Files"), {"frb"});
}

QStringList SkeletonPlugin::identifyUrl(const QUrl& url) const
{
    return identifyByExtension(url);
}

std::unique_ptr<IPluginInstance> SkeletonPlugin::createInstance()
{
    return std::make_unique<SkeletonPluginInstance>();
}
