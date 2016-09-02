#ifndef GENERICPLUGIN_H
#define GENERICPLUGIN_H

#include "shared/plugins/basegenericplugin.h"
#include "app/graph/mutablegraph.h"

class BuilderPluginInstance : public BaseGenericPluginInstance
{
    Q_OBJECT
public:
    BuilderPluginInstance();

    void onSelectionChanged(const ISelectionManager* selectionManager);
    Q_INVOKABLE void addNodeToSelected();
};

class BuilderPlugin : public BaseGenericPlugin, public PluginInstanceProvider<BuilderPluginInstance>
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID IPluginIID FILE "builderplugin.json")

public:
    QString name() const { return "Builder"; }
    QString description() const
    {
        return tr("A plugin that builds graphs");
    }
    QString imageSource() const { return "qrc:///tools.svg"; }
    QString qmlPath() const { return "qrc:///qml/builderplugin.qml"; }
};

#endif // GENERICPLUGIN_H

BuilderPluginInstance::BuilderPluginInstance()
{
    connect(this, &BuilderPluginInstance::selectionChanged, this, &BuilderPluginInstance::onSelectionChanged);
}

void BuilderPluginInstance::onSelectionChanged(const ISelectionManager* selectionManager)
{

}

void BuilderPluginInstance::addNodeToSelected()
{
    auto* graph = dynamic_cast<MutableGraph*>(&(graphModel()->mutableGraph()));
    auto selectedNodes = selectionManager()->selectedNodes();
    graph->performTransaction(
                [this, &selectedNodes](MutableGraph& graph){
        {
                for(auto nodeId : selectedNodes)
                {
                    auto newNodeId = graph.addNode();
                    graph.addEdge(nodeId, newNodeId);
                }
        }
    });


}
