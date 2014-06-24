#ifndef SCENE_H
#define SCENE_H

#include <QObject>

#include <memory>

class QOpenGLContext;

class Scene : public QObject
{
    Q_OBJECT

public:
    Scene(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~Scene() {}

    void setContext(std::shared_ptr<QOpenGLContext> context) { _context = context; }
    const QOpenGLContext& context() const { return *_context; }

    virtual void initialise() = 0;
    virtual void cleanup() {}
    virtual void update(float t) = 0;
    virtual void render() = 0;
    virtual void resize(int w, int h) = 0;

protected:
    std::shared_ptr<QOpenGLContext> _context;

signals:
    void userInteractionStarted();
    void userInteractionFinished();
};

#endif // SCENE_H
