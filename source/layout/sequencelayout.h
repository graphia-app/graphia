#ifndef SEQUENCELAYOUT_H
#define SEQUENCELAYOUT_H

#include "layout.h"

#include <vector>
#include <memory>
#include <algorithm>

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
        return std::any_of(_subLayouts.cbegin(), _subLayouts.cend(),
                           [](Layout* layout) { return layout->shouldPause(); });
    }

    bool iterative()
    {
        return std::any_of(_subLayouts.cbegin(), _subLayouts.cend(),
                           [](Layout* layout) { return layout->iterative(); });
    }

    void executeReal(uint64_t iteration)
    {
        for(auto subLayout : _subLayouts)
            subLayout->execute(iteration);
    }
};

#endif // SEQUENCELAYOUT_H
