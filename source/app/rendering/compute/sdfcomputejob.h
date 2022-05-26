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

#ifndef SDFCOMPUTEJOB_H
#define SDFCOMPUTEJOB_H

#include "gpucomputejob.h"

#include "rendering/glyphmap.h"
#include "rendering/doublebufferedtexture.h"

#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

#include <functional>

class SDFComputeJob : public GPUComputeJob
{
private:
    DoubleBufferedTexture* _sdfTexture;
    GlyphMap* _glyphMap;

    std::function<void()> _onCompleteFn;

    void generateSDF();

public:
    SDFComputeJob(DoubleBufferedTexture* sdfTexture, GlyphMap *glyphMap);

    void run() override;
    void executeWhenComplete(std::function<void()> onCompleteFn);
};

#endif // SDFCOMPUTEJOB_H
