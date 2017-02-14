#include "compoundtransform.h"

#include "transformedgraph.h"

#include "shared/utils/iterator_range.h"

bool CompoundTransform::applyFromSource(const Graph& source, TransformedGraph& target) const
{
    if(_transforms.empty())
    {
        // Effectively behave like an identity transform
        target.cloneFrom(source);
        return false;
    }

    // We can only use applyFromSource for the first transformation...
    bool changed = _transforms.front()->applyFromSource(source, target);

    // ...thereafter we use the inplace one
    for(const auto& transform : make_iterator_range(_transforms.begin() + 1, _transforms.end()))
        changed = transform->applyAndUpdate(target) || changed;

    return changed;
}

bool CompoundTransform::apply(TransformedGraph& target) const
{
    if(_transforms.empty())
        return false;

    return applyAndUpdate(target);
}
