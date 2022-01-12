/* Copyright © 2013-2022 Graphia Technologies Ltd.
 *
 * This file is part of Graphia.
 *
 * Graphia is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Graphia is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Graphia.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LOADER_H
#define LOADER_H

#include "shared/loading/iparser.h"
#include "shared/plugins/iplugin.h"

#include "shared/graph/elementid_containers.h"

#include "layout/layout.h"
#include "layout/nodepositions.h"
#include "rendering/projection.h"
#include "rendering/shading.h"
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

    struct EnrichmentTableData
    {
        EnrichmentTableModel::Table _table;
        QString _selectionA;
        QString _selectionB;
    };

    std::vector<EnrichmentTableData> _enrichmentTableData;

    QByteArray _uiData;
    QByteArray _pluginUiData;
    int _pluginUiDataVersion = -1;

    QString _layoutName;
    std::vector<LayoutSettingKeyValue> _layoutSettings;
    std::unique_ptr<ExactNodePositions> _nodePositions;
    bool _layoutPaused = false;

    Projection _projection = Projection::Perspective;
    Shading _shading = Shading::Smooth;

    QString _log;

public:
    bool parse(const QUrl& url, IGraphModel* igraphModel) override;
    void setPluginInstance(IPluginInstance* pluginInstance);

    QStringList transforms() const { return _transforms; }
    QStringList visualisations() const { return _visualisations; }
    const auto& bookmarks() const { return _bookmarks; }
    QString log() const override { return _log; }
    const std::vector<EnrichmentTableData>& enrichmentTableData() const
    { return _enrichmentTableData; }

    const QByteArray& uiData() const { return _uiData; }
    const QByteArray& pluginUiData() const { return _pluginUiData; }
    int pluginUiDataVersion() const { return _pluginUiDataVersion; }

    QString layoutName() const { return _layoutName; }
    auto layoutSettings() const { return _layoutSettings; }
    const ExactNodePositions* nodePositions() const;
    bool layoutPaused() const { return _layoutPaused; }

    Projection projection() const { return _projection; }
    Shading shading() const { return _shading; }

    static QString pluginNameFor(const QUrl& url);
    static bool canOpen(const QUrl& url);
};

#endif // LOADER_H
