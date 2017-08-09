#ifndef LOADER_H
#define LOADER_H

#include "shared/loading/iparser.h"
#include "shared/plugins/iplugin.h"

#include "layout/nodepositions.h"

#include <QString>
#include <QStringList>
#include <QByteArray>

#include <memory>

class Loader : public IParser
{
private:
    IPluginInstance *_pluginInstance = nullptr;
    QStringList _transforms;
    QStringList _visualisations;
    std::unique_ptr<ExactNodePositions> _nodePositions;
    QByteArray _uiData;
    int _pluginDataVersion = -1;
    bool _layoutPaused = false;

public:
    bool parse(const QUrl& url, IMutableGraph& graph, const ProgressFn& progressFn);
    void setPluginInstance(IPluginInstance* pluginInstance);

    QStringList transforms() const { return _transforms; }
    QStringList visualisations() const { return _visualisations; }
    const ExactNodePositions* nodePositions() const;
    const QByteArray& uiData() const { return _uiData; }
    int pluginDataVersion() const { return _pluginDataVersion; }
    bool layoutPaused() const { return _layoutPaused; }

    static QString pluginNameFor(const QUrl& url);
    static bool canOpen(const QUrl& url);
};

#endif // LOADER_H
