#include "gmlfiletype.h"

#include <QFileInfo>

bool GmlFileType::identify(const QFileInfo& fileInfo) const
{
    //FIXME actually check the file contents
    return fileInfo.suffix().compare("gml", Qt::CaseInsensitive) == 0;
}
