#ifndef OPENGLWINDOW_H
#define OPENGLWINDOW_H

#include <QWindow>
#include <QTime>

class AbstractScene;
class GraphView;
class QOpenGLDebugMessage;

class OpenGLWindow : public QWindow
{
    Q_OBJECT

public:
    explicit OpenGLWindow( const QSurfaceFormat& format,
                           GraphView* graphView,
                           QScreen* parent = 0 );
    //FIXME destructor to delete things

    QOpenGLContext* context() const { return m_context; }

    void setScene( AbstractScene* scene );
    AbstractScene* scene() const { return m_scene; }
    
protected:
    virtual void initialise();
    virtual void resize();
    virtual void render();

    virtual void keyPressEvent(QKeyEvent* e);
    virtual void keyReleaseEvent(QKeyEvent* e);
    virtual void mousePressEvent(QMouseEvent* e);
    virtual void mouseReleaseEvent(QMouseEvent* e);
    virtual void mouseMoveEvent(QMouseEvent* e);
    virtual void mouseDoubleClickEvent(QMouseEvent* e);
    virtual void wheelEvent(QWheelEvent* e);

protected slots:
    virtual void updateScene();

    void resizeEvent( QResizeEvent* e );

    void messageLogged(const QOpenGLDebugMessage &message);

private:
    QOpenGLContext* m_context;
    AbstractScene* m_scene;
    GraphView* m_graphView;
    int m_debugLevel;
    
    QTime m_time;
};

#endif // OPENGLWINDOW_H
