/* Copyright © 2013-2025 Tim Angus
 * Copyright © 2013-2025 Tom Freeman
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

#ifndef SAVER_H
#define SAVER_H

#include "isaver.h"

#include "app/application.h"
#include "app/layout/layoutsettings.h"

#include "shared/graph/elementid_containers.h"

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QUrl>

#include <map>
#include <memory>
#include <vector>

class Document;
class EnrichmentTableModel;
class GraphModel;
class IGraph;
class IPluginInstance;

class NativeSaver : public ISaver
{
public:
    // The parts of a document that are saved, other than those held by its graph model and plugin
    struct Content
    {
        QString _layoutName;
        std::vector<LayoutSettingKeyValue> _layoutSettings;
        bool _layoutPaused = false;

        int _projection = 0;
        int _shading2D = 0;
        int _shading3D = 0;

        QStringList _transforms;
        QStringList _visualisations;
        std::map<QString, NodeIdSet> _bookmarks;
        QString _log;

        std::vector<const EnrichmentTableModel*> _enrichmentTableModels;
    };

private:
    QUrl _fileUrl;
    GraphModel* _graphModel = nullptr;
    Content _content;
    const IPluginInstance* _pluginInstance = nullptr;
    QByteArray _uiData;
    QByteArray _pluginUiData;

public:
    static const int Version;
    static const int MaxHeaderSize;

    NativeSaver(const QUrl& fileUrl, GraphModel* graphModel, Content content,
        const IPluginInstance* pluginInstance, const QByteArray& uiData, const QByteArray& pluginUiData) :
        _fileUrl(fileUrl), _graphModel(graphModel), _content(std::move(content)),
        _pluginInstance(pluginInstance), _uiData(uiData), _pluginUiData(pluginUiData)
    {}

    bool save() override;
};

class NativeSaverFactory : public ISaverFactory
{
public:
    QString name() const override { return Application::name(); }
    QString extension() const override { return Application::nativeExtension(); }
    std::unique_ptr<ISaver> create(const QUrl& url, Document* document, const IPluginInstance* pluginInstance,
                                   const QByteArray& uiData, const QByteArray& pluginUiData) override;
};

#endif // SAVER_H
