#ifndef GRAPHMODELPROVIDER_H
#define GRAPHMODELPROVIDER_H

#include "igraphmodelprovider.h"

#include <QObject>

class GraphModelProvider : public virtual IGraphModelProvider
{
private:
    IGraphModel* _graphModel = nullptr;

public:
    IGraphModel* graphModel() { return _graphModel; }

    void setGraphModel(IGraphModel* graphModel)
    {
        _graphModel = graphModel;
        onGraphModelChanged();
    }

    virtual void onGraphModelChanged() = 0;
};

#endif // GRAPHMODELPROVIDER_H
