#ifndef ISaver_H
#define ISaver_H

#include "shared/utils/cancellable.h"
#include "shared/utils/progressable.h"

#include <QByteArray>

class QUrl;
class QString;
class IGraphModel;
class Document;
class IPluginInstance;

class ISaver : public Progressable, public Cancellable
{
public:
    ~ISaver() override = default;

    virtual bool save() = 0;
};

class ISaverFactory
{
public:
    virtual ~ISaverFactory() = default;

    virtual std::unique_ptr<ISaver> create(const QUrl& url, Document* document,
                                           const IPluginInstance* pluginInstance, const QByteArray& uiData,
                                           const QByteArray& pluginUiData);
    virtual QString name() const = 0;
    virtual QString extension() const = 0;
};

#endif // ISaver_H
