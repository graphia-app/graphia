#include "graphtransform.h"
#include "transformedgraph.h"

#include "graph/graph.h"

bool GraphTransform::applyFromSource(const Graph& source, TransformedGraph& target) const
{
    target.cloneFrom(source);
    return applyAndUpdate(target);
}

bool GraphTransform::applyAndUpdate(TransformedGraph& target) const
{
    bool anyChange = false;
    bool change = false;

    do
    {
        change = apply(target);
        anyChange = anyChange || change;
        target.update();
    } while(repeating() && change);

    return anyChange;
}
