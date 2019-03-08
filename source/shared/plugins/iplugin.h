#ifndef IPLUGIN_H
#define IPLUGIN_H

#include "shared/loading/iurltypes.h"
#include "shared/loading/iparser.h"

#include "shared/utils/failurereason.h"

#include <QtPlugin>
#include <QString>
#include <QStringList>
#include <QByteArray>

#include <memory>

class IPlugin;
class IDocument;
class IParserThread;
class IMutableGraph;
class QUrl;

class IPluginInstance : public FailureReason
{
public:
    virtual ~IPluginInstance() = default;

    virtual void initialise(const IPlugin* plugin, IDocument* document,
                            const IParserThread* parserThread) = 0;
    virtual std::unique_ptr<IParser> parserForUrlTypeName(const QString& urlTypeName) = 0;

    virtual void applyParameter(const QString& name, const QVariant& value) = 0;

    virtual QStringList defaultTransforms() const = 0;
    virtual QStringList defaultVisualisations() const = 0;

    virtual QByteArray save(IMutableGraph& mutableGraph, Progressable& progressable) const = 0;
    virtual bool load(const QByteArray& data, int dataVersion,
        IMutableGraph& mutableGraph, IParser& parser) = 0;

    virtual const IPlugin* plugin() = 0;
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
    ~IPlugin() override = default;

    virtual QString name() const = 0;
    virtual QString description() const = 0;
    virtual QString imageSource() const = 0; // Displayed in the about dialog

    virtual int dataVersion() const = 0;

    virtual QStringList identifyUrl(const QUrl& url) const = 0;
    virtual QString failureReason(const QUrl& url) const = 0;

    virtual bool editable() const = 0;
    virtual bool directed() const = 0;

    virtual QString parametersQmlPath() const = 0;
    virtual QString qmlPath() const = 0;
};

#define IPluginIID "com.kajeka.IPlugin/" VERSION
Q_DECLARE_INTERFACE(IPlugin, IPluginIID)

#endif // IPLUGIN_H
