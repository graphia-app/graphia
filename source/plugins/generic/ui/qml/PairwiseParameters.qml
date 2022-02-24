/* Copyright © 2013-2022 Graphia Technologies Ltd.
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

import QtQuick.Controls 1.5
import QtQuick 2.14
import QtQml 2.12
import QtQuick.Layouts 1.3
import QtQuick.Window 2.2
import app.graphia 1.0

import "../../../../shared/ui/qml/Constants.js" as Constants
import "../../../../shared/ui/qml/Utils.js" as Utils
import "Controls"

BaseParameterDialog
{
    id: root

    title: qsTr("Pairwise Parameters")

    minimumWidth: 640
    minimumHeight: 480

    property bool _graphEstimatePerformed: false

    TabularDataParser
    {
        id: tabularDataParser

        onDataLoaded: { parameters.data = tabularDataParser.data; }
    }

    ColumnLayout
    {
        id: loadingInfo

        anchors.fill: parent
        visible: !tabularDataParser.complete || tabularDataParser.failed

        Item { Layout.fillHeight: true }

        Text
        {
            Layout.fillWidth: true

            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.WrapAnywhere

            text:
            {
                if(tabularDataParser.failed)
                {
                    let failureMessage = qsTr("Failed to Load ") + QmlUtils.baseFileNameForUrl(url);

                    if(tabularDataParser.failureReason.length > 0)
                        failureMessage += qsTr(":\n\n") + tabularDataParser.failureReason;
                    else
                        failureMessage += qsTr(".");

                    return failureMessage;
                }

                return qsTr("Loading ") + QmlUtils.baseFileNameForUrl(url) + qsTr("…");
            }
        }

        RowLayout
        {
            Layout.alignment: Qt.AlignHCenter

            ProgressBar
            {
                visible: !tabularDataParser.failed
                value: tabularDataParser.progress >= 0.0 ? tabularDataParser.progress / 100.0 : 0.0
                indeterminate: tabularDataParser.progress < 0.0
            }

            Button
            {
                text: tabularDataParser.failed ? qsTr("Close") : qsTr("Cancel")
                onClicked:
                {
                    if(!tabularDataParser.failed)
                        tabularDataParser.cancelParse();

                    root.close();
                }
            }
        }

        Item { Layout.fillHeight: true }
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: Constants.margin

        visible: !loadingInfo.visible

        DataTable
        {
            Layout.fillWidth: true
            Layout.fillHeight: true

            model: tabularDataParser.model
        }

        RowLayout
        {
            Layout.alignment: Qt.AlignRight
            Layout.topMargin: Constants.spacing

            Button
            {
                Layout.alignment: Qt.AlignRight
                text: qsTr("OK")
                onClicked:
                {
                    accepted();
                    root.close();
                }
            }

            Button
            {
                Layout.alignment: Qt.AlignRight
                text: qsTr("Cancel")
                onClicked:
                {
                    rejected();
                    root.close();
                }
            }
        }
    }

    onInitialised:
    {
        parameters =
        {
        };
    }

    onVisibleChanged:
    {
        if(visible)
        {
            if(QmlUtils.urlIsValid(root.url) && root.type.length !== 0)
                tabularDataParser.parse(root.url, root.type);
            else
                console.log("ERROR: url or type is empty");
        }
    }
}
