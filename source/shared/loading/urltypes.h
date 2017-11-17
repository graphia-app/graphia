#ifndef URLTYPES_H
#define URLTYPES_H

#include "iurltypes.h"

#include <QString>
#include <QStringList>

#include <map>

class QUrl;

class UrlTypes : public virtual IUrlTypes
{
private:
    class UrlType
    {
    public:
        UrlType(const QString& individualDescription,
                const QString& collectiveDescription,
                const QStringList& extensions) :
            _individualDescription(individualDescription),
            _collectiveDescription(collectiveDescription),
            _extensions(extensions)
        {}

        const QString& individualDescription() const { return _individualDescription; }
        const QString& collectiveDescription() const { return _collectiveDescription; }
        const QStringList extensions() const { return _extensions; }

    private:
        QString _individualDescription;
        QString _collectiveDescription;
        QStringList _extensions;
    };

    std::map<QString, UrlType> _urlTypes;

protected:
    void registerUrlType(const QString& urlTypeName,
                         const QString& individualDescription,
                         const QString& collectiveDescription,
                         const QStringList& extensions);

    QStringList identifyByExtension(const QUrl& url) const;

public:
    ~UrlTypes() override = default;

    QStringList loadableUrlTypeNames() const override;
    QString individualDescriptionForUrlTypeName(const QString& urlTypeName) const override;
    QString collectiveDescriptionForUrlTypeName(const QString& urlTypeName) const override;
    QStringList extensionsForUrlTypeName(const QString& urlTypeName) const override;
};

#endif // URLTYPES_H
