#include "edgecontractiontransform.h"
#include "transformedgraph.h"

bool edgeIdContracted(std::vector<EdgeContractedFn> edgeIds, EdgeId value)
{
    return std::any_of(edgeIds.begin(), edgeIds.end(), [value](EdgeContractedFn f) { return f(value); });
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
