#include "openglwindow.h"

#include "opengldebugmessagemodel.h"

#include "graphcomponentscene.h"
#include "../ui/graphcomponentinteractor.h"
#include "../utils/utils.h"

#include <QGuiApplication>
#include <QKeyEvent>
#include <QOpenGLContext>
#include <QOpenGLDebugLogger>
#include <QTimer>

OpenGLWindow::OpenGLWindow(QScreen* screen)
    : QWindow(screen),
      _scene(nullptr),
      _interactor(nullptr),
      _sceneUpdateEnabled(true),
      _interactionEnabled(true)
{
    setSurfaceType(OpenGLSurface);

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

    setFormat(format);
    create();

    _context = std::make_shared<QOpenGLContext>();
    _context->setFormat(format);
    _context->create();

    initialise();
}

OpenGLWindow::~OpenGLWindow()
{
}

void OpenGLWindow::setScene(Scene* scene)
{
    if(_scene != nullptr)
    {
        _scene->setVisible(false);
        _scene->onHide();
    }

    _scene = scene;
    _scene->setContext(_context);

    if(!_scene->initialised())
    {
        _scene->initialise();
        _scene->setInitialised();
    }

    _scene->setVisible(true);
    _scene->onShow();

    resize();
}

void OpenGLWindow::enableInteraction()
{
    _interactionEnabled = true;
    _scene->enableInteraction();
}

void OpenGLWindow::disableInteraction()
{
    _interactionEnabled = false;
    _scene->disableInteraction();
}

void OpenGLWindow::initialise()
{
    makeContextCurrent();

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

    _time.start();
}

void OpenGLWindow::resize()
{
    if(_scene && width() > 0 && height() > 0)
    {
        makeContextCurrent();
        _scene->resize(width(), height());
    }
}

void OpenGLWindow::render()
{
    if(!isExposed())
        return;

    // FIXME: make configurable
    glEnable(GL_MULTISAMPLE);

    _scene->render();
    _context->swapBuffers(this);
}

void OpenGLWindow::update()
{
    makeContextCurrent();

    if(_sceneUpdateEnabled)
    {
        float time = _time.elapsed() / 1000.0f;
        _scene->update(time);
    }

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

void OpenGLWindow::makeContextCurrent()
{
    bool contextIsCurrent = _context->makeCurrent(this);
    Q_ASSERT(contextIsCurrent);
    Q_UNUSED(contextIsCurrent);
}

void OpenGLWindow::keyPressEvent(QKeyEvent* e)
{
    if(_interactor && _interactionEnabled)
        _interactor->keyPressEvent(e);
}

void OpenGLWindow::keyReleaseEvent(QKeyEvent* e)
{
    if(_interactor && _interactionEnabled)
        _interactor->keyReleaseEvent(e);
}

void OpenGLWindow::mousePressEvent(QMouseEvent* e)
{
    if(_interactor && _interactionEnabled)
        _interactor->mousePressEvent(e);
}

void OpenGLWindow::mouseReleaseEvent(QMouseEvent* e)
{
    if(_interactor && _interactionEnabled)
        _interactor->mouseReleaseEvent(e);
}

void OpenGLWindow::mouseMoveEvent(QMouseEvent* e)
{
    if(_interactor && _interactionEnabled)
        _interactor->mouseMoveEvent(e);
}

void OpenGLWindow::mouseDoubleClickEvent(QMouseEvent* e)
{
    if(_interactor && _interactionEnabled)
        _interactor->mouseDoubleClickEvent(e);
}

void OpenGLWindow::wheelEvent(QWheelEvent* e)
{
    if(_interactor && _interactionEnabled)
        _interactor->wheelEvent(e);
}
