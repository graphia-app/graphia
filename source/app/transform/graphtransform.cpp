#include "graphtransform.h"
#include "transformedgraph.h"

#include "graph/graph.h"

bool GraphTransform::applyFromSource(const Graph& source, TransformedGraph& target) const
{
    target.cloneFrom(source);
    return apply(target);
}
