/* Copyright © 2013-2020 Graphia Technologies Ltd.
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

#ifndef JSONGRAPHPARSER_H
#define JSONGRAPHPARSER_H

#include "shared/loading/iparser.h"
#include "shared/loading/iuserelementdata.h"
#include "json_helper.h"

class JsonGraphParser : public IParser
{
private:
    IUserNodeData* _userNodeData;
    IUserEdgeData* _userEdgeData;
public:
    JsonGraphParser(IUserNodeData* userNodeData, IUserEdgeData* userEdgeData) :
        _userNodeData(userNodeData), _userEdgeData(userEdgeData)
    {}

    bool parse(const QUrl &url, IGraphModel *graphModel) override;
    static bool canLoad(const QUrl &) { return true; }
    static bool parseGraphObject(const json& jsonGraphObject, IGraphModel *graphModel,
        IParser& parser, bool useElementIdsLiterally = false,
        IUserNodeData* userNodeData = nullptr, IUserEdgeData* userEdgeData = nullptr);
};

#endif // JSONGRAPHPARSER_H
