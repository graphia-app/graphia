#ifndef MATFILEPARSER_H
#define MATFILEPARSER_H

#include "shared/loading/iparser.h"
#include "shared/plugins/userelementdata.h"

#include <matio.h>

class MatFileParser : public IParser
{
private:
    UserNodeData* _userNodeData;
    UserEdgeData* _userEdgeData;

public:
    MatFileParser(UserNodeData* userNodeData, UserEdgeData* userEdgeData) :
        _userNodeData(userNodeData), _userEdgeData(userEdgeData)
    {}

    bool parse(const QUrl& url, IGraphModel* graphModel) override;
    static bool canLoad(const QUrl&) { return true; }

    template<class T>
    bool matVarToGraph(const matvar_t& matvar, IGraphModel* graphModel)
    {
        size_t height = matvar.dims[0];
        size_t width = matvar.dims[1];
        auto nodeAndEdgeCount = static_cast<int>((height * width) + height);
        int progress = 0;

        std::map<size_t, NodeId> indexToNodeId;
        auto* dataptr = static_cast<T*>(matvar.data);

        for(size_t row = 0; row < height; ++row)
        {
            auto nodeId = graphModel->mutableGraph().addNode();
            indexToNodeId[row] = nodeId;

            _userNodeData->setValueBy(nodeId, QObject::tr("Node Name"), QObject::tr("Node %1").arg(row + 1));
            progress++;
            setProgress(static_cast<int>(progress * 100 / nodeAndEdgeCount));
        }

        for(size_t row = 0; row < height; ++row)
        {
            for(size_t column = 0; column < width; ++column)
            {
                T value = *(dataptr + (column * matvar.dims[0]) + row);

                auto edgeId = graphModel->mutableGraph().addEdge(indexToNodeId[row], indexToNodeId[column]);
                _userEdgeData->setValueBy(edgeId, QObject::tr("Edge Weight"), QString::number(value));
                progress++;

                setProgress(static_cast<int>(progress * 100 / nodeAndEdgeCount));
            }
        }

        return true;
    }

    bool processMatVarData(const matvar_t& matvar, IGraphModel* graphModel)
    {
        switch(matvar.data_type)
        {
        case MAT_T_DOUBLE: matVarToGraph<double>(matvar, graphModel); break;
        case MAT_T_SINGLE: matVarToGraph<float>(matvar, graphModel); break;
        case MAT_T_INT64: matVarToGraph<mat_int64_t>(matvar, graphModel); break;
        case MAT_T_UINT64: matVarToGraph<mat_uint64_t>(matvar, graphModel); break;
        case MAT_T_INT32: matVarToGraph<mat_int32_t>(matvar, graphModel); break;
        case MAT_T_UINT32: matVarToGraph<mat_uint32_t>(matvar, graphModel); break;
        case MAT_T_INT16: matVarToGraph<mat_int16_t>(matvar, graphModel); break;
        case MAT_T_UINT16: matVarToGraph<mat_uint16_t>(matvar, graphModel); break;
        case MAT_T_INT8: matVarToGraph<mat_int8_t>(matvar, graphModel); break;
        case MAT_T_UINT8: matVarToGraph<mat_uint8_t>(matvar, graphModel); break;
        default: return false;
        }
        return true;
    }
};

#endif // MATFILEPARSER_H
