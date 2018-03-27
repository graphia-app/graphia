#include "skeletonplugin.h"

std::unique_ptr<IParser> SkeletonPluginInstance::parserForUrlTypeName(const QString& /*urlTypeName*/)
{
    return nullptr;
}

QStringList SkeletonPlugin::identifyUrl(const QUrl& url) const
{
    return identifyByExtension(url);
}
