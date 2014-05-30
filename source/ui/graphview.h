#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QWidget>
#include <QSurfaceFormat>

#include "../gl/graphscene.h"

class GraphModel;
class SelectionManager;

class GraphView : public QWidget
{
    Q_OBJECT
public:
    explicit GraphView(QWidget *parent = nullptr);

private:
    GraphScene* _graphScene;
    GraphModel* _graphModel;
    SelectionManager* _selectionManager;

public:
    void setGraphModel(GraphModel* graphModel)
    {
        this->_graphModel = graphModel;
        _graphScene->setGraphModel(graphModel);
    }
    GraphModel* graphModel() { return _graphModel; }

    void setSelectionManager(SelectionManager* selectionManager);
    SelectionManager* selectionManager() { return _selectionManager; }

    static QSurfaceFormat& surfaceFormat()
    {
        static QSurfaceFormat format;
        static bool initialised = false;

        if(!initialised)
        {
            format.setMajorVersion(3);
            format.setMinorVersion(3);

            format.setDepthBufferSize(24);
            format.setSamples(GraphScene::multisamples);
            format.setProfile(QSurfaceFormat::CoreProfile);
            initialised = true;
        }

        return format;
    }

signals:
    void userInteractionStarted();
    void userInteractionFinished();

public slots:
    void layoutChanged();
    
private:
    bool _rightMouseButtonHeld;
    bool _leftMouseButtonHeld;

    bool _selecting;
    bool _frustumSelecting;
    QPoint _frustumSelectStart;

    QPoint _prevCursorPosition;
    QPoint _cursorPosition;
    bool _mouseMoving;
    NodeId _clickedNodeId;

protected:
    friend class OpenGLWindow;
    void mousePressEvent(QMouseEvent* mouseEvent);
    void mouseReleaseEvent(QMouseEvent* mouseEvent);
    void mouseMoveEvent(QMouseEvent* mouseEvent);
    void mouseDoubleClickEvent(QMouseEvent* mouseEvent);
    void keyPressEvent(QKeyEvent* keyEvent);
    void keyReleaseEvent(QKeyEvent* keyEvent);
    void wheelEvent(QWheelEvent* wheelEvent);
};

#endif // GRAPHVIEW_H
