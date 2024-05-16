/* Copyright © 2013-2024 Graphia Technologies Ltd.
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

#include <utility>

#include "isaver.h"
#include "shared/utils/progressable.h"

#include "app/graph/graphmodel.h"
#include "app/graph/mutablegraph.h"
#include "app/ui/document.h"

#include <json_helper.h>

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QUrl>

class Document;
class IGraph;
class IPluginInstance;

class NativeSaver : public ISaver
{
private:
    QUrl _fileUrl;
    Document* _document = nullptr;
    const IPluginInstance* _pluginInstance = nullptr;
    QByteArray _uiData;
    QByteArray _pluginUiData;

public:
    static const int Version;
    static const int MaxHeaderSize;

    NativeSaver(const QUrl& fileUrl, Document* document, const IPluginInstance* pluginInstance,
        const QByteArray& uiData, const QByteArray& pluginUiData) :
        _fileUrl(fileUrl),
        _document(document), _pluginInstance(pluginInstance), _uiData(uiData),
        _pluginUiData(pluginUiData)
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
