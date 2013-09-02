#ifndef SEQUENCELAYOUT_H
#define SEQUENCELAYOUT_H

#include "layout.h"

#include <QList>

class SequenceLayout : public Layout
{
    Q_OBJECT
private:
    QList<Layout*> subLayouts;

public:
    SequenceLayout(const ReadOnlyGraph& graph, NodeArray<QVector3D>& positions) :
        Layout(graph, positions)
    {}

    SequenceLayout(const ReadOnlyGraph& graph, NodeArray<QVector3D>& positions, QList<Layout*> subLayouts) :
        Layout(graph, positions), subLayouts(subLayouts)
    {}

    void addSubLayout(Layout* layout) { subLayouts.append(layout); }

    void cancel()
    {
        for(Layout* subLayout : subLayouts)
            subLayout->cancel();
    }

    bool shouldPause()
    {
        for(Layout* subLayout : subLayouts)
        {
            if(!subLayout->shouldPause())
                return false;
        }

        return true;
    }

    bool iterative()
    {
        for(Layout* subLayout : subLayouts)
        {
            if(subLayout->iterative())
                return true;
        }

        return false;
    }

    void executeReal()
    {
        for(Layout* subLayout : subLayouts)
            subLayout->execute();
    }
};

#endif // SEQUENCELAYOUT_H
