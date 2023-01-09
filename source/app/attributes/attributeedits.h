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

#ifndef ATTRIBUTEEDITS_H
#define ATTRIBUTEEDITS_H

#include "shared/graph/elementid.h"
#include "shared/graph/elementid_containers.h"

#include "shared/utils/static_block.h"

#include <QMetaType>
#include <QString>

#include <map>

class AttributeEdits
{
    Q_GADGET

    friend class EditAttributeTableModel;
    friend class EditAttributeCommand;

private:
    NodeIdMap<QString> _nodeValues;
    EdgeIdMap<QString> _edgeValues;

public:
    auto nodeValues() const { return _nodeValues; }
    auto edgeValues() const { return _edgeValues; }
};

static_block
{
    qRegisterMetaType<AttributeEdits>("AttributeEdits");
}

#endif // ATTRIBUTEEDITS_H
