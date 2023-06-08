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

#ifndef IGRAPHMODEL_H
#define IGRAPHMODEL_H

#include "shared/graph/elementid.h"
#include "shared/graph/elementtype.h"

#include <vector>
#include <QString>

class IGraph;
class IMutableGraph;
class IAttribute;
struct IElementVisual;

template<typename> class IUserElementData;
using IUserNodeData = IUserElementData<NodeId>;
using IUserEdgeData = IUserElementData<EdgeId>;

enum class VisualChangeFlags
{
    None        = 0x00,
    Size        = 0x01,
    Color       = 0x02,
    Text        = 0x04,
    TextColor   = 0x08,
    TextSize    = 0x10,
    State       = 0x20
};

class IGraphModel
{
public:
    virtual ~IGraphModel() = default;

protected:
    virtual IMutableGraph& mutableGraphImpl() = 0;
    virtual const IMutableGraph& mutableGraphImpl() const = 0;
    virtual const IGraph& graphImpl() const = 0;

    virtual const IElementVisual& nodeVisualImpl(NodeId nodeId) const = 0;
    virtual const IElementVisual& edgeVisualImpl(EdgeId edgeId) const = 0;

public:
    IMutableGraph& mutableGraph() { return mutableGraphImpl(); }
    const IMutableGraph& mutableGraph() const { return mutableGraphImpl(); }
    const IGraph& graph() const { return graphImpl(); }

    const IElementVisual& nodeVisual(NodeId nodeId) const { return nodeVisualImpl(nodeId); }
    const IElementVisual& edgeVisual(EdgeId edgeId) const { return edgeVisualImpl(edgeId); }

    virtual QString nodeName(NodeId nodeId) const = 0;
    virtual void setNodeName(NodeId nodeId, const QString& name) = 0;

    virtual IAttribute& createAttribute(QString name, QString* assignedName = nullptr) = 0;
    virtual const IAttribute* attributeByName(const QString& name) const = 0;
    virtual IAttribute* attributeByName(const QString& name) = 0;
    virtual bool attributeExists(const QString& name) const = 0;
    virtual std::vector<QString> attributeNames(ElementType elementType = ElementType::All) const = 0;

    template<typename Fn>
    std::vector<QString> attributeNamesMatching(Fn&& predicate) const
    {
        std::vector<QString> matchingAttributeNames;
        matchingAttributeNames.reserve(attributeNames().size());

        for(const auto& attributeName : attributeNames())
        {
            const auto* attribute = attributeByName(attributeName);
            Q_ASSERT(attribute != nullptr);

            if(predicate(*attribute))
                matchingAttributeNames.push_back(attributeName);
        }

        matchingAttributeNames.shrink_to_fit();
        return matchingAttributeNames;
    }

    virtual IUserNodeData& userNodeData() = 0;
    virtual IUserEdgeData& userEdgeData() = 0;
};

#endif // IGRAPHMODEL_H
