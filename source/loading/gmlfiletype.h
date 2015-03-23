#ifndef GMLFILETYPE_H
#define GMLFILETYPE_H

#include "fileidentifier.h"

#include <QString>

class GmlFileType : public FileIdentifier::Type
{
public:
    GmlFileType() :
        FileIdentifier::Type("GML", QObject::tr("GML Files"), {"gml"})
    {}

    bool identify(const QFileInfo& fileInfo) const;
};

#endif // GMLFILETYPE_H
