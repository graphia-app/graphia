#ifndef BASEPLUGIN_H
#define BASEPLUGIN_H

#include "shared/interfaces/iplugin.h"
#include "shared/graph/igraph.h"
#include "shared/graph/igraphmodel.h"
#include "shared/ui/iselectionmanager.h"
#include "shared/loading/urltypes.h"

#include <QObject>

// The plugins never see these types; they only need to know they exist
// for the purposes of signal connection
class Graph;
class SelectionManager;

// Provide some default implementations that most plugins will require, to avoid
// them having to repetitively implement the same functionality themselves
class BasePluginInstance : public QObject, public IPluginInstance
{
    Q_OBJECT

private:
    IGraphModel* _graphModel = nullptr;
    ISelectionManager* _selectionManager = nullptr;

public:
    void initialise(IGraphModel* graphModel, ISelectionManager* selectionManager)
    {
        _graphModel = graphModel;
        _selectionManager = selectionManager;

        auto graphQObject = dynamic_cast<const QObject*>(&graphModel->graph());
        Q_ASSERT(graphQObject != nullptr);

        connect(graphQObject, SIGNAL(graphWillChange(const Graph*)),
                this, SIGNAL(graphWillChange()), Qt::DirectConnection);

        connect(graphQObject, SIGNAL(nodeAdded(const Graph*, NodeId)),
                this, SLOT(onNodeAdded(const Graph*, NodeId)), Qt::DirectConnection);
        connect(graphQObject, SIGNAL(nodeRemoved(const Graph*, NodeId)),
                this, SLOT(onNodeRemoved(const Graph*, NodeId)), Qt::DirectConnection);
        connect(graphQObject, SIGNAL(edgeAdded(const Graph*, EdgeId)),
                this, SLOT(onEdgeAdded(const Graph*, EdgeId)), Qt::DirectConnection);
        connect(graphQObject, SIGNAL(edgeRemoved(const Graph*, EdgeId)),
                this, SLOT(onEdgeRemoved(const Graph*, EdgeId)), Qt::DirectConnection);

        connect(graphQObject, SIGNAL(graphChanged(const Graph*)),
                this, SIGNAL(graphChanged()), Qt::DirectConnection);

        auto selectionManagerQObject = dynamic_cast<const QObject*>(selectionManager);
        Q_ASSERT(selectionManagerQObject != nullptr);

        connect(selectionManagerQObject, SIGNAL(selectionChanged(const SelectionManager*)),
                this, SLOT(onSelectionChanged(const SelectionManager*)), Qt::DirectConnection);
    }

    IGraphModel* graphModel() { return _graphModel; }
    ISelectionManager* selectionManager() { return _selectionManager; }

private slots:
    void onNodeAdded(const Graph*, NodeId nodeId) const   { emit nodeAdded(nodeId); }
    void onNodeRemoved(const Graph*, NodeId nodeId) const { emit nodeRemoved(nodeId); }
    void onEdgeAdded(const Graph*, EdgeId edgeId) const   { emit edgeAdded(edgeId); }
    void onEdgeRemoved(const Graph*, EdgeId edgeId) const { emit edgeRemoved(edgeId); }

    void onSelectionChanged(const SelectionManager*) const { emit selectionChanged(_selectionManager); }

signals:
    void graphWillChange() const;

    void nodeAdded(NodeId) const;
    void nodeRemoved(NodeId) const;
    void edgeAdded(EdgeId) const;
    void edgeRemoved(EdgeId) const;

    void graphChanged() const;

    void selectionChanged(const ISelectionManager* selectionManager) const;
};

class BasePlugin : public QObject, public IPlugin, public UrlTypes
{
    Q_OBJECT
    Q_INTERFACES(IPlugin)
};

#endif // BASEPLUGIN_H
