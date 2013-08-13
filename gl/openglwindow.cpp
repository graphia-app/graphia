#include "openglwindow.h"

#include "camerascene.h"
#include "cameracontroller.h"

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
      m_scene( 0 ),
      m_controller( 0 ),
      m_leftButtonPressed( false )
{
    m_controller = new CameraController;

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

    CameraScene* camScene = qobject_cast<CameraScene*>( scene );
    if ( camScene )
    {
        m_controller->setCamera( camScene->camera() );
    }
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

    if ( m_controller->isMultisampleEnabled() )
        glEnable( GL_MULTISAMPLE );
    else
        glDisable( GL_MULTISAMPLE );

    // Do the rendering (to the back buffer)
    m_scene->render();

    // Swap front/back buffers
    m_context->swapBuffers( this );
}

void OpenGLWindow::updateScene()
{
    float time = m_time.elapsed() / 1000.0f;
    m_controller->update( time );
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

void OpenGLWindow::keyPressEvent( QKeyEvent* e )
{
    const float MOVE_SPEED = 2.0f;

    GraphScene* scene = static_cast<GraphScene*>( this->scene() );
    switch ( e->key() )
    {
    case Qt::Key_D:
        scene->setSideSpeed(MOVE_SPEED);
        break;

    case Qt::Key_A:
        scene->setSideSpeed(-MOVE_SPEED);
        break;

    case Qt::Key_W:
        scene->setForwardSpeed(MOVE_SPEED);
        break;

    case Qt::Key_S:
        scene->setForwardSpeed(-MOVE_SPEED);
        break;

    case Qt::Key_Shift:
        scene->setVerticalSpeed(MOVE_SPEED);
        break;

    case Qt::Key_Control:
        scene->setVerticalSpeed(-MOVE_SPEED);
        break;

    case Qt::Key_Space:
        scene->setViewCenterFixed( true );
        break;

    default:
        if (m_controller->keyPressEvent(e))
            return;

        QWindow::keyPressEvent( e );
        break;
    }
}

void OpenGLWindow::keyReleaseEvent( QKeyEvent* e )
{
    GraphScene* scene = static_cast<GraphScene*>( this->scene() );
    switch ( e->key() )
    {
    case Qt::Key_A:
    case Qt::Key_D:
        scene->setSideSpeed( 0.0f );
        break;

    case Qt::Key_W:
    case Qt::Key_S:
        scene->setForwardSpeed( 0.0f );
        break;

    case Qt::Key_Shift:
    case Qt::Key_Control:
        scene->setVerticalSpeed( 0.0f );
        break;

    case Qt::Key_Space:
        scene->setViewCenterFixed( false );

    default:
        if (m_controller->keyReleaseEvent(e))
            return;

        QWindow::keyReleaseEvent(e);
        break;
    }
}

void OpenGLWindow::mousePressEvent( QMouseEvent* e )
{
    if ( e->button() == Qt::LeftButton )
    {
        m_leftButtonPressed = true;
        m_pos = m_prevPos = e->pos();
    }

    m_controller->mousePressEvent(e);
}

void OpenGLWindow::mouseReleaseEvent( QMouseEvent* e )
{
    if ( e->button() == Qt::LeftButton )
        m_leftButtonPressed = false;

    m_controller->mouseReleaseEvent(e);
}

void OpenGLWindow::mouseMoveEvent( QMouseEvent* e )
{
    if ( m_leftButtonPressed )
    {
        m_pos = e->pos();
        float dx = 0.5f * ( m_pos.x() - m_prevPos.x() );
        float dy = 0.5f * ( m_pos.y() - m_prevPos.y() );
        m_prevPos = m_pos;

        GraphScene* scene = static_cast<GraphScene*>( this->scene() );
        scene->pan( dx );
        scene->tilt( dy );
    }

    m_controller->mouseMoveEvent(e);
}

