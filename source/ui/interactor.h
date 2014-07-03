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
    Interactor(QObject* parent = nullptr) :
        QObject(parent),
        _interacting(false)
    {
        connect(this, &Interactor::userInteractionStarted, this, &Interactor::onUserInteractionStarted);
        connect(this, &Interactor::userInteractionFinished, this, &Interactor::onUserInteractionFinished);
    }

    virtual ~Interactor() {}

    virtual void mousePressEvent(QMouseEvent*) = 0;
    virtual void mouseReleaseEvent(QMouseEvent*) = 0;
    virtual void mouseMoveEvent(QMouseEvent*) = 0;
    virtual void mouseDoubleClickEvent(QMouseEvent*) = 0;
    virtual void wheelEvent(QWheelEvent*) = 0;

    virtual void keyPressEvent(QKeyEvent*) = 0;
    virtual void keyReleaseEvent(QKeyEvent*) = 0;

    bool interacting() const { return _interacting; }

private:
    bool _interacting;

private slots:
    void onUserInteractionStarted() { _interacting = true; }
    void onUserInteractionFinished() { _interacting = false; }

signals:
    void userInteractionStarted() const;
    void userInteractionFinished() const;
};

#endif // INTERACTOR_H
