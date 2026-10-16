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

#include "nativeloader_legacy.h"

#include "shared/utils/container.h"
#include "shared/utils/json.h"
#include "shared/utils/string.h"

#include <array>
#include <map>
#include <set>
#include <string>

// Converts an array indexed by NodeId into the current [{id, value}] form,
// keeping only the nodes that are in the graph
static json graphArrayFrom(const json& jsonArray, const std::set<int>& nodeIds)
{
    json jsonGraphArray = json::array();

    int nodeId = 0;
    for(const auto& value : jsonArray)
    {
        if(nodeIds.contains(nodeId))
            jsonGraphArray.push_back({{"id", nodeId}, {"value", value}});

        nodeId++;
    }

    return jsonGraphArray;
}

// Version 3 and earlier call the ids indexes
static const char* idsKeyFor(const json& jsonUserData)
{
    return u::contains(jsonUserData, "ids") ? "ids" : "indexes";
}

static bool isUserData(const json& jsonUserData)
{
    return jsonUserData.is_object() &&
        u::contains(jsonUserData, idsKeyFor(jsonUserData)) && jsonUserData[idsKeyFor(jsonUserData)].is_array() &&
        u::contains(jsonUserData, "vectors") && jsonUserData["vectors"].is_array();
}

// Merges one set of user data into another, matching their values by element id
static void mergeUserData(const json& jsonFrom, json& jsonTo)
{
    const std::array<const json*, 2> userDatas{&jsonFrom, &jsonTo};

    std::map<int, size_t> indexes;
    json jsonIds = json::array();

    for(const auto* jsonUserData : userDatas)
    {
        for(const auto& jsonId : (*jsonUserData)[idsKeyFor(*jsonUserData)])
        {
            if(indexes.emplace(jsonId.get<int>(), indexes.size()).second)
                jsonIds.push_back(jsonId);
        }
    }

    json jsonVectors = json::array();
    json jsonAttributes = json::object();

    for(const auto* jsonUserData : userDatas)
    {
        const auto& jsonUserDataIds = (*jsonUserData)[idsKeyFor(*jsonUserData)];

        for(auto jsonVector : (*jsonUserData)["vectors"])
        {
            if(!jsonVector.is_object() || jsonVector.empty())
                continue;

            auto& jsonValues = jsonVector.begin().value()["values"];
            json jsonMergedValues(std::vector<std::string>(indexes.size()));

            for(size_t i = 0; i < jsonUserDataIds.size() && i < jsonValues.size(); i++)
                jsonMergedValues[indexes.at(jsonUserDataIds[i].get<int>())] = jsonValues[i];

            jsonValues = jsonMergedValues;
            jsonVectors.push_back(jsonVector);
        }

        if(u::contains(*jsonUserData, "attributes"))
            jsonAttributes.update((*jsonUserData)["attributes"]);
    }

    jsonTo = {{"ids", jsonIds}, {"vectors", jsonVectors}, {"attributes", jsonAttributes}};
}

void upgradeLegacyFormat(json& jsonBody, int version)
{
    if(version < 2 && u::contains(jsonBody, "ui"))
    {
        // There is no UI data of our own, and the plugin's is stored under "ui"
        jsonBody["pluginUiData"] = jsonBody["ui"];
        jsonBody.erase("ui");
    }

    if(version < 4 && u::contains(jsonBody, "graph") && u::contains(jsonBody["graph"], "nodes"))
    {
        // Node names and positions are plain arrays indexed by NodeId
        std::set<int> nodeIds;
        for(const auto& jsonNode : jsonBody["graph"]["nodes"])
        {
            if(u::contains(jsonNode, "id") && jsonNode["id"].is_string() &&
                u::isNumeric(jsonNode["id"].get<std::string>()))
            {
                nodeIds.insert(std::stoi(jsonNode["id"].get<std::string>()));
            }
        }

        if(u::contains(jsonBody, "nodeNames"))
            jsonBody["nodeNames"] = graphArrayFrom(jsonBody["nodeNames"], nodeIds);

        if(u::contains(jsonBody, "layout") && u::contains(jsonBody["layout"], "positions"))
            jsonBody["layout"]["positions"] = graphArrayFrom(jsonBody["layout"]["positions"], nodeIds);
    }

    if(version < 6 && u::contains(jsonBody, "pluginData") && jsonBody["pluginData"].is_object())
    {
        // The user data is stored in the plugin section, and sometimes partly in the body too
        auto& jsonPluginData = jsonBody["pluginData"];

        for(const auto* key : {"userNodeData", "userEdgeData"})
        {
            if(!u::contains(jsonPluginData, key))
                continue;

            const bool bodyHasUserData = u::contains(jsonBody, key) && isUserData(jsonBody[key]);

            if(bodyHasUserData && isUserData(jsonPluginData[key]))
                mergeUserData(jsonPluginData[key], jsonBody[key]);
            else if(!bodyHasUserData)
                jsonBody[key] = jsonPluginData[key];

            jsonPluginData.erase(key);
        }
    }
}
