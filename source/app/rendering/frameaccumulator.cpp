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

#include "frameaccumulator.h"
#include "shadertools.h"

#include <QString>

using namespace Qt::Literals::StringLiterals;

const int FrameAccumulator::MAX_SAMPLES = 32;

FrameAccumulator::FrameAccumulator()
{
    resolveOpenGLFunctions();

    ShaderTools::loadShaderProgram(_copyShader, u":/shaders/fullscreen.vert"_s, u":/shaders/copy.frag"_s);
}

bool FrameAccumulator::resize(int width, int height)
{
    restart();

    _width = width;
    _height = height;

    return _mean.resize(width, height);
}

void FrameAccumulator::setEnabled(bool enabled)
{
    if(enabled == _enabled)
        return;

    _enabled = enabled;
    restart();
}

void FrameAccumulator::restart()
{
    _numSamples = 0;
}

QVector2D FrameAccumulator::nextSampleOffset() const
{
    if(_numSamples == 0)
        return {};

    auto radicalInverse = [](int index, int base)
    {
        float result = 0.0f;
        float fraction = 1.0f;

        while(index > 0)
        {
            fraction /= static_cast<float>(base);
            result += fraction * static_cast<float>(index % base);
            index /= base;
        }

        return result;
    };

    return {radicalInverse(_numSamples, 2) - 0.5f, radicalInverse(_numSamples, 3) - 0.5f};
}

void FrameAccumulator::add(GLuint image)
{
    _mean.bindForDrawing();

    blend(image, 1.0f / static_cast<float>(_numSamples + 1));

    _numSamples++;
}

void FrameAccumulator::draw(GLuint framebuffer)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, _width, _height);

    blend(_mean.texture(), 1.0f);
}

void FrameAccumulator::blend(GLuint image, float alpha)
{
    if(alpha < 1.0f)
    {
        glBlendColor(0.0f, 0.0f, 0.0f, alpha);
        glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
        glEnable(GL_BLEND);
    }
    else
        glDisable(GL_BLEND);

    _screenPass.draw(_copyShader, {image});

    glDisable(GL_BLEND);
}
