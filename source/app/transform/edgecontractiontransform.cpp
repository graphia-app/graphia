#include "edgecontractiontransform.h"
#include "transformedgraph.h"

#include <QObject>

static bool edgeIdContracted(const std::vector<EdgeConditionFn>& filters, EdgeId value)
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
    target.setSubPhase(QObject::tr("Contracting"));

    EdgeIdSet edgeIdsToContract;

    for(auto edgeId : target.edgeIds())
    {
        if(edgeIdContracted(_edgeFilters, edgeId))
            edgeIdsToContract.insert(edgeId);
    }

    target.contractEdges(edgeIdsToContract);
}

std::unique_ptr<GraphTransform> EdgeContractionTransformFactory::create(const EdgeConditionFn& conditionFn) const
{
    auto edgeContractionTransform = std::make_unique<EdgeContractionTransform>();
    edgeContractionTransform->addEdgeContractionFilter(conditionFn);

    return std::move(edgeContractionTransform);
}
