#include "matfileparser.h"

#include <QDebug>
#include <QImage>
#include <QUrl>

bool MatFileParser::parse(const QUrl& url, IGraphModel* graphModel)
{
    setProgress(-1);

    mat_t* matFile;
    matFile = Mat_Open(url.toLocalFile().toUtf8().data(), MAT_ACC_RDONLY);

    if(matFile == nullptr)
        return false;

    matvar_t* matVar;
    matVar = Mat_VarReadNext(matFile);
    bool result = false;

    // Check all stored variables within the matfile for a numerical matrix
    while(matVar != nullptr && !result)
    {
        if(matVar->rank == 2)
            result = processMatVarData(*matVar, graphModel);

        Mat_VarFree(matVar);
        matVar = Mat_VarReadNext(matFile);
    }

    Mat_Close(matFile);
    return result;
}
