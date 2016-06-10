#ifndef BASEPLUGIN_H
#define BASEPLUGIN_H

#include "shared/interfaces/iplugin.h"
#include "shared/graph/igraph.h"
#include "shared/graph/igraphmodel.h"
#include "shared/loading/urltypes.h"

#include <QObject>

class Graph;

// Provide some default implementations that most plugins will require, to avoid
// them having to repetitively implement the same functionality themselves
class BasePluginInstance : public QObject, public IPluginInstance
{
    Q_OBJECT

private:
    IGraphModel* _graphModel = nullptr;

public:
    IGraphModel* graphModel() { return _graphModel; }

    void setGraphModel(IGraphModel* graphModel)
    {
        _graphModel = graphModel;

        auto qObject = dynamic_cast<const QObject*>(&graphModel->graph());
        Q_ASSERT(qObject != nullptr);

        connect(qObject, SIGNAL(graphWillChange(const Graph*)), this, SIGNAL(graphWillChange()), Qt::DirectConnection);

        connect(qObject, SIGNAL(nodeAdded(const Graph*, NodeId)), this,
                SLOT(onNodeAdded(const Graph*, NodeId)), Qt::DirectConnection);
        connect(qObject, SIGNAL(nodeRemoved(const Graph*, NodeId)), this,
                SLOT(onNodeRemoved(const Graph*, NodeId)), Qt::DirectConnection);
        connect(qObject, SIGNAL(edgeAdded(const Graph*, EdgeId)), this,
                SLOT(onEdgeAdded(const Graph*, EdgeId)), Qt::DirectConnection);
        connect(qObject, SIGNAL(edgeRemoved(const Graph*, EdgeId)), this,
                SLOT(onEdgeRemoved(const Graph*, EdgeId)), Qt::DirectConnection);

        connect(qObject, SIGNAL(graphChanged(const Graph*)), this, SIGNAL(graphChanged()), Qt::DirectConnection);
    }

private slots:
    void onNodeAdded(const Graph*, NodeId nodeId) const   { emit nodeAdded(nodeId); }
    void onNodeRemoved(const Graph*, NodeId nodeId) const { emit nodeRemoved(nodeId); }
    void onEdgeAdded(const Graph*, EdgeId edgeId) const   { emit edgeAdded(edgeId); }
    void onEdgeRemoved(const Graph*, EdgeId edgeId) const { emit edgeRemoved(edgeId); }

signals:
    void graphWillChange() const;

    void nodeAdded(NodeId) const;
    void nodeRemoved(NodeId) const;
    void edgeAdded(EdgeId) const;
    void edgeRemoved(EdgeId) const;

    void graphChanged() const;
};

class BasePlugin : public QObject, public IPlugin, public UrlTypes
{
    Q_OBJECT
    Q_INTERFACES(IPlugin)
};

#endif // BASEPLUGIN_H
