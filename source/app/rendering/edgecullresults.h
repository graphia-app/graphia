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

#ifndef EDGECULLRESULTS_H
#define EDGECULLRESULTS_H

#include "edgedatatexels.h"

#include <QOpenGLShaderProgram>

#include <array>
#include <cstddef>

class EdgeCullResults : public OpenGLFunctions
{
public:
    EdgeCullResults();

    // Anything written is lost if more room is needed
    void reserve(size_t numEdges);
    bool usable() const;

    bool written() const { return _numWritten > 0; }
    void bindForWriting(QOpenGLShaderProgram& shader);

    void bind(GLenum textureUnit, QOpenGLShaderProgram& shader);
    void bindPrevious(GLenum textureUnit, QOpenGLShaderProgram& shader);
    void release(GLenum textureUnit);

private:
    std::array<EdgeDataTexels, 2> _walks;
    size_t _latest = 0;
    int _numWritten = 0;

    void setSizeUniform(QOpenGLShaderProgram& shader) const;
};

#endif // EDGECULLRESULTS_H
