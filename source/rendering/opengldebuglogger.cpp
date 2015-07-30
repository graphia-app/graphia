#include "opengldebuglogger.h"

#include <QOpenGLDebugLogger>
#include <QDebug>

OpenGLDebugLogger::OpenGLDebugLogger(QObject* parent) :
    QObject(parent)
{
    _debugLevel = qgetenv("OPENGL_DEBUG").toInt();
    if(_debugLevel != 0)
    {
        _logger = new QOpenGLDebugLogger(this);
        if(_logger->initialize())
        {
            const QList<QOpenGLDebugMessage> startupMessages = _logger->loggedMessages();

            connect(_logger, &QOpenGLDebugLogger::messageLogged,
                    this, &OpenGLDebugLogger::onMessageLogged, Qt::DirectConnection);

            if(!startupMessages.isEmpty())
            {
                for(auto startupMessage : startupMessages)
                    onMessageLogged(startupMessage);
            }

            _logger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
            _logger->enableMessages();
        }
        else
        {
            qDebug() << "Debugging requested but logger failed to initialize";
        }
    }
}

OpenGLDebugLogger::~OpenGLDebugLogger()
{
    if(_logger != nullptr)
    {
        _logger->disableMessages();
        _logger->stopLogging();
    }
}

void OpenGLDebugLogger::onMessageLogged(const QOpenGLDebugMessage& message)
{
    if(!(message.severity() & _debugLevel))
        return;

    qDebug() << "OpenGL:" << message.message();
}
