#include "gmlfiletype.h"

#include <QFileInfo>

bool GmlFileType::identify(const QString& filename)
{
    QFileInfo info(filename);

    //FIXME actually check the file contents
    return info.completeSuffix().compare("gml", Qt::CaseInsensitive) == 0;
}
