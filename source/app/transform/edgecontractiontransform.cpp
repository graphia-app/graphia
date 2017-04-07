#include "edgecontractiontransform.h"
#include "transformedgraph.h"
#include "attributes/conditionfncreator.h"
#include "graph/graphmodel.h"

#include <QObject>

static bool edgeIdContracted(const std::vector<EdgeConditionFn>& filters, EdgeId edgeId)
{
    for(auto& filter : filters)
    {
        if(filter(edgeId))
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
        if(_ignoreTails && target.typeOf(edgeId) == MultiElementType::Tail)
            continue;

        if(edgeIdContracted(_edgeFilters, edgeId))
            edgeIdsToContract.insert(edgeId);
    }

    target.mutableGraph().contractEdges(edgeIdsToContract);

    return !edgeIdsToContract.empty();
}

std::unique_ptr<GraphTransform> EdgeContractionTransformFactory::create(const GraphTransformConfig& graphTransformConfig) const
{
    auto edgeContractionTransform = std::make_unique<EdgeContractionTransform>();

    auto conditionFn = CreateConditionFnFor::edge(graphModel()->attributes(), graphTransformConfig._condition);
    if(conditionFn == nullptr)
        return nullptr;

    edgeContractionTransform->addEdgeContractionFilter(conditionFn);

    if(!edgeContractionTransform->hasEdgeContractionFilters())
        return nullptr;

    auto attributeNames = graphTransformConfig.attributeNames();
    edgeContractionTransform->setIgnoreTails(
        std::any_of(attributeNames.begin(), attributeNames.end(),
        [this](const auto& attributeName)
        {
            return this->graphModel()->attribute(attributeName).testFlag(AttributeFlag::IgnoreTails);
        }));

    return std::move(edgeContractionTransform); //FIXME std::move required because of clang bug
}
