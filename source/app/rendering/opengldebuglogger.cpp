/* Copyright © 2013-2021 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "opengldebuglogger.h"

#include <QOpenGLDebugLogger>
#include <QOpenGLContext>
#include <QDebug>

OpenGLDebugLogger::OpenGLDebugLogger(QObject* parent) :
    QObject(parent), _debugLevel(qEnvironmentVariableIntValue("OPENGL_DEBUG"))
{
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
                for(const auto& startupMessage : startupMessages)
                    onMessageLogged(startupMessage);
            }

            _logger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
            _logger->enableMessages();
        }
        else
        {
            qDebug() << "Debugging requested but logger failed to initialize";

            const auto* context = QOpenGLContext::currentContext();
            Q_ASSERT(context != nullptr);

            if(!context->hasExtension(QByteArrayLiteral("GL_KHR_debug")))
                qDebug() << "...GL_KHR_debug not available";
        }
    }
}

OpenGLDebugLogger::~OpenGLDebugLogger()
{
    if(_logger != nullptr && _logger->isLogging())
    {
        _logger->disableMessages();
        _logger->stopLogging();
    }
}

void OpenGLDebugLogger::onMessageLogged(const QOpenGLDebugMessage& message) const
{
    if((message.severity() & _debugLevel) == 0)
        return;

    qDebug() << "OpenGL:" << message.message();
}
