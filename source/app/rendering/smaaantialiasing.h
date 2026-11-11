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

#ifndef SMAAANTIALIASING_H
#define SMAAANTIALIASING_H

#include "openglfunctions.h"
#include "screenpass.h"
#include "screentexture.h"

#include <QOpenGLShaderProgram>
#include <QString>

#include <initializer_list>

// Subpixel morphological antialiasing (https://github.com/iryoku/smaa), with the
// silhouettes found where the element shown changes
class SmaaAntialiasing : public OpenGLFunctions
{
public:
    SmaaAntialiasing();
    ~SmaaAntialiasing() override;

    SmaaAntialiasing(const SmaaAntialiasing&) = delete;
    SmaaAntialiasing& operator=(const SmaaAntialiasing&) = delete;

    bool resize(int width, int height);

    void findSilhouettes(GLuint elements);

    // Draws colors into framebuffer, blended across the silhouettes last found;
    // colors must filter linearly
    void blend(GLuint colors, GLuint framebuffer);

    // How much each pixel is blended with which neighbour, by the silhouettes last found
    GLuint weights() const { return _weights.texture(); }

private:
    int _width = 0;
    int _height = 0;

    ScreenTexture _edges{GL_RG8, GL_RG, GL_UNSIGNED_BYTE, GL_LINEAR};
    ScreenTexture _weights{GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, GL_LINEAR};

    GLuint _areaTable = 0;
    GLuint _searchTable = 0;

    QOpenGLShaderProgram _edgesShader;
    QOpenGLShaderProgram _weightsShader;
    QOpenGLShaderProgram _blendShader;
    ScreenPass _screenPass;

    GLuint loadTable(const QString& resource);
    void drawPass(QOpenGLShaderProgram& shader, std::initializer_list<GLuint> textures);
};

#endif // SMAAANTIALIASING_H
