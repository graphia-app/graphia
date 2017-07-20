#ifndef SAVER_H
#define SAVER_H

#include "shared/loading/progressfn.h"

#include <QUrl>
#include <QStringList>
#include <QString>
#include <QByteArray>

class GraphModel;
class IPluginInstance;

class Saver
{
private:
    QUrl _fileUrl;

    GraphModel* _graphModel = nullptr;
    const IPluginInstance* _pluginInstance = nullptr;

public:
    static const int MaxHeaderSize = 1 << 12;

    Saver(const QUrl& fileUrl) { _fileUrl = fileUrl; }

    QUrl fileUrl() const { return _fileUrl; }

    void setGraphModel(GraphModel* graphModel) { _graphModel = graphModel; }
    void setPluginInstance(const IPluginInstance* pluginInstance) { _pluginInstance = pluginInstance; }

    bool encode(const ProgressFn& progressFn);
};

#endif // SAVER_H
