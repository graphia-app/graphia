#include "compoundtransform.h"

#include "transformedgraph.h"

void CompoundTransform::apply(const Graph& source, TransformedGraph& target) const
{
    if(_transforms.empty())
        return;

    // We can only use the apply overload with a source for the first transformation...
    _transforms.front()->apply(source, target);
    target.update();

    // ...thereafter we use the inplace one
    for(auto i = _transforms.begin() + 1; i != _transforms.end(); i++)
    {
        auto& transform = *i;
        transform->apply(target);
        target.update();
    }
}

void CompoundTransform::apply(TransformedGraph& target) const
{
    if(_transforms.empty())
        return;

    for(auto& transform : _transforms)
    {
        transform->apply(target);
        target.update();
    }
}
