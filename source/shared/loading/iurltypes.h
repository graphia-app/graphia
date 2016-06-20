#ifndef IURLTYPES_H
#define IURLTYPES_H

#include <QStringList>
#include <QString>

class IUrlTypes
{
public:
    virtual ~IUrlTypes() = default;

    virtual QStringList loadableUrlTypeNames() const = 0;
    virtual QString individualDescriptionForUrlTypeName(const QString& urlTypeName) const = 0;
    virtual QString collectiveDescriptionForUrlTypeName(const QString& urlTypeName) const = 0;
    virtual QStringList extensionsForUrlTypeName(const QString& urlTypeName) const = 0;
};

#endif // IURLTYPES_H
