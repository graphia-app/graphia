#include "openglwindow.h"

#include "opengldebugmessagemodel.h"

#include "graphscene.h"

#include <QGuiApplication>
#include <QKeyEvent>
#include <QOpenGLContext>
#include <QOpenGLDebugLogger>
#include <QTimer>

OpenGLWindow::OpenGLWindow( const QSurfaceFormat& format,
                            QScreen* screen )
    : QWindow( screen ),
      m_context( 0 ),
      m_scene( 0 )
{
    // Tell Qt we will use OpenGL for this window
    setSurfaceType( OpenGLSurface );

    // Request a full screen button (if available)
#if !defined(Q_OS_WIN)
    setFlags( flags() | Qt::WindowFullscreenButtonHint );
#endif

    QSurfaceFormat actualFormat = format;

    if (qgetenv("OPENGL_DEBUG").toInt())
        actualFormat.setOption(QSurfaceFormat::DebugContext);

    // Create the native window
    setFormat( actualFormat );
    create();

    // Create an OpenGL context
    m_context = new QOpenGLContext;
    m_context->setFormat( actualFormat );
    m_context->create();
}

void OpenGLWindow::setScene( AbstractScene* scene )
{
    // We take ownership of the scene
    Q_ASSERT( scene );
    m_scene = scene;
    m_scene->setParent( this );

    // Initialise the scene
    m_scene->setContext( m_context );
    initialise();

    // This timer drives the scene updates
    QTimer* timer = new QTimer( this );
    connect( timer, &QTimer::timeout, this, &OpenGLWindow::updateScene );
    timer->start( 16 );
}

void OpenGLWindow::initialise()
{
    m_context->makeCurrent( this );

    if (qgetenv("OPENGL_DEBUG").toInt())
    {
        QOpenGLDebugLogger *logger = new QOpenGLDebugLogger(this);
        if (logger->initialize())
        {
            const QList<QOpenGLDebugMessage> startupMessages = logger->loggedMessages();

            connect(logger, &QOpenGLDebugLogger::messageLogged,
                    this, &OpenGLWindow::messageLogged);

            if (!startupMessages.isEmpty())
                qDebug() << "Debug messages logged on startup:" << startupMessages;
        }
        else
        {
            qDebug() << "Debugging requested but logger failed to initialize";
        }
    }

    m_scene->initialise();

    m_time.start();
}

void OpenGLWindow::resize()
{
    m_context->makeCurrent( this );
    m_scene->resize( width(), height() );
}

void OpenGLWindow::render()
{
    if ( !isExposed() )
        return;

    // Make the context current
    m_context->makeCurrent( this );

    // FIXME: make configurable
    glEnable( GL_MULTISAMPLE );

    // Do the rendering (to the back buffer)
    m_scene->render();

    // Swap front/back buffers
    m_context->swapBuffers( this );
}

void OpenGLWindow::updateScene()
{
    float time = m_time.elapsed() / 1000.0f;
    m_scene->update( time );
    render();
}

void OpenGLWindow::resizeEvent( QResizeEvent* e )
{
    Q_UNUSED( e );
    if ( m_context )
        resize();
}

void OpenGLWindow::messageLogged(const QOpenGLDebugMessage &message)
{
    qDebug() << message;
}

void OpenGLWindow::keyPressEvent(QKeyEvent* e)
{
    if (!m_scene->keyPressEvent(e))
        QWindow::keyPressEvent(e);
}

void OpenGLWindow::keyReleaseEvent(QKeyEvent* e)
{
    if (!m_scene->keyReleaseEvent(e))
        QWindow::keyReleaseEvent(e);
}

void OpenGLWindow::mousePressEvent(QMouseEvent* e)
{
    m_scene->mousePressEvent(e);
}

void OpenGLWindow::mouseReleaseEvent(QMouseEvent* e)
{
    m_scene->mouseReleaseEvent(e);
}

void OpenGLWindow::mouseMoveEvent(QMouseEvent* e)
{
    m_scene->mouseMoveEvent(e);
}

void OpenGLWindow::mouseDoubleClickEvent(QMouseEvent* e)
{
    m_scene->mouseDoubleClickEvent(e);
}

void OpenGLWindow::wheelEvent(QWheelEvent* e)
{
    m_scene->wheelEvent(e);
}

