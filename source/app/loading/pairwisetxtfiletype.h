#ifndef PAIRWISETXTFILETYPE_H
#define PAIRWISETXTFILETYPE_H

#include "fileidentifier.h"

class PairwiseTxtFileType : public FileIdentifier::Type
{
public:
    PairwiseTxtFileType() :
        FileIdentifier::Type("PairwiseTXT", QObject::tr("Pairwise Text Files"), {"txt"})
    {}

    bool identify(const QFileInfo& fileInfo) const;
};

#endif // PAIRWISETXTFILETYPE_H
