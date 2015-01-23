#ifndef SCENE_H
#define SCENE_H

#include <QObject>

#include <memory>

class QOpenGLContext;

class Scene : public QObject
{
    Q_OBJECT

    friend class OpenGLWindow;

public:
    Scene(QObject* parent = nullptr) :
        QObject(parent),
        _interactionEnabled(true),
        _initialised(false),
        _visible(false)
    {}
    virtual ~Scene() {}

    void setContext(std::shared_ptr<QOpenGLContext> context) { _context = context; }
    const QOpenGLContext& context() const { return *_context; }

    bool initialised() { return _initialised; }
    void setInitialised() { _initialised = true; }

    void enableInteraction() { _interactionEnabled = true; }
    void disableInteraction() { _interactionEnabled = false; }
    bool interactionEnabled() { return _interactionEnabled; }

    virtual void initialise() = 0;
    virtual void cleanup() {}
    virtual void update(float t) = 0;
    virtual void render() = 0;
    virtual void resize(int width, int height) = 0;

    virtual void onShow() {}
    virtual void onHide() {}

protected:
    bool visible() { return _visible; }

private:
    bool _interactionEnabled;
    bool _initialised;
    bool _visible;
    std::shared_ptr<QOpenGLContext> _context;

    void setVisible(bool visible) { _visible = visible; }
};

#endif // SCENE_H
