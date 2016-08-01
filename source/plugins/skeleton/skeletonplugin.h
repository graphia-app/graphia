#ifndef SKELETONPLUGIN_H
#define SKELETONPLUGIN_H

#include "shared/plugins/baseplugin.h"
#include "shared/loading/iparser.h"

#include <memory>

class SkeletonPluginInstance : public BasePluginInstance
{
    Q_OBJECT

public:
    SkeletonPluginInstance();

    std::unique_ptr<IParser> parserForUrlTypeName(const QString& urlTypeName);
};

class SkeletonPlugin : public BasePlugin, public PluginInstanceProvider<SkeletonPluginInstance>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginIID FILE "skeletonplugin.json")

public:
    SkeletonPlugin();

    QString name() const { return "Skeleton"; }
    QString description() const
    {
        return tr("An example plugin that does nothing. This serves as "
                  "a template for developers creating new plugins.");
    }

    QStringList identifyUrl(const QUrl& url) const;

    bool editable() const { return true; }
    QString qmlPath() const { return "qrc:///qml/skeletonplugin.qml"; }
};

#endif // SKELETONPLUGIN_H
