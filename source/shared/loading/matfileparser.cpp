/* Copyright © 2013-2020 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "matfileparser.h"

#include <QDebug>
#include <QImage>
#include <QUrl>

bool MatFileParser::parse(const QUrl& url, IGraphModel* graphModel)
{
    setProgress(-1);

    mat_t* matFile = nullptr;
    matFile = Mat_Open(url.toLocalFile().toUtf8().data(), MAT_ACC_RDONLY);

    if(matFile == nullptr)
        return false;

    matvar_t* matVar = nullptr;
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
