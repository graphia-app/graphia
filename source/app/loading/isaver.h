/* Copyright © 2013-2023 Graphia Technologies Ltd.
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

#ifndef ISaver_H
#define ISaver_H

#include "shared/utils/cancellable.h"
#include "shared/utils/progressable.h"

#include <QByteArray>

#include <memory>

class QUrl;
class QString;
class IGraphModel;
class Document;
class IPluginInstance;

class ISaver : virtual public Progressable, virtual public Cancellable
{
public:
    ~ISaver() override = default;

    virtual bool save() = 0;
};

class ISaverFactory
{
public:
    virtual ~ISaverFactory() = default;

    virtual std::unique_ptr<ISaver> create(const QUrl& url, Document* document,
                                           const IPluginInstance* pluginInstance, const QByteArray& uiData,
                                           const QByteArray& pluginUiData) = 0;
    virtual QString name() const = 0;
    virtual QString extension() const = 0;
};

#endif // ISaver_H
