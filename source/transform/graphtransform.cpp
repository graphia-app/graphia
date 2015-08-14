#include "graphtransform.h"
#include "transformedgraph.h"

#include "../graph/graph.h"

void GraphTransform::apply(const Graph& source, TransformedGraph& target) const
{
    target.cloneFrom(source);
    apply(target);
}
