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

#ifndef FRAMEACCUMULATOR_H
#define FRAMEACCUMULATOR_H

#include "openglfunctions.h"
#include "screenpass.h"
#include "screentexture.h"

#include <QElapsedTimer>
#include <QOpenGLShaderProgram>
#include <QVector2D>

// Averages frames drawn at points within each pixel, as a supersampled image would
// The interim frame is shown until the average is complete
class FrameAccumulator : public OpenGLFunctions
{
public:
    FrameAccumulator();

    FrameAccumulator(const FrameAccumulator&) = delete;
    FrameAccumulator& operator=(const FrameAccumulator&) = delete;

    bool resize(int width, int height);
    void setEnabled(bool enabled);

    void setFinishedOnly(bool finishedOnly);

    void restart();
    bool complete() const { return _numSamples >= MAX_SAMPLES; }
    bool gathering() const { return _enabled && !complete(); }
    bool handingOver() const { return complete() && !_finishedOnly && _gatheredAlpha < 1.0f; }

    // Whether the next picture is drawn with everything traced and added to the mean
    bool nextFrameIsSample() const { return gathering() && (_interimFrameDrawn || _finishedOnly); }

    // In pixels, from the middle of the pixel
    QVector2D nextSampleOffset() const;

    void add(GLuint image);

    GLuint beginInterimFrame();
    void draw(GLuint framebuffer);

private:
    static const int MAX_SAMPLES;

    bool _enabled = true;
    bool _finishedOnly = false;
    int _numSamples = 0;
    int _width = 0;
    int _height = 0;

    ScreenTexture _mean{GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, GL_LINEAR};

    ScreenTexture _interimFrame{GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, GL_NEAREST};
    bool _interimFrameDrawn = false;

    QElapsedTimer _handover;
    float _gatheredAlpha = 0.0f;

    QOpenGLShaderProgram _copyShader;
    ScreenPass _screenPass;

    void drawMean(GLuint framebuffer, float alpha);
    void blend(GLuint image, float alpha);
};

#endif // FRAMEACCUMULATOR_H
