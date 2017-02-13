#include "compoundtransform.h"

#include "transformedgraph.h"

#include "shared/utils/iterator_range.h"

bool CompoundTransform::applyFromSource(const Graph& source, TransformedGraph& target) const
{
    bool changed = false;

    if(_transforms.empty())
    {
        // Effectively behave like an identity transform
        target.cloneFrom(source);
        return changed;
    }

    // We can only use applyFromSource for the first transformation...
    changed = changed || _transforms.front()->applyFromSource(source, target);
    target.update();

    // ...thereafter we use the inplace one
    for(const auto& transform : make_iterator_range(_transforms.begin() + 1, _transforms.end()))
    {
        changed = changed || transform->apply(target);
        target.update();
    }

    return changed;
}

bool CompoundTransform::apply(TransformedGraph& target) const
{
    bool changed = false;

    if(_transforms.empty())
        return changed;

    for(auto& transform : _transforms)
    {
        changed = changed || transform->apply(target);
        target.update();
    }

    return changed;
}
