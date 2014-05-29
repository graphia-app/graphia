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
    GraphScene* graphScene;
    GraphModel* _graphModel;
    SelectionManager* _selectionManager;

public:
    void setGraphModel(GraphModel* graphModel)
    {
        this->_graphModel = graphModel;
        graphScene->setGraphModel(graphModel);
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
    bool m_rightMouseButtonHeld;
    bool m_leftMouseButtonHeld;

    bool m_selecting;
    bool m_frustumSelecting;
    QPoint m_frustumSelectStart;

    QPoint m_prevCursorPosition;
    QPoint m_cursorPosition;
    bool m_mouseMoving;
    NodeId clickedNodeId;

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
