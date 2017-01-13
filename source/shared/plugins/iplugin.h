#ifndef IPLUGIN_H
#define IPLUGIN_H

#include "shared/loading/iurltypes.h"

#include <QtPlugin>
#include <QString>
#include <QStringList>

#include <memory>

class IGraphModel;
class ISelectionManager;
class IParserThread;
class IParser;
class QUrl;

class IPluginInstance
{
public:
    virtual ~IPluginInstance() = default;

    virtual void initialise(IGraphModel* graphModel, ISelectionManager* selectionManager, const IParserThread* parserThread) = 0;
    virtual std::unique_ptr<IParser> parserForUrlTypeName(const QString& urlTypeName) = 0;
    virtual void applyParameter(const QString& name, const QString& value) = 0;
    virtual QStringList defaultTransforms() const = 0;
};

class IPluginInstanceProvider
{
public:
    virtual ~IPluginInstanceProvider() = default;

    virtual std::unique_ptr<IPluginInstance> createInstance() = 0;
};

class IPlugin : public virtual IUrlTypes, public virtual IPluginInstanceProvider
{
public:
    virtual ~IPlugin() = default;

    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QString imageSource() const = 0; // Displayed in the about dialog

    virtual QStringList identifyUrl(const QUrl& url) const = 0;

    virtual bool editable() const = 0;

    virtual QString parametersQmlPath() const = 0;
    virtual QString qmlPath() const = 0;
};

#define IPluginIID "com.kajeka.IPlugin/" VERSION
Q_DECLARE_INTERFACE(IPlugin, IPluginIID)

#endif // IPLUGIN_H
