#ifndef SCENE_H
#define SCENE_H

#include "projection.h"

#include <QObject>

#include <memory>

class Scene : public QObject
{
    Q_OBJECT

    friend class GraphRenderer;

public:
    explicit Scene(QObject* parent = nullptr) :
        QObject(parent)
    {}
    ~Scene() override = default;

    virtual void cleanup() {}
    virtual void update(float t) = 0;
    virtual void setViewportSize(int width, int height) = 0;

    virtual bool transitionActive() const = 0;

    virtual void onShow() {}
    virtual void onHide() {}

    virtual void resetView(bool doTransition = true) = 0;
    virtual bool viewIsReset() const = 0;

    virtual void onProjectionChanged(Projection projection) = 0;

protected:
    bool visible() const { return _visible; }

private:
    bool _interactionEnabled = true;
    bool _initialised = false;
    bool _visible = false;

    void setVisible(bool visible) { _visible = visible; }
};

#endif // SCENE_H
