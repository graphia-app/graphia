#ifndef IGENERICPLUGININSTANCE_H
#define IGENERICPLUGININSTANCE_H

#include "../graph/elementid.h"

#include <QString>

// An interface to a thing that has node names and edge weights, provided
// by the GenericPluginInstance in the generic plugin
class IGenericPluginInstance
{
public:
    virtual ~IGenericPluginInstance() = default;

    virtual void setNodeName(NodeId nodeId, const QString& name) = 0;
    virtual void setEdgeWeight(EdgeId edgeId, float weight) = 0;
};

#endif // IGENERICPLUGININSTANCE_H
