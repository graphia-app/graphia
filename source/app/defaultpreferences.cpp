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

#include "defaultpreferences.h"

#include "app/preferences.h"
#include "app/rendering/graphrenderertypes.h"

#include "shared/ui/visualisations/defaultgradients.h"
#include "shared/ui/visualisations/defaultpalettes.h"

#include <QFont>
#include <QGuiApplication>
#include <QStandardPaths>
#include <QString>
#include <QUrl>
#include <QVariant>

using namespace Qt::Literals::StringLiterals;

void initialisePreferences()
{
    // A GUI-less application has no font
    const auto* guiApplication = qobject_cast<QGuiApplication*>(QCoreApplication::instance());
    const QString textFontFamily = guiApplication != nullptr ?
        QGuiApplication::font().family() : u"Sans"_s;

    u::definePref(u"visuals/defaultNodeColor"_s,                "#0000FF");
    u::definePref(u"visuals/defaultEdgeColor"_s,                "#FFFFFF");
    u::definePref(u"visuals/multiElementColor"_s,               "#FF0000");
    u::definePref(u"visuals/backgroundColor"_s,                 "#DDDDDD");
    u::definePref(u"visuals/highlightColor"_s,                  "#FFFFFF");

    u::definePref(u"visuals/defaultNormalNodeSize"_s,           0.333);
    u::definePref(u"visuals/defaultNormalEdgeSize"_s,           0.25);

    u::definePref(u"visuals/showNodeText"_s,                    QVariant::fromValue(static_cast<int>(TextState::Selected)));
    u::definePref(u"visuals/showEdgeText"_s,                    QVariant::fromValue(static_cast<int>(TextState::Selected)));
    u::definePref(u"visuals/textFont"_s,                        textFontFamily);
    u::definePref(u"visuals/textSize"_s,                        24.0f);
    u::definePref(u"visuals/edgeVisualType"_s,                  QVariant::fromValue(static_cast<int>(EdgeVisualType::Cylinder)));
    u::definePref(u"visuals/textAlignment"_s,                   QVariant::fromValue(static_cast<int>(TextAlignment::Right)));
    u::definePref(u"visuals/showMultiElementIndicators"_s,      true);
    u::definePref(u"visuals/showEdges"_s,                       true);
    u::definePref(u"visuals/savedGradients"_s,                  Defaults::GRADIENT_PRESETS);
    u::definePref(u"visuals/defaultGradient"_s,                 Defaults::GRADIENT);
    u::definePref(u"visuals/savedPalettes"_s,                   Defaults::PALETTE_PRESETS);
    u::definePref(u"visuals/defaultPalette"_s,                  Defaults::PALETTE);

    u::definePref(u"visuals/projection"_s,                      QVariant::fromValue(static_cast<int>(Projection::Perspective)));

    u::definePref(u"visuals/minimumComponentRadius"_s,          2.0);
    u::definePref(u"visuals/transitionTime"_s,                  1.0);

    u::definePref(u"visuals/disableMultisampling"_s,            false);

    u::definePref(u"misc/maxUndoLevels"_s,                      25);

    u::definePref(u"misc/showGraphMetrics"_s,                   false);
    u::definePref(u"misc/showLayoutSettings"_s,                 false);

    u::definePref(u"misc/focusFoundNodes"_s,                    true);
    u::definePref(u"misc/focusFoundComponents"_s,               true);
    u::definePref(u"misc/stayInComponentMode"_s,                false);

    u::definePref(u"misc/disableHubbles"_s,                     false);

    u::definePref(u"misc/hasSeenTutorial"_s,                    false);

    u::definePref(u"misc/autoBackgroundUpdateCheck"_s,          true);

    u::definePref(u"find/findByAttributeSortLexically"_s,       true);

    u::definePref(u"screenshot/width"_s,                        1920);
    u::definePref(u"screenshot/height"_s,                       1080);
    u::definePref(u"screenshot/path"_s,
        QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString());

    u::definePref(u"servers/redirects"_s,                       "https://redirects.graphia.app");
    u::definePref(u"servers/updates"_s,                         "https://updates.graphia.app");
    u::definePref(u"servers/crashreports"_s,                    "https://crashreports.graphia.app");
    u::definePref(u"servers/tracking"_s,                        "https://tracking.graphia.app");

    u::definePref(u"proxy/type"_s,                              "disabled");

#ifdef Q_OS_WASM
    u::definePref(u"system/uiTheme"_s,                          "Fusion");
#else
    u::definePref(u"system/uiTheme"_s,                          "Default");
#endif
}
