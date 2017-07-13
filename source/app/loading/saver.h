#ifndef SAVER_H
#define SAVER_H

#include <QUrl>
#include <QStringList>
#include <QString>
#include <QByteArray>

class IGraph;
class NodePositions;

class Saver
{
private:
    QUrl _fileUrl;

    const IGraph* _graph = nullptr;
    const NodePositions* _nodePositions = nullptr;

    QStringList _transforms;
    QStringList _visualisations;

    QString _pluginName;
    QByteArray _pluginData;

public:
    Saver(const QUrl& fileUrl) { _fileUrl = fileUrl; }

    QUrl fileUrl() const { return _fileUrl; }

    void addGraph(const IGraph& graph) { _graph = &graph; }
    void addNodePositions(const NodePositions& nodePositions) { _nodePositions = &nodePositions; }

    void addTransforms(const QStringList& transforms) { _transforms = transforms; }
    void addVisualisations(const QStringList& visualisations) { _visualisations = visualisations; }

    void setPlugin(const QString& pluginName) { _pluginName = pluginName; }
    void addPluginData(const QByteArray& pluginData) { _pluginData = pluginData; }

    bool encode();
};

#endif // SAVER_H
