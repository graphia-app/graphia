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
        UrlType(const QString& collectiveDescription,
             const QStringList& extensions) :
            _collectiveDescription(collectiveDescription),
            _extensions(extensions)
        {}

        const QString& collectiveDescription() const { return _collectiveDescription; }
        const QStringList extensions() const { return _extensions; }

    private:
        QString _collectiveDescription;
        QStringList _extensions;
    };

    std::map<QString, UrlType> _urlTypes;

protected:
    void registerUrlType(const QString& urlTypeName,
                         const QString& collectiveDescription,
                         const QStringList& extensions);

    QStringList identifyByExtension(const QUrl& url) const;

public:
    virtual ~UrlTypes() = default;

    QStringList loadableUrlTypeNames() const;
    QString collectiveDescriptionForUrlTypeName(const QString& urlTypeName) const;
    QStringList extensionsForUrlTypeName(const QString& urlTypeName) const;
};

#endif // URLTYPES_H
