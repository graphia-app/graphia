#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QWidget>
#include <QSurfaceFormat>

#include "../graph/graphmodel.h"
#include "../gl/graphscene.h"

class GraphView : public QWidget
{
    Q_OBJECT
public:
    explicit GraphView(QWidget *parent = 0);

private:
    GraphScene* graphScene;
    GraphModel* _graphModel;

public:
    void setGraphModel(GraphModel* graphModel)
    {
        this->_graphModel = graphModel;
        graphScene->setGraphModel(graphModel);
    }
    GraphModel* graphModel() { return _graphModel; }

    static QSurfaceFormat& surfaceFormat()
    {
        static QSurfaceFormat format;
        static bool initialised = false;

        if(!initialised)
        {
            format.setMajorVersion(3);
            format.setMinorVersion(3);

            format.setDepthBufferSize(24);
            format.setSamples(4);
            format.setProfile(QSurfaceFormat::CoreProfile);
            initialised = true;
        }

        return format;
    }

signals:
    
public slots:
    
};

#endif // GRAPHVIEW_H
