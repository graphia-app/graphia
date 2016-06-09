#ifndef IGRAPHMODELPROVIDER_H
#define IGRAPHMODELPROVIDER_H

#include "igraphmodel.h"

class IGraphModelProvider
{
public:
    virtual ~IGraphModelProvider() = default;

    virtual IGraphModel* graphModel() = 0;
    virtual void setGraphModel(IGraphModel* graphModel) = 0;
};

#endif // IGRAPHMODELPROVIDER_H
