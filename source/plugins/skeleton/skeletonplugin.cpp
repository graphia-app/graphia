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
