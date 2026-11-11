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

#ifndef EDGEDATATEXELS_H
#define EDGEDATATEXELS_H

#include "openglfunctions.h"

#include <QVector2D>

#include <cstddef>
#include <vector>

// Render targets with the same number of texels for each edge, which a shader
// writes by drawing a point at each
class EdgeDataTexels : public OpenGLFunctions
{
public:
    EdgeDataTexels(int texelsPerEdge, GLenum internalFormat, GLenum format, size_t numTargets);
    ~EdgeDataTexels() override;

    EdgeDataTexels(const EdgeDataTexels&) = delete;
    EdgeDataTexels& operator=(const EdgeDataTexels&) = delete;

    // Returns true when more room was needed, losing anything written
    bool reserve(size_t numEdges);
    bool usable() const { return _usable; }

    void bindForWriting();

    GLuint texture(size_t target) const { return _textures.at(target); }
    QVector2D size() const;

private:
    int _texelsPerEdge;
    GLenum _internalFormat;
    GLenum _format;
    std::vector<GLuint> _textures;
    GLuint _fbo = 0;
    int _width = 0;
    int _height = 0;
    bool _usable = false;
};

#endif // EDGEDATATEXELS_H
