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

#ifndef SCREENPASS_H
#define SCREENPASS_H

#include "openglfunctions.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

#include <initializer_list>

// Draws a shader over the whole of what is bound, with fullscreen.vert
class ScreenPass : public OpenGLFunctions
{
public:
    ScreenPass();

    ScreenPass(const ScreenPass&) = delete;
    ScreenPass& operator=(const ScreenPass&) = delete;

    // Binds textures to the units from 0, in order
    void draw(QOpenGLShaderProgram& shader, std::initializer_list<GLuint> textures = {});

private:
    QOpenGLVertexArrayObject _triangle;
};

#endif // SCREENPASS_H
