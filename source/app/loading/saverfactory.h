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

#ifndef SAVERFACTORY_H
#define SAVERFACTORY_H

#include "isaver.h"

#include <QString>

class Document;
class IGraphModel;

// The entire point of this function is to avoid including document.h
// Sometimes C++ really sucks
IGraphModel* graphModelFor(Document* document);

template<class SaverType>
class SaverFactory : public ISaverFactory
{
public:
    std::unique_ptr<ISaver> create(const QUrl& url, Document* document, const IPluginInstance*,
                                   const QByteArray&, const QByteArray&) override
    {
        return std::make_unique<SaverType>(url, graphModelFor(document));
    }
    QString name() const override { return SaverType::name(); }
    QString extension() const override { return SaverType::extension(); }
};

#endif // SAVERFACTORY_H
