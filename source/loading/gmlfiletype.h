#ifndef GMLFILETYPE_H
#define GMLFILETYPE_H

#include "fileidentifier.h"

class GmlFileType : public FileIdentifier::Type
{
public:
    GmlFileType() :
        FileIdentifier::Type("GML")
    {}

    bool identify(const QString& filename);
};

#endif // GMLFILETYPE_H
