#ifndef GRAPHTRANSFORM_H
#define GRAPHTRANSFORM_H

class TransformedGraph;

class GraphTransform
{
public:
    virtual void apply(TransformedGraph&) const {}
};

using IdentityTransform = GraphTransform;

#endif // GRAPHTRANSFORM_H
