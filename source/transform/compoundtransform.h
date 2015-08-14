#ifndef COMPOUNDTRANSFORM_H
#define COMPOUNDTRANSFORM_H

#include "graphtransform.h"

#include <vector>
#include <memory>

class CompoundTransform : public GraphTransform
{
public:
    void apply(const Graph &source, TransformedGraph &target) const;
    void apply(TransformedGraph &target) const;

    void addTransform(std::unique_ptr<const GraphTransform> t) { _transforms.emplace_back(std::move(t)); }
    void clear() { _transforms.clear(); }

private:
    std::vector<std::unique_ptr<const GraphTransform>> _transforms;
};

#endif // COMPOUNDTRANSFORM_H
