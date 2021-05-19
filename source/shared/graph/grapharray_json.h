/* Copyright © 2013-2021 Graphia Technologies Ltd.
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

#ifndef GRAPHARRAY_JSON_H
#define GRAPHARRAY_JSON_H

#include <json_helper.h>

#include "shared/graph/grapharray.h"
#include "shared/utils/progressable.h"
#include "shared/utils/container.h"

namespace u
{
    template<typename GraphArray, typename C, typename ElementToJsonFn>
    json graphArrayAsJson(const GraphArray& graphArray, const C& elementIds,
        Progressable* progressable, const ElementToJsonFn& elementToJson)
    {
        json array = json::array();

        uint64_t i = 0;
        for(auto elementId : elementIds)
        {
            json object;

            object["id"] = elementId;

            const auto& value = graphArray.at(elementId);
            object["value"] = elementToJson(value);

            array.push_back(object);

            if(progressable != nullptr)
                progressable->setProgress(static_cast<int>((i++) * 100 / elementIds.size()));
        }

        if(progressable != nullptr)
            progressable->setProgress(-1);

        return array;
    }

    template<typename GraphArray, typename C>
    json graphArrayAsJson(const GraphArray& graphArray, const C& elementIds,
        Progressable* progressable = nullptr)
    {
        return graphArrayAsJson(graphArray, elementIds, progressable,
            [](const auto& element) { return element; });
    }

    template<typename Fn>
    void forEachJsonGraphArray(const json& jsonArray, const Fn& fn)
    {
        if(jsonArray.is_null() || !jsonArray.is_array())
        {
            qWarning() << "forEachJsonGraphArray: json is null or not an array";
            return;
        }

        for(const auto& element : jsonArray)
        {
            if(!element.is_object())
            {
                qWarning() << "forEachJsonGraphArray: element is not a json object";
                return;
            }

            if(!u::contains(element, "id") || !element["id"].is_number() ||
                !u::contains(element, "value"))
            {
                qWarning() << "forEachJsonGraphArray: element does not have id or value";
                return;
            }
        }

        for(const auto& element : jsonArray)
        {
            int elementId = element["id"];
            fn(elementId, element["value"]);
        }
    }

} // namespace u

#endif // GRAPHARRAY_JSON_H
