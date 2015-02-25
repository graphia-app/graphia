#ifndef GRAPHCOMMONINTERACTOR_H
#define GRAPHCOMMONINTERACTOR_H

#include "interactor.h"

class GraphCommonInteractor : public Interactor
{
    Q_OBJECT
public:
    GraphCommonInteractor(QObject* parent = nullptr) :
        Interactor(parent)
    {}

private:
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseDoubleClickEvent(QMouseEvent* e);
    void wheelEvent(QWheelEvent* e);

    void keyPressEvent(QKeyEvent* e);
    void keyReleaseEvent(QKeyEvent* e);
};

#endif // GRAPHCOMMONINTERACTOR_H
