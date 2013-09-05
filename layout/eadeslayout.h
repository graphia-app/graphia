#ifndef EADESLAYOUT_H
#define EADESLAYOUT_H

#include "layout.h"
#include "sequencelayout.h"
#include "centreinglayout.h"
#include "../graph/graphmodel.h"

#include <QVector3D>

class EadesLayout : public Layout
{
    Q_OBJECT
private:
    bool firstIteration;
    QVector<QVector3D> moves;

public:
    EadesLayout(const ReadOnlyGraph& graph, NodeArray<QVector3D>& positions) :
        Layout(graph, positions, true),
        firstIteration(true),
        moves(graph.numNodes())
    {}

    void executeReal();
};

class EadesLayoutFactory : public LayoutFactory
{
public:
    EadesLayoutFactory(GraphModel* _graphModel) :
        LayoutFactory(_graphModel)
    {}

    Layout* create(ComponentId componentId) const
    {
        const ReadOnlyGraph* graph = this->_graphModel->graph().componentById(componentId);
        EadesLayout* eadesLayout = new EadesLayout(*graph, this->_graphModel->layout());
        CentreingLayout* centreingLayout = new CentreingLayout(*graph, this->_graphModel->layout());

        QList<Layout*> subLayouts;
        subLayouts.append(eadesLayout);
        subLayouts.append(centreingLayout);
        SequenceLayout* sequenceLayout = new SequenceLayout(*graph, this->_graphModel->layout(), subLayouts);
        return sequenceLayout;
    }
};

#endif // EADESLAYOUT_H
