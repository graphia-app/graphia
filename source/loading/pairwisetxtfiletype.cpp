#include "pairwisetxtfiletype.h"

#include <QFileInfo>

bool PairwiseTxtFileType::identify(const QFileInfo& fileInfo) const
{
    //FIXME actually check the file contents
    return fileInfo.suffix().compare("txt", Qt::CaseInsensitive) == 0;
}
