#ifndef IDENTITYTRANSFORM_H
#define IDENTITYTRANSFORM_H

#include "graphtransform.h"

class IdentityTransform : public GraphTransform
{
public:
    bool nodeIsFiltered(const Node&) const { return false; }
    bool edgeIsFiltered(const Edge&) const { return false; }
};

#endif // IDENTITYTRANSFORM_H
