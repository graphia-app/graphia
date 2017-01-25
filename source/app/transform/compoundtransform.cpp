#include "compoundtransform.h"

#include "transformedgraph.h"

#include "shared/utils/iterator_range.h"

void CompoundTransform::applyFromSource(const Graph& source, TransformedGraph& target) const
{
    if(_transforms.empty())
    {
        // Effectively behave like an identity transform
        target.cloneFrom(source);
        return;
    }

    // We can only use applyFromSource for the first transformation...
    _transforms.front()->applyFromSource(source, target);
    target.update();

    // ...thereafter we use the inplace one
    for(const auto& transform : make_iterator_range(_transforms.begin() + 1, _transforms.end()))
    {
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
