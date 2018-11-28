#ifndef SCREENSHOTRENDERER_H
#define SCREENSHOTRENDERER_H

#include "graph/graphmodel.h"
#include "graphrenderer.h"
#include "graphrenderercore.h"
#include "shared/graph/grapharray.h"

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

    const int TILE_SIZE = 1024;

    GLuint _screenshotFBO = 0;
    GLuint _screenshotTex = 0;
    GLuint _sdfTexture = 0;

    std::vector<Camera> _componentCameras;
    std::vector<QRectF> _componentViewports;

    OpenGLDebugLogger _openGLDebugLogger;

    GLuint sdfTexture() const override;

    void render();
    bool cloneState(const GraphRenderer& renderer);
    void fetchPreview();
    void fetchAndDrawTile(QPixmap &fullScreenshot, QSize screenshotSize, int currentTileX, int currentTileY);
    void updateComponentGPUData(ScreenshotType screenshotType, QSize screenshotSize,
                                QSize viewportSize, int currentTileX=0, int currentTileY=0);
    QMatrix4x4 subViewportMatrix(QRectF scaledDimensions, QSize screenshotSize);
signals:
    // Base64 encoded png image for QML...
    void previewComplete(QString previewBase64) const;
    // Screenshot doesn't go to QML so we can use QImage
    void screenshotComplete(const QImage& screenshot, const QString& path) const;
};

#endif // SCREENSHOTRENDERER_H
