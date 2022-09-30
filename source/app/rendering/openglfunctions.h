/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

#include <QOpenGLFunctions_4_0_Core>
#include <QString>

class OpenGLFunctions : public QOpenGLFunctions_4_0_Core
{
public:
    void resolveOpenGLFunctions();

    static bool hasOpenGLSupport();
    static QString vendor();
    static QString info();

    static QSurfaceFormat minimumFormat();
};

#endif // OPENGLFUNCTIONS_H
