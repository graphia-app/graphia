#ifndef LOADER_H
#define LOADER_H

#include "shared/loading/iparser.h"
#include "shared/plugins/iplugin.h"

#include "shared/graph/elementid_containers.h"

#include "layout/nodepositions.h"

#include <QString>
#include <QStringList>
#include <QByteArray>

#include <memory>
#include <map>

class Loader : public IParser
{
private:
    IPluginInstance *_pluginInstance = nullptr;
    QStringList _transforms;
    QStringList _visualisations;
    std::map<QString, NodeIdSet> _bookmarks;
    std::unique_ptr<ExactNodePositions> _nodePositions;
    QByteArray _uiData;
    QByteArray _pluginUiData;
    int _pluginUiDataVersion = -1;
    bool _layoutPaused = false;

public:
    bool parse(const QUrl& url, IGraphModel& graphModel, const ProgressFn& progressFn) override;
    void setPluginInstance(IPluginInstance* pluginInstance);

    QStringList transforms() const { return _transforms; }
    QStringList visualisations() const { return _visualisations; }
    const auto& bookmarks() const { return _bookmarks; }
    const ExactNodePositions* nodePositions() const;
    const QByteArray& uiData() const { return _uiData; }
    const QByteArray& pluginUiData() const { return _pluginUiData; }
    int pluginUiDataVersion() const { return _pluginUiDataVersion; }
    bool layoutPaused() const { return _layoutPaused; }

    static QString pluginNameFor(const QUrl& url);
    static bool canOpen(const QUrl& url);
};

#endif // LOADER_H
