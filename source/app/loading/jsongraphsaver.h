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

#ifndef JSONGRAPHEXPORTER_H
#define JSONGRAPHEXPORTER_H

#include "saverfactory.h"

#include <QString>

#include <json_helper.h>

using namespace Qt::Literals::StringLiterals;

class IGraph;

class JSONGraphSaver : public ISaver
{
private:
    QUrl _url;
    IGraphModel* _graphModel;
public:
    static QString name() { return u"JSON Graph"_s; }
    static QString extension() { return u"json"_s; }

    static json graphAsJson(const IGraph& graph, Progressable& progressable);

    JSONGraphSaver(const QUrl& url, IGraphModel* graphModel) : _url(url), _graphModel(graphModel) {}
    bool save() override;

};

using JSONGraphSaverFactory = SaverFactory<JSONGraphSaver>;

#endif // JSONGRAPHEXPORTER_H
