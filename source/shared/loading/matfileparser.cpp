#include "matfileparser.h"

#include "thirdparty/matio/matio.h"

#include <QDebug>
#include <QUrl>

bool MatFileParser::parse(const QUrl& url, IGraphModel* graphModel)
{
    mat_t *matfp;
    matfp = Mat_Open(url.toLocalFile().toUtf8().data(), MAT_ACC_RDONLY);
    if(matfp == nullptr)
    {
        qDebug() << "Error Opening File";
        return false;
    }
    qDebug() << Mat_GetVersion(matfp);

    matvar_t* matvar;
    matvar = Mat_VarReadNext(matfp);
    qDebug() << matvar;
    while ( matvar != nullptr )
    {
        qDebug() << "Name" << matvar->name;
        qDebug() << "Class Type" << matvar->class_type;
        qDebug() << "Total Byte Size" << matvar->nbytes;
        qDebug() << "Rank" << matvar->rank;
        qDebug() << "Data Type" << matvar->data_type;
        qDebug() << "Data Size" << matvar->data_size;

        for(int dim = 0; dim < matvar->rank; dim++)
            qDebug() << "Dimension" << dim << "size" << matvar->dims[dim];

        auto dataptr = static_cast<double*>(matvar->data);

        // Check it's a 2D matrix
        if(matvar->rank != 2)
            return false;

        size_t height = matvar->dims[0];
        size_t width = matvar->dims[1];

        std::map<size_t, NodeId> indexToNodeId;

        for(size_t row = 0; row < height; ++row)
        {
            auto nodeId = graphModel->mutableGraph().addNode();
            indexToNodeId[row] = nodeId;

            _userNodeData->setValueBy(nodeId, QObject::tr("Node Name"),
                                      QObject::tr("Node %1").arg(row + 1));
        }

        for(size_t row = 0; row < height; ++row)
        {
            for(size_t column = 0; column < width; ++column)
            {
                double& value = *(dataptr + (column * matvar->dims[0]) + row);

                auto edgeId = graphModel->mutableGraph().addEdge(indexToNodeId[row], indexToNodeId[column]);
                _userEdgeData->setValueBy(edgeId, QObject::tr("Edge Weight"),
                                          QString::number(value));
            }
        }

        matvar = Mat_VarReadNext(matfp);
    }

    Mat_Close(matfp);
    return true;
}
