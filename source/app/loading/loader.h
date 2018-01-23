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
    QByteArray _pluginUIData;
    int _pluginUIDataVersion = -1;
    bool _layoutPaused = false;

public:
    bool parse(const QUrl& url, IGraphModel& graphModel, const ProgressFn& progressFn) override;
    void setPluginInstance(IPluginInstance* pluginInstance);

    QStringList transforms() const { return _transforms; }
    QStringList visualisations() const { return _visualisations; }
    const ExactNodePositions* nodePositions() const;
    const QByteArray& pluginUIData() const { return _pluginUIData; }
    int pluginUIDataVersion() const { return _pluginUIDataVersion; }
    bool layoutPaused() const { return _layoutPaused; }

    static QString pluginNameFor(const QUrl& url);
    static bool canOpen(const QUrl& url);
};

#endif // LOADER_H
