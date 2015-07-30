#ifndef SEQUENCELAYOUT_H
#define SEQUENCELAYOUT_H

#include "layout.h"

#include <vector>
#include <memory>

class SequenceLayout : public Layout
{
    Q_OBJECT
private:
    std::vector<Layout*> _subLayouts;

public:
    SequenceLayout(const Graph& graph, NodePositions& positions) :
        Layout(graph, positions)
    {}

    SequenceLayout(const Graph& graph,
                   NodePositions& positions,
                   std::vector<Layout*> subLayouts) :
        Layout(graph, positions), _subLayouts(subLayouts)
    {}

    virtual ~SequenceLayout()
    {}

    void addSubLayout(Layout* layout) { _subLayouts.push_back(layout); }

    void cancel()
    {
        for(auto subLayout : _subLayouts)
            subLayout->cancel();
    }

    void uncancel()
    {
        for(auto subLayout : _subLayouts)
            subLayout->uncancel();
    }

    bool shouldPause()
    {
        for(auto subLayout : _subLayouts)
        {
            if(!subLayout->shouldPause())
                return false;
        }

        return true;
    }

    bool iterative()
    {
        for(auto subLayout : _subLayouts)
        {
            if(subLayout->iterative())
                return true;
        }

        return false;
    }

    void executeReal(uint64_t iteration)
    {
        for(auto subLayout : _subLayouts)
            subLayout->execute(iteration);
    }
};

#endif // SEQUENCELAYOUT_H
