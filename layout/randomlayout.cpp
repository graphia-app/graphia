#include "randomlayout.h"

#include "../utils.h"

void RandomLayout::execute()
{
    NodeArray<QVector3D>& positions = *this->positions;

    for(Graph::NodeId nodeId : graph().nodeIds())
    {
        positions[nodeId] = QVector3D(Utils::rand(-1.0f, 1.0f),
                                      Utils::rand(-1.0f, 1.0f),
                                      Utils::rand(-1.0f, 1.0f));
    }
}
