#include "openglwindow.h"

#include "opengldebugmessagemodel.h"

#include "graphscene.h"
#include "../ui/graphview.h"

#include <QGuiApplication>
#include <QKeyEvent>
#include <QOpenGLContext>
#include <QOpenGLDebugLogger>
#include <QTimer>

OpenGLWindow::OpenGLWindow(const QSurfaceFormat& format, GraphView* graphView,
                            QScreen* screen )
    : QWindow( screen ),
      _context( nullptr ),
      _scene( nullptr ),
      _graphView(graphView)
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
    _context = new QOpenGLContext;
    _context->setFormat( actualFormat );
    _context->create();
}

void OpenGLWindow::setScene( AbstractScene* scene )
{
    // We take ownership of the scene
    Q_ASSERT( scene );
    _scene = scene;
    _scene->setParent( this );

    // Initialise the scene
    _scene->setContext( _context );
    initialise();

    // This timer drives the scene updates
    QTimer* timer = new QTimer( this );
    connect( timer, &QTimer::timeout, this, &OpenGLWindow::updateScene );
    timer->start( 16 );
}

void OpenGLWindow::initialise()
{
    _context->makeCurrent( this );

    _debugLevel = qgetenv("OPENGL_DEBUG").toInt();
    if(_debugLevel != 0)
    {
        QOpenGLDebugLogger *logger = new QOpenGLDebugLogger(this);
        if (logger->initialize())
        {
            const QList<QOpenGLDebugMessage> startupMessages = logger->loggedMessages();

            connect(logger, &QOpenGLDebugLogger::messageLogged,
                    this, &OpenGLWindow::messageLogged, Qt::DirectConnection);

            if (!startupMessages.isEmpty())
            {
                for(auto startupMessage : startupMessages)
                    messageLogged(startupMessage);
            }

            logger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
            logger->enableMessages();
        }
        else
        {
            qDebug() << "Debugging requested but logger failed to initialize";
        }
    }

    _scene->initialise();

    _time.start();
}

void OpenGLWindow::resize()
{
    _context->makeCurrent( this );
    _scene->resize( width(), height() );
}

void OpenGLWindow::render()
{
    if ( !isExposed() )
        return;

    // Make the context current
    _context->makeCurrent( this );

    // FIXME: make configurable
    glEnable( GL_MULTISAMPLE );

    // Do the rendering (to the back buffer)
    _scene->render();

    // Swap front/back buffers
    _context->swapBuffers( this );
}

void OpenGLWindow::updateScene()
{
    float time = _time.elapsed() / 1000.0f;
    _scene->update( time );
    render();
}

void OpenGLWindow::resizeEvent( QResizeEvent* e )
{
    Q_UNUSED( e );
    if ( _context )
        resize();
}

void OpenGLWindow::messageLogged(const QOpenGLDebugMessage &message)
{
    if(!(message.severity() & _debugLevel))
        return;

    qDebug() << "OpenGL:" << message.message();
}

void OpenGLWindow::keyPressEvent(QKeyEvent* e)
{
    _graphView->keyPressEvent(e);
}

void OpenGLWindow::keyReleaseEvent(QKeyEvent* e)
{
    _graphView->keyReleaseEvent(e);
}

void OpenGLWindow::mousePressEvent(QMouseEvent* e)
{
    _graphView->mousePressEvent(e);
}

void OpenGLWindow::mouseReleaseEvent(QMouseEvent* e)
{
    _graphView->mouseReleaseEvent(e);
}

void OpenGLWindow::mouseMoveEvent(QMouseEvent* e)
{
    _graphView->mouseMoveEvent(e);
}

void OpenGLWindow::mouseDoubleClickEvent(QMouseEvent* e)
{
    _graphView->mouseDoubleClickEvent(e);
}

void OpenGLWindow::wheelEvent(QWheelEvent* e)
{
    _graphView->wheelEvent(e);
}

