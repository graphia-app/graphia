#include "openglwindow.h"

#include "opengldebugmessagemodel.h"

#include "graphcomponentscene.h"
#include "../ui/graphcomponentinteractor.h"

#include <QGuiApplication>
#include <QKeyEvent>
#include <QOpenGLContext>
#include <QOpenGLDebugLogger>
#include <QTimer>

OpenGLWindow::OpenGLWindow(QScreen* screen)
    : QWindow(screen),
      _context(nullptr),
      _scene(nullptr),
      _interactor(nullptr)
{
    // Tell Qt we will use OpenGL for this window
    setSurfaceType(OpenGLSurface);

    // Request a full screen button (if available)
#if !defined(Q_OS_WIN)
    setFlags(flags() | Qt::WindowFullscreenButtonHint);
#endif

    QSurfaceFormat format;
    format.setMajorVersion(3);
    format.setMinorVersion(3);

    format.setDepthBufferSize(24);
    format.setSamples(1);
    format.setProfile(QSurfaceFormat::CoreProfile);

    if (qgetenv("OPENGL_DEBUG").toInt())
        format.setOption(QSurfaceFormat::DebugContext);

    // Create the native window
    setFormat(format);
    create();

    // Create an OpenGL context
    _context = new QOpenGLContext;
    _context->setFormat(format);
    _context->create();
}

OpenGLWindow::~OpenGLWindow()
{
    delete _context;
}

void OpenGLWindow::setScene(Scene* scene)
{
    Q_ASSERT(scene);
    _scene = scene;

    // Initialise the scene
    _scene->setContext(_context);
    initialise();

    // This timer drives the scene updates
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &OpenGLWindow::updateScene);
    timer->start(16);
}

void OpenGLWindow::initialise()
{
    _context->makeCurrent(this);

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
    _context->makeCurrent(this);
    _scene->resize(width(), height());
}

void OpenGLWindow::render()
{
    if(!isExposed())
        return;

    // Make the context current
    _context->makeCurrent(this);

    // FIXME: make configurable
    glEnable(GL_MULTISAMPLE);

    // Do the rendering (to the back buffer)
    _scene->render();

    // Swap front/back buffers
    _context->swapBuffers(this);
}

void OpenGLWindow::updateScene()
{
    float time = _time.elapsed() / 1000.0f;
    _scene->update(time);
    render();
}

void OpenGLWindow::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e);
    if(_context)
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
    _interactor->keyPressEvent(e);
}

void OpenGLWindow::keyReleaseEvent(QKeyEvent* e)
{
    _interactor->keyReleaseEvent(e);
}

void OpenGLWindow::mousePressEvent(QMouseEvent* e)
{
    _interactor->mousePressEvent(e);
}

void OpenGLWindow::mouseReleaseEvent(QMouseEvent* e)
{
    _interactor->mouseReleaseEvent(e);
}

void OpenGLWindow::mouseMoveEvent(QMouseEvent* e)
{
    _interactor->mouseMoveEvent(e);
}

void OpenGLWindow::mouseDoubleClickEvent(QMouseEvent* e)
{
    _interactor->mouseDoubleClickEvent(e);
}

void OpenGLWindow::wheelEvent(QWheelEvent* e)
{
    _interactor->wheelEvent(e);
}

