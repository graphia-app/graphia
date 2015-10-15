#include "edgecontractiontransform.h"
#include "transformedgraph.h"

bool edgeIdContracted(const std::vector<EdgeContractionFn>& filters, EdgeId value)
{
    for(auto& filter : filters)
    {
        if(filter(value))
            return true;
    }

    return false;
}

void EdgeContractionTransform::apply(TransformedGraph& target) const
{
    EdgeIdSet edgeIdsToContract;

    for(auto edgeId : target.edgeIds())
    {
        if(edgeIdContracted(_edgeFilters, edgeId))
            edgeIdsToContract.insert(edgeId);
    }

    target.contractEdges(edgeIdsToContract);
}
