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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtWebEngine

import app.graphia.Shared

PluginContent
{
    id: root

    anchors.fill: parent
    visible: true

    toolStrip: RowLayout
    {
        anchors.fill: parent

        ComboBox
        {
            id: urlTemplate
            editable: true
            model:
            [
                "https://en.wikipedia.org/wiki/%1",
                "https://www.google.co.uk/#q=%1",
                "https://www.google.co.uk/maps/search/%1",
                "https://twitter.com/%1",
                "https://www.reddit.com/search?q=%1",
                "https://www.ncbi.nlm.nih.gov/gene/?term=%1",
                "https://www.youtube.com/results?search_query=%1",
                "https://uk.finance.yahoo.com/lookup;?s=%1",
                "https://www.amazon.co.uk/s/ref=nb_sb_noss_2?field-keywords=%1",
                "https://www.ebay.co.uk/sch/i.html?_nkw=%1"
            ]
            Layout.fillWidth: true

            onEditTextChanged: { root.saveRequired = true; }
            onCurrentIndexChanged: { root.saveRequired = true; }
        }
    }

    ColumnLayout
    {
        anchors.fill: parent

        ProgressBar
        {
            Layout.preferredHeight: 3
            Layout.fillWidth: true
            background: Item {}
            z: -2
            from: 0
            to: 100
            value: webEngineView.loadProgress < 100 ? webEngineView.loadProgress : 0
        }

        WebEngineView
        {
            id: webEngineView
            Layout.fillWidth: true
            Layout.fillHeight: true

            settings.focusOnNavigationEnabled: false

            url:
            {
                return urlTemplate.editText.indexOf("%1") >= 0 ?
                    urlTemplate.editText.arg(plugin.model.selectedNodeNames) : "";
            }
        }
    }

    property bool saveRequired: false

    function save()
    {
        let data =
        {
            "urlIndex": urlTemplate.currentIndex
        };

        if(urlTemplate.currentIndex < 0)
            data["customUrl"] = urlTemplate.editText;

        return data;
    }

    function load(data, version)
    {
        if(data.urlIndex !== undefined)
            urlTemplate.currentIndex = data.urlIndex;

        if(urlTemplate.currentIndex < 0 && data.customUrl !== undefined)
            urlTemplate.editText = data.customUrl;
    }
}
