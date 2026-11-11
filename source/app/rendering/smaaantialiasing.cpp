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

#include "smaaantialiasing.h"
#include "shadertools.h"

#include <QImage>
#include <QString>
#include <QVector4D>
#include <QtGlobal>

using namespace Qt::Literals::StringLiterals;

SmaaAntialiasing::SmaaAntialiasing()
{
    resolveOpenGLFunctions();

    ShaderTools::loadShaderProgram(_edgesShader, u":/shaders/fullscreen.vert"_s, u":/shaders/smaaedges.frag"_s);
    ShaderTools::loadShaderProgram(_weightsShader, u":/shaders/fullscreen.vert"_s, u":/shaders/smaaweights.frag"_s);
    ShaderTools::loadShaderProgram(_blendShader, u":/shaders/fullscreen.vert"_s, u":/shaders/smaablend.frag"_s);

    _edgesShader.bind();
    _edgesShader.setUniformValue("elements", 0);
    _weightsShader.bind();
    _weightsShader.setUniformValue("edges", 0);
    _weightsShader.setUniformValue("areaTable", 1);
    _weightsShader.setUniformValue("searchTable", 2);
    _blendShader.bind();
    _blendShader.setUniformValue("colors", 0);
    _blendShader.setUniformValue("weights", 1);
    _blendShader.release();

    _areaTable = loadTable(u":/textures/smaaarea.png"_s);
    _searchTable = loadTable(u":/textures/smaasearch.png"_s);
}

SmaaAntialiasing::~SmaaAntialiasing()
{
    glDeleteTextures(1, &_searchTable);
    glDeleteTextures(1, &_areaTable);
}

GLuint SmaaAntialiasing::loadTable(const QString& resource)
{
    const auto image = QImage(resource).convertToFormat(QImage::Format_RGBA8888);
    Q_ASSERT(!image.isNull());

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width(), image.height(), 0,
        GL_RGBA, GL_UNSIGNED_BYTE, image.constBits());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

bool SmaaAntialiasing::resize(int width, int height)
{
    _width = width;
    _height = height;

    return _edges.resize(width, height) && _weights.resize(width, height);
}

void SmaaAntialiasing::drawPass(QOpenGLShaderProgram& shader, std::initializer_list<GLuint> textures)
{
    shader.bind();
    shader.setUniformValue("imageMetrics", QVector4D(1.0f / static_cast<float>(_width),
        1.0f / static_cast<float>(_height), static_cast<float>(_width), static_cast<float>(_height)));

    _screenPass.draw(shader, textures);
}

void SmaaAntialiasing::findSilhouettes(GLuint elements)
{
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    // The edges pass discards where there is no silhouette
    _edges.bindForDrawing();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    drawPass(_edgesShader, {elements});

    _weights.bindForDrawing();
    drawPass(_weightsShader, {_edges.texture(), _areaTable, _searchTable});
}

void SmaaAntialiasing::blend(GLuint colors, GLuint framebuffer)
{
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, _width, _height);
    drawPass(_blendShader, {colors, _weights.texture()});
}
