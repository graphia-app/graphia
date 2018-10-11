#include "matfileparser.h"

#include <QDebug>
#include <QImage>
#include <QUrl>

bool MatFileParser::parse(const QUrl& url, IGraphModel* graphModel)
{
    setProgress(-1);

    mat_t *matfp;
    matfp = Mat_Open(url.toLocalFile().toUtf8().data(), MAT_ACC_RDONLY);

    if(matfp == nullptr)
        return false;

    matvar_t* matvar;
    matvar = Mat_VarReadNext(matfp);

    while (matvar != nullptr)
    {
        // Check it's a 2D matrix if not just fail
        if(matvar->rank != 2)
            return false;

        if (!processMatVarData(*matvar, graphModel))
            return false;

        Mat_VarFree(matvar);
        matvar = Mat_VarReadNext(matfp);
    }

    Mat_Close(matfp);
    return true;
}
