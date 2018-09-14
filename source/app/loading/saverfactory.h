#ifndef SAVERFACTORY_H
#define SAVERFACTORY_H

#include "isaver.h"
#include "ui/document.h"

template<class SaverT>
class SaverFactory : public ISaverFactory
{
    std::unique_ptr<ISaver> create(const QUrl& url, Document* document,
                                           const IPluginInstance*, const QByteArray&,
                                           const QByteArray&)
    {
        return std::make_unique<SaverT>(url, document->graphModel());
    }
    virtual QString name() const = 0;
    virtual QString extension() const = 0;
};

#endif // SAVERFACTORY_H
