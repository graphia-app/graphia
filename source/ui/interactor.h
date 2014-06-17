#ifndef INTERACTOR_H
#define INTERACTOR_H

#include <QObject>

class QMouseEvent;
class QWheelEvent;
class QKeyEvent;

class Interactor : public QObject
{
    Q_OBJECT

public:
    Interactor(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~Interactor() {}

    virtual void mousePressEvent(QMouseEvent*) = 0;
    virtual void mouseReleaseEvent(QMouseEvent*) = 0;
    virtual void mouseMoveEvent(QMouseEvent*) = 0;
    virtual void mouseDoubleClickEvent(QMouseEvent*) = 0;
    virtual void wheelEvent(QWheelEvent*) = 0;

    virtual void keyPressEvent(QKeyEvent*) = 0;
    virtual void keyReleaseEvent(QKeyEvent*) = 0;

signals:
    void userInteractionStarted();
    void userInteractionFinished();
};

#endif // INTERACTOR_H
