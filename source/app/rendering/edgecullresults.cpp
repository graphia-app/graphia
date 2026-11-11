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

#include "edgecullresults.h"

#include <algorithm>

EdgeCullResults::EdgeCullResults() :
    _walks{{{2, GL_RG32F, GL_RG, 1}, {2, GL_RG32F, GL_RG, 1}}}
{
    resolveOpenGLFunctions();
}

void EdgeCullResults::reserve(size_t numEdges)
{
    bool reallocated = false;

    for(auto& walk : _walks)
        reallocated = walk.reserve(numEdges) || reallocated;

    if(reallocated)
        _numWritten = 0;
}

bool EdgeCullResults::usable() const
{
    return std::ranges::all_of(_walks, [](const auto& walk) { return walk.usable(); });
}

void EdgeCullResults::bindForWriting(QOpenGLShaderProgram& shader)
{
    _latest = 1 - _latest;
    _walks.at(_latest).bindForWriting();
    setSizeUniform(shader);

    _numWritten = std::min(_numWritten + 1, static_cast<int>(_walks.size()));
}

void EdgeCullResults::bind(GLenum textureUnit, QOpenGLShaderProgram& shader)
{
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, _walks.at(_latest).texture(0));
    setSizeUniform(shader);
}

void EdgeCullResults::bindPrevious(GLenum textureUnit, QOpenGLShaderProgram& shader)
{
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, _walks.at(1 - _latest).texture(0));
    shader.setUniformValue("hasPreviousVisibleRanges", _numWritten > 1 ? 1 : 0);
}

void EdgeCullResults::release(GLenum textureUnit)
{
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void EdgeCullResults::setSizeUniform(QOpenGLShaderProgram& shader) const
{
    shader.setUniformValue("visibleRangesSize", _walks.at(_latest).size());
}
