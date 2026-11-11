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

#ifndef OPENGLFUNCTIONS_H
#define OPENGLFUNCTIONS_H

#ifdef OPENGL_ES
#include <QOpenGLFunctions_ES2>
#else
#include <QOpenGLFunctions_3_3_Core>
#endif

#include <QString>
#include <QSurfaceFormat>

#ifdef OPENGL_ES
class OpenGLFunctions : public QOpenGLFunctions_ES2
#else
class OpenGLFunctions : public QOpenGLFunctions_3_3_Core
#endif
{
public:
    void resolveOpenGLFunctions();

    static bool hasOpenGLSupport();
    static QString vendor();
    static QString info();

    static QSurfaceFormat minimumFormat();
    static void requestMinimumFormat();
};

#endif // OPENGLFUNCTIONS_H
