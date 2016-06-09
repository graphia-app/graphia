#ifndef IPLUGIN_H
#define IPLUGIN_H

#include "../loading/iurltypes.h"
#include "../graph/igraphmodelprovider.h"

#include <QtPlugin>
#include <QString>
#include <QStringList>

class IParser;
class QUrl;

class IPlugin : public virtual IUrlTypes, public virtual IGraphModelProvider
{
public:
    virtual ~IPlugin() = default;

    virtual QStringList identifyUrl(const QUrl& url) const = 0;
    virtual IParser* parserForUrlTypeName(const QString& urlTypeName) = 0;

    virtual bool editable() const = 0;
    virtual QString contentQmlPath() const = 0;
};

#define IPluginIID "com.kajeka.IPlugin/" VERSION
Q_DECLARE_INTERFACE(IPlugin, IPluginIID)

#endif // IPLUGIN_H
