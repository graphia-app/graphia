#include "graphtransform.h"
#include "transformedgraph.h"

#include "../graph/graph.h"

void GraphTransform::apply(const MutableGraph& source, TransformedGraph& target) const
{
    target.cloneFrom(source);
    apply(target);
}
