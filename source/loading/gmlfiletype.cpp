#include "gmlfiletype.h"

#include <QFileInfo>

bool GmlFileType::identify(const QFileInfo& fileInfo)
{
    //FIXME actually check the file contents
    return fileInfo.completeSuffix().compare("gml", Qt::CaseInsensitive) == 0;
}
