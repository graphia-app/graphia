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

#include "saverfactory.h"

#include "app/ui/document.h"
#include "app/graph/graphmodel.h"
#include "shared/graph/igraphmodel.h"
#include "shared/attributes/iattribute.h"
#include "shared/utils/container.h"

#include <set>
#include <utility>

IGraphModel* graphModelFor(Document* document)
{
    return document->graphModel();
}

std::map<QString, QString> exportedAttributeNames(const IGraphModel& graphModel)
{
    std::set<QString> structuralAttributeNames;
    for(const auto& declaration : GraphModel::attributeDeclarations())
        structuralAttributeNames.insert(declaration._name);

    std::map<std::pair<ElementType, QString>, std::set<QString>> attributesShownAs;

    for(const auto& attributeName : graphModel.attributeNames())
    {
        if(u::contains(structuralAttributeNames, attributeName))
            continue;

        const auto* attribute = graphModel.attributeByName(attributeName);
        attributesShownAs[{attribute->elementType(), graphModel.attributeDisplayName(attributeName)}].insert(attributeName);
    }

    std::map<QString, QString> names;

    for(const auto& [shownAs, attributeNames] : attributesShownAs)
    {
        const auto& displayName = shownAs.second;

        for(const auto& attributeName : attributeNames)
        {
            const bool clashes = attributeNames.size() > 1 && attributeName != displayName;
            names.emplace(attributeName, clashes ? attributeName : displayName);
        }
    }

    return names;
}
