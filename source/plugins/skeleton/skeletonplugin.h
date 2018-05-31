#ifndef SKELETONPLUGIN_H
#define SKELETONPLUGIN_H

#include "shared/plugins/baseplugin.h"
#include "shared/loading/iparser.h"

#include <memory>

class SkeletonPluginInstance : public BasePluginInstance
{
    Q_OBJECT

public:
    std::unique_ptr<IParser> parserForUrlTypeName(const QString& urlTypeName) override;
};

class SkeletonPlugin : public BasePlugin, public PluginInstanceProvider<SkeletonPluginInstance>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginIID FILE "skeletonplugin.json")

public:
    QString name() const override { return QStringLiteral("Skeleton"); }
    QString description() const override
    {
        return tr("An example plugin that does nothing. This serves as "
                  "a template for developers creating new plugins.");
    }

    int dataVersion() const override { return 1; }

    QStringList identifyUrl(const QUrl& url) const override;
    QString failureReason(const QUrl&) const override { return {}; }

    bool editable() const override { return true; }
    QString qmlPath() const override { return QStringLiteral("qrc:///qml/skeletonplugin.qml"); }
};

#endif // SKELETONPLUGIN_H
