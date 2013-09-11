#include "genericgraphmodel.h"

GenericGraphModel::GenericGraphModel(const QString &name) :
    _graph(),
    _nodePositions(_graph),
    _componentPositions(_graph),
    _name(name)
{
}

