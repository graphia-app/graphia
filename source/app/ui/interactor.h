#ifndef INTERACTOR_H
#define INTERACTOR_H

#include "graph/qmlelementid.h"

#include <QObject>
#include <Qt>

class QPoint;

class Interactor : public QObject
{
    Q_OBJECT

public:
    explicit Interactor(QObject* parent = nullptr) :
        QObject(parent)
    {}

    ~Interactor() override = default;

    virtual void mousePressEvent(const QPoint& pos, Qt::KeyboardModifiers modifiers, Qt::MouseButton button) = 0;
    virtual void mouseReleaseEvent(const QPoint& pos, Qt::KeyboardModifiers modifiers, Qt::MouseButton button) = 0;
    virtual void mouseMoveEvent(const QPoint& pos, Qt::KeyboardModifiers modifiers, Qt::MouseButton button) = 0;
    virtual void mouseDoubleClickEvent(const QPoint& pos, Qt::KeyboardModifiers modifiers, Qt::MouseButton button) = 0;
    virtual void wheelEvent(const QPoint& pos, float angle) = 0;
    virtual void nativeGestureEvent(Qt::NativeGestureType type, const QPoint& pos, float value) = 0;

signals:
    void userInteractionStarted() const;
    void userInteractionFinished() const;

    void clicked(int button, QmlNodeId nodeId) const;
};

#endif // INTERACTOR_H
