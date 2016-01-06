#ifndef SCENE_H
#define SCENE_H

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
    virtual ~Scene() {}

    virtual void cleanup() {}
    virtual void update(float t) = 0;
    virtual void setViewportSize(int width, int height) = 0;

    virtual bool transitionActive() const = 0;

    virtual void onShow() {}
    virtual void onHide() {}

protected:
    bool visible() { return _visible; }

private:
    bool _interactionEnabled = true;
    bool _initialised = false;
    bool _visible = false;

    void setVisible(bool visible) { _visible = visible; }
};

#endif // SCENE_H
