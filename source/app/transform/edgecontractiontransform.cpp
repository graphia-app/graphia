#include "edgecontractiontransform.h"
#include "transformedgraph.h"
#include "attributes/conditionfncreator.h"

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

bool EdgeContractionTransform::apply(TransformedGraph& target) const
{
    target.setPhase(QObject::tr("Contracting"));

    EdgeIdSet edgeIdsToContract;

    for(auto edgeId : target.edgeIds())
    {
        if(edgeIdContracted(_edgeFilters, edgeId))
            edgeIdsToContract.insert(edgeId);
    }

    target.mutableGraph().contractEdges(edgeIdsToContract);

    return !edgeIdsToContract.empty();
}

std::unique_ptr<GraphTransform> EdgeContractionTransformFactory::create(const GraphTransformConfig& graphTransformConfig,
                                                                        const std::map<QString, DataField>& dataFields) const
{
    auto edgeContractionTransform = std::make_unique<EdgeContractionTransform>();

    edgeContractionTransform->addEdgeContractionFilter(CreateConditionFnFor::edge(dataFields, graphTransformConfig._condition));

    if(!edgeContractionTransform->hasEdgeContractionFilters())
        return nullptr;

    return std::move(edgeContractionTransform); //FIXME std::move required because of clang bug
}
