#ifndef SEQUENCELAYOUT_H
#define SEQUENCELAYOUT_H

#include "layout.h"

#include <QList>

class SequenceLayout : public NodeLayout
{
    Q_OBJECT
private:
    QList<NodeLayout*> subLayouts;

public:
    SequenceLayout(const ReadOnlyGraph& graph, NodePositions& positions) :
        NodeLayout(graph, positions)
    {}

    SequenceLayout(const ReadOnlyGraph& graph, NodePositions& positions, QList<NodeLayout*> subLayouts) :
        NodeLayout(graph, positions), subLayouts(subLayouts)
    {}

    virtual ~SequenceLayout()
    {
        for(NodeLayout* subLayout : subLayouts)
            delete subLayout;
    }

    void addSubLayout(NodeLayout* layout) { subLayouts.append(layout); }

    void cancel()
    {
        for(NodeLayout* subLayout : subLayouts)
            subLayout->cancel();
    }

    bool shouldPause()
    {
        for(NodeLayout* subLayout : subLayouts)
        {
            if(!subLayout->shouldPause())
                return false;
        }

        return true;
    }

    bool iterative()
    {
        for(NodeLayout* subLayout : subLayouts)
        {
            if(subLayout->iterative())
                return true;
        }

        return false;
    }

    void executeReal()
    {
        for(NodeLayout* subLayout : subLayouts)
            subLayout->execute();
    }
};

#endif // SEQUENCELAYOUT_H
