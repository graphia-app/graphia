#include "matfileparser.h"

#include <QDebug>
#include <QImage>
#include <QUrl>

bool MatFileParser::parse(const QUrl& url, IGraphModel* graphModel)
{
    setProgress(-1);

    mat_t *matFile;
    matFile = Mat_Open(url.toLocalFile().toUtf8().data(), MAT_ACC_RDONLY);

    if(matFile == nullptr)
        return false;

    matvar_t* matVar;
    matVar = Mat_VarReadNext(matFile);

    while(matVar != nullptr)
    {
        // Check it's a 2D matrix if not just fail
        if(matVar->rank != 2)
            return false;

        if (!processMatVarData(*matVar, graphModel))
            return false;

        Mat_VarFree(matVar);
        matVar = Mat_VarReadNext(matFile);
    }

    Mat_Close(matFile);
    return true;
}
