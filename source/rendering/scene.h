#ifndef SCENE_H
#define SCENE_H

#include <QObject>

#include <memory>

class QOpenGLContext;

class Scene : public QObject
{
    Q_OBJECT

public:
    Scene(QObject* parent = nullptr) :
        QObject(parent), _interactionEnabled(true)
    {}
    virtual ~Scene() {}

    void setContext(std::shared_ptr<QOpenGLContext> context) { _context = context; }
    const QOpenGLContext& context() const { return *_context; }

    void enableInteraction() { _interactionEnabled = true; }
    void disableInteraction() { _interactionEnabled = false; }
    bool interactionEnabled() { return _interactionEnabled; }

    virtual void initialise() = 0;
    virtual void cleanup() {}
    virtual void update(float t) = 0;
    virtual void render() = 0;
    virtual void resize(int w, int h) = 0;

private:
    bool _interactionEnabled;
    std::shared_ptr<QOpenGLContext> _context;

signals:
    void userInteractionStarted();
    void userInteractionFinished();
};

#endif // SCENE_H
