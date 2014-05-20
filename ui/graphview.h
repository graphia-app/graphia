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

    void setSelectionManager(SelectionManager* selectionManager)
    {
        this->_selectionManager = selectionManager;
        graphScene->setSelectionManager(selectionManager);
    }
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
    
public slots:
    void layoutChanged();
    
};

#endif // GRAPHVIEW_H
