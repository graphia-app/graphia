#ifndef SEQUENCELAYOUT_H
#define SEQUENCELAYOUT_H

#include "layout.h"

#include <vector>

class SequenceLayout : public NodeLayout
{
    Q_OBJECT
private:
    std::vector<NodeLayout*> _subLayouts;

public:
    SequenceLayout(const ReadOnlyGraph& graph, NodePositions& positions) :
        NodeLayout(graph, positions)
    {}

    SequenceLayout(const ReadOnlyGraph& graph, NodePositions& positions, std::vector<NodeLayout*> subLayouts) :
        NodeLayout(graph, positions), _subLayouts(subLayouts)
    {}

    virtual ~SequenceLayout()
    {}

    void addSubLayout(NodeLayout* layout) { _subLayouts.push_back(layout); }

    void cancel()
    {
        for(NodeLayout* subLayout : _subLayouts)
            subLayout->cancel();
    }

    bool shouldPause()
    {
        for(NodeLayout* subLayout : _subLayouts)
        {
            if(!subLayout->shouldPause())
                return false;
        }

        return true;
    }

    bool iterative()
    {
        for(NodeLayout* subLayout : _subLayouts)
        {
            if(subLayout->iterative())
                return true;
        }

        return false;
    }

    void executeReal(uint64_t iteration)
    {
        for(NodeLayout* subLayout : _subLayouts)
            subLayout->execute(iteration);
    }
};

#endif // SEQUENCELAYOUT_H
