#ifndef LAYOUTALGORITHM_H
#define LAYOUTALGORITHM_H

#include "../graph/grapharray.h"

#include <QList>
#include <QVector3D>
#include <QObject>

class LayoutAlgorithm : public QObject
{
    Q_OBJECT
protected:
    NodeArray<QVector3D>* positions;

public:
    LayoutAlgorithm(NodeArray<QVector3D>& positions) : positions(&positions) {}

    Graph& graph() { return *positions->graph(); }
    virtual void execute() = 0;

signals:
    void progress(int percentage);
    void complete();
};

#endif // LAYOUTALGORITHM_H
