#ifndef SCREENSHOTRENDERER_H
#define SCREENSHOTRENDERER_H

#include "graphrenderer.h"
#include "graph/graphmodel.h"
#include "graphrenderercore.h"
#include "shared/graph/grapharray.h"

#include <QObject>

class ScreenshotRenderer :
        public QObject,
        public GraphRendererCore
{
    Q_OBJECT

public:
    ScreenshotRenderer(GraphRenderer& renderer);
    virtual ~ScreenshotRenderer() = default;

    void copyTextureObject();

    void updateGPU();
public slots:
    void onPreviewRequested(int width, int height, bool fillSize);
    void onScreenshotRequested(int width, int height, const QString& path, int dpi, bool fillSize);


private:
    GraphModel* _graphModel = nullptr;
    int _numComponents = 0;

    const int TILE_SIZE = 1024;

    GLuint _screenshotFBO = 0;
    GLuint _screenshotTex = 0;

    bool _isScreenshot = false;
    bool _isPreview = false;
    int _screenshotHeight = 0;
    int _screenshotWidth = 0;
    int _currentTileX = 0;
    int _currentTileY = 0;
    int _tileXCount = 0;
    int _tileYCount = 0;
    QPixmap _fullScreenshot;

    std::unique_ptr<GlyphMap> _glyphMap;

    // Store a copy of the text layout results as its computation is a long running
    // process that occurs in a separate thread; we don't want to be rendering from
    // a set of results that is currently changing
    GlyphMap::Results _textLayoutResults;

    // It's important that these are pointers and not values, because the array will
    // be resized during ComponentManager::update, and we still want to be
    // able to use the existing renderers while this occurs. If the array stored
    // values, then the storage for the renderers themselves would potentially be
    // moved around, as opposed to just the storage for the pointers.
    ComponentArray<MovablePointer<GraphComponentRenderer>, LockingGraphArray> _componentRenderers;
    bool _transitionPotentiallyInProgress = false;
    DeferredExecutor _preUpdateExecutor;

    enum class Mode
    {
        Overview,
        Component
    };

    Scene* _scene = nullptr;
    GraphOverviewScene* _graphOverviewScene;
    GraphComponentScene* _graphComponentScene;
    SelectionManager* _selectionManager;

    Interactor* _interactor = nullptr;
    GraphOverviewInteractor* _graphOverviewInteractor;
    GraphComponentInteractor* _graphComponentInteractor;

    OpenGLDebugLogger _openGLDebugLogger;

    QOpenGLShaderProgram _debugLinesShader;

    bool _resized = false;

    GLuint _colorTexture = 0;

    GLuint _sdfTexture;

    bool _FBOcomplete = false;

    // When elements are added to the scene, it may be that they would lie
    // outside the confines of where they should be rendered, until a transition
    // is completed, so these arrays allow us to hide the elements until such
    // transitions are complete
    NodeArray<bool> _hiddenNodes;
    EdgeArray<bool> _hiddenEdges;

    bool _gpuDataRequiresUpdate = false;

    QRect _selectionRect;

    QTime _time;
    float _lastTime = 0.0f;
    int _sceneUpdateDisabled = 1;
    std::recursive_mutex _sceneUpdateMutex;

    std::atomic<bool> _layoutChanged;
    bool _synchronousLayoutChanged = false;

    GLuint sdfTexture() const override;

    void render();
    void updateComponentGPUData();
    void updateGPUDataIfRequired();
    GraphComponentRenderer *componentRendererForId(ComponentId componentId) const;
signals:
    // Base64 encoded png image for QML...
    void previewComplete(QString previewBase64) const;
    // Screenshot doesn't go to QML so we can use QImage
    void screenshotComplete(const QImage& screenshot, const QString& path) const;
};

#endif // SCREENSHOTRENDERER_H
