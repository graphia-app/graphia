#ifndef IEXPORTER_H
#define IEXPORTER_H

#include "shared/utils/progressable.h"
#include "shared/utils/cancellable.h"

class QUrl;
class QString;
class IGraphModel;

class IExporter : public Progressable, public Cancellable
{
public:
    ~IExporter() override = default;

    virtual bool save(const QUrl& url, IGraphModel* graphModel = nullptr) = 0;
    virtual QString name() const = 0;
    virtual QString extension() const = 0;
};

#endif // IEXPORTER_H
