#ifndef SAVERFACTORY_H
#define SAVERFACTORY_H

#include "isaver.h"
#include "ui/document.h"

template<class SaverType>
class SaverFactory : public ISaverFactory
{
    std::unique_ptr<ISaver> create(const QUrl& url, Document* document, const IPluginInstance*,
                                   const QByteArray&, const QByteArray&) override
    {
        return std::make_unique<SaverType>(url, document->graphModel());
    }
    QString name() const override { return SaverType::name(); }
    QString extension() const override { return SaverType::extension(); }
};

#endif // SAVERFACTORY_H
