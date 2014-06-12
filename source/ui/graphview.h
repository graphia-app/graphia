#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QWidget>
#include <QSurfaceFormat>

#include "../gl/graphscene.h"

class GraphModel;
class CommandManager;
class SelectionManager;

class GraphView : public QWidget
{
    Q_OBJECT
public:
    explicit GraphView(GraphModel* graphModel,
                       CommandManager* commandManager,
                       SelectionManager* selectionManager,
                       QWidget *parent = nullptr);

private:
    GraphScene* _graphScene;
    GraphModel* _graphModel;
    CommandManager* _commandManager;
    SelectionManager* _selectionManager;

public:
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
    
private:
    bool _rightMouseButtonHeld;
    bool _leftMouseButtonHeld;

    bool _selecting;
    bool _frustumSelecting;
    QPoint _frustumSelectStart;

    QPoint _prevCursorPosition;
    QPoint _cursorPosition;
    QPoint _clickPosition;
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
