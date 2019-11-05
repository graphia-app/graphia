#ifndef LOADER_H
#define LOADER_H

#include "shared/loading/iparser.h"
#include "shared/plugins/iplugin.h"

#include "shared/graph/elementid_containers.h"

#include "layout/layout.h"
#include "layout/nodepositions.h"
#include "rendering/projection.h"
#include "attributes/enrichmenttablemodel.h"

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

    std::vector<EnrichmentTableModel::Table> _enrichmentTablesData;

    QByteArray _uiData;
    QByteArray _pluginUiData;
    int _pluginUiDataVersion = -1;

    QString _layoutName;
    std::vector<LayoutSettingKeyValue> _layoutSettings;
    std::unique_ptr<ExactNodePositions> _nodePositions;
    bool _layoutPaused = false;

    Projection _projection = Projection::Perspective;

public:
    bool parse(const QUrl& url, IGraphModel* graphModel) override;
    void setPluginInstance(IPluginInstance* pluginInstance);

    QStringList transforms() const { return _transforms; }
    QStringList visualisations() const { return _visualisations; }
    const auto& bookmarks() const { return _bookmarks; }
    const std::vector<EnrichmentTableModel::Table>& enrichmentTableModels() const
    { return _enrichmentTablesData; }

    const QByteArray& uiData() const { return _uiData; }
    const QByteArray& pluginUiData() const { return _pluginUiData; }
    int pluginUiDataVersion() const { return _pluginUiDataVersion; }

    QString layoutName() const { return _layoutName; }
    auto layoutSettings() const { return _layoutSettings; }
    const ExactNodePositions* nodePositions() const;
    bool layoutPaused() const { return _layoutPaused; }

    Projection projection() const { return _projection; }

    static QString pluginNameFor(const QUrl& url);
    static bool canOpen(const QUrl& url);
};

#endif // LOADER_H
