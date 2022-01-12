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

#ifndef SCREENSHOTRENDERER_H
#define SCREENSHOTRENDERER_H

#include "graph/graphmodel.h"
#include "graphrenderer.h"
#include "graphrenderercore.h"
#include "graphcomponentrenderer.h"

#include <QObject>

class ScreenshotRenderer : public QObject, public GraphRendererCore
{
    Q_OBJECT

    enum class ScreenshotType { Preview, Tile };

public:
    ScreenshotRenderer();
    explicit ScreenshotRenderer(const GraphRenderer& renderer);
    ~ScreenshotRenderer() override;

    void requestPreview(const GraphRenderer& renderer, int width, int height, bool fillSize);
    void requestScreenshot(const GraphRenderer& renderer, int width, int height, const QString& path, int dpi,
                           bool fillSize);

private:
    GraphModel* _graphModel = nullptr;


    GLuint _screenshotFBO = 0;
    GLuint _screenshotTex = 0;
    GLuint _sdfTexture = 0;

    std::vector<GraphComponentRenderer::CameraAndLighting> _componentCameraAndLightings;

    GLuint sdfTexture() const override;

    void render();
    bool copyState(const GraphRenderer& renderer);
    void updateComponentGPUData(ScreenshotType screenshotType, QSize screenshotSize,
        QSize viewportSize, int tileX = 0, int tileY = 0);

signals:
    // Base64 encoded png image for QML...
    void previewComplete(QString previewBase64);
    // Screenshot doesn't go to QML so we can use QImage
    void screenshotComplete(const QImage& screenshot, const QString& path);
};

#endif // SCREENSHOTRENDERER_H
