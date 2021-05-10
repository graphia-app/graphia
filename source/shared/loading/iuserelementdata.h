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

#ifndef IUSERELEMENTDATA_H
#define IUSERELEMENTDATA_H

#include "shared/loading/iuserdata.h"

#include "shared/graph/elementid.h"

#include <QString>
#include <QVariant>

#include <vector>

template<typename E>
class IUserElementData : public virtual IUserData
{
public:
    virtual ~IUserElementData() = default;

    virtual QString exposedAttributeName(const QString& vectorName) const = 0;
    virtual std::vector<QString> exposedAttributeNames() const = 0;

    virtual E elementIdForIndex(size_t index) const = 0;
    virtual void setElementIdForIndex(E elementId, size_t index) = 0;

    virtual size_t indexFor(E elementId) const = 0;

    virtual QVariant valueBy(E elementId, const QString& name) const = 0;
    virtual bool setValueBy(E elementId, const QString& name, const QString& value) = 0;
};

using IUserNodeData = IUserElementData<NodeId>;
using IUserEdgeData = IUserElementData<EdgeId>;

#endif // IUSERELEMENTDATA_H
