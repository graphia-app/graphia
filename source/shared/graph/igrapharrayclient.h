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

#ifndef IGRAPHARRAYCLIENT_H
#define IGRAPHARRAYCLIENT_H

#include "shared/graph/elementid.h"

class IGraphArray;

// Graph classes must implement this interface for a GraphArray
// to be able to associate itself with said graph
class IGraphArrayClient
{
public:
    virtual ~IGraphArrayClient() = default;

    virtual NodeId nextNodeId() const = 0;
    virtual EdgeId nextEdgeId() const = 0;

    virtual NodeId lastNodeIdInUse() const = 0;
    virtual EdgeId lastEdgeIdInUse() const = 0;

    virtual void insertNodeArray(IGraphArray* nodeArray) const = 0;
    virtual void eraseNodeArray(IGraphArray* nodeArray) const = 0;

    virtual void insertEdgeArray(IGraphArray* edgeArray) const = 0;
    virtual void eraseEdgeArray(IGraphArray* edgeArray) const = 0;

    virtual size_t numComponentArrays() const = 0;
    virtual void insertComponentArray(IGraphArray* componentArray) const = 0;
    virtual void eraseComponentArray(IGraphArray* componentArray) const = 0;

    virtual bool isComponentManaged() const = 0;
};

#endif // IGRAPHARRAYCLIENT_H
