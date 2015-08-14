#include "edgecontractiontransform.h"
#include "transformedgraph.h"

bool edgeIdContracted(std::vector<EdgeContractedFn> edgeIds, EdgeId value)
{
    return std::any_of(edgeIds.cbegin(), edgeIds.cend(), [value](EdgeContractedFn f) { return f(value); });
}

void EdgeContractionTransform::apply(TransformedGraph& target) const
{
    for(auto edgeId : target.edgeIds())
    {
        if(edgeIdContracted(_edgeFilters, edgeId))
            target.contractEdge(edgeId);
    }
}
