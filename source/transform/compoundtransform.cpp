#include "compoundtransform.h"

void CompoundTransform::apply(const Graph& source, TransformedGraph& target) const
{
    if(_transforms.empty())
        return;

    // We can only use the apply overload with a source for the first transformation...
    _transforms.front()->apply(source, target);

    // ...thereafter we use the inplace one
    for(auto i = _transforms.cbegin() + 1; i != _transforms.cend(); i++)
        (*i)->apply(target);
}

void CompoundTransform::apply(TransformedGraph& target) const
{
    if(_transforms.empty())
        return;

    for(auto& transform : _transforms)
        transform->apply(target);
}
