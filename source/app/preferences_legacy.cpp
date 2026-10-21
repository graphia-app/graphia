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

#include "preferences_legacy.h"

#include "app/application.h"
#include "app/legacynames.h"
#include "app/limitconstants.h"
#include "app/preferences.h"

#include "shared/utils/container.h"
#include "shared/utils/json.h"
#include "shared/utils/utils.h"

#include <QString>

using namespace Qt::Literals::StringLiterals;

// Before the default sizes were normalised, they were stored as absolute sizes
static void normaliseDefaultSizes()
{
    if(u::prefExists(u"visuals/defaultNodeSize"_s))
    {
        auto absNodeSize = u::pref(u"visuals/defaultNodeSize"_s).toFloat();
        auto normalNodeSize = u::normalise(LimitConstants::minimumNodeSize(),
                                           LimitConstants::maximumNodeSize(), absNodeSize);

        u::setPref(u"visuals/defaultNormalNodeSize"_s, normalNodeSize);
        u::removePref(u"visuals/defaultNodeSize"_s);
    }

    if(u::prefExists(u"visuals/defaultEdgeSize"_s))
    {
        auto absEdgeSize = u::pref(u"visuals/defaultEdgeSize"_s).toFloat();
        auto normalEdgeSize = u::normalise(LimitConstants::minimumEdgeSize(),
                                           LimitConstants::maximumEdgeSize(), absEdgeSize);

        u::setPref(u"visuals/defaultNormalEdgeSize"_s, normalEdgeSize);
        u::removePref(u"visuals/defaultEdgeSize"_s);
    }
}

static void renameInJsonPreference(const QString& key, const std::function<void(json&)>& renameFn)
{
    if(!u::prefExists(key))
        return;

    auto jsonValue = parseJsonFrom(u::pref(key).toString().toUtf8());
    if(!jsonValue.is_array())
        return;

    const auto original = jsonValue;
    renameFn(jsonValue);

    if(jsonValue != original)
        u::setPref(key, QString::fromStdString(jsonValue.dump()));
}

void upgradeLegacyPreferences(const Application* application)
{
    normaliseDefaultSizes();

    LegacyNames legacyNames(
        [application](const QString& name) { return application->transformFactory(name); },
        [application](const QString& name) { return application->attributeIsDeclared(name); });

    renameInJsonPreference(u"misc/favouriteTransforms"_s, [&legacyNames](json& favourites)
    {
        renameEachIn(favourites, [&legacyNames](const QString& name) { return legacyNames.transformName(name); });
    });

    renameInJsonPreference(u"misc/templates"_s, [&legacyNames](json& templates)
    {
        for(auto& jsonTemplate : templates)
        {
            if(!jsonTemplate.is_object())
                continue;

            if(u::contains(jsonTemplate, "transforms"))
                renameEachIn(jsonTemplate["transforms"], [&legacyNames](const QString& transform) { return legacyNames.transform(transform); });

            if(u::contains(jsonTemplate, "visualisations"))
                renameEachIn(jsonTemplate["visualisations"], [&legacyNames](const QString& visualisation) { return legacyNames.visualisation(visualisation); });
        }
    });
}
