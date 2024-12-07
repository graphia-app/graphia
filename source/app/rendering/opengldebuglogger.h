/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#ifndef OPENGLDEBUGLOGGER_H
#define OPENGLDEBUGLOGGER_H

#include <QObject>

class QOpenGLDebugLogger;
class QOpenGLDebugMessage;

class OpenGLDebugLogger : public QObject
{
    Q_OBJECT
public:
    explicit OpenGLDebugLogger(QObject* parent = nullptr);
    ~OpenGLDebugLogger() override; // modernize-use-equals-delete

private:
    int _debugLevel = 0;
    QOpenGLDebugLogger* _logger = nullptr;

private slots:
    void onMessageLogged(const QOpenGLDebugMessage& message) const;
};

#endif // OPENGLDEBUGLOGGER_H
