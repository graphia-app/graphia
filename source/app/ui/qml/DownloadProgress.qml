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

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3

import app.graphia 1.0
import app.graphia.Controls 1.0
import app.graphia.Shared.Controls 1.0

RowLayout
{
    id: root

    property int progress: -1
    property url blockedUrl
    property bool waitingForOpen: { return QmlUtils.urlIsValid(root.blockedUrl); }

    NamedIcon
    {
        id: icon

        iconName: "network-server"
        transform: Rotation
        {
            origin.x: icon.width * 0.5
            origin.y: icon.height * 0.5
            angle: 0

            SequentialAnimation on angle
            {
                id: w
                loops: Animation.Infinite
                alwaysRunToEnd: true
                running: root.waitingForOpen

                property int wiggleMagnitude: 30
                property int wiggleDuration: 700
                readonly property int m1: wiggleMagnitude * 1.0
                readonly property int m2: wiggleMagnitude * 0.6
                readonly property int m3: wiggleMagnitude * 0.2
                readonly property int d: wiggleDuration / 16

                NumberAnimation { from:     0; to:  w.m1; easing.type: Easing.OutSine; duration: w.d }
                NumberAnimation { from:  w.m1; to:     0; easing.type: Easing.InSine;  duration: w.d }
                NumberAnimation { from:     0; to: -w.m1; easing.type: Easing.OutSine; duration: w.d }
                NumberAnimation { from: -w.m1; to:     0; easing.type: Easing.InSine;  duration: w.d }
                NumberAnimation { from:     0; to:  w.m2; easing.type: Easing.OutSine; duration: w.d }
                NumberAnimation { from:  w.m2; to:     0; easing.type: Easing.InSine;  duration: w.d }
                NumberAnimation { from:     0; to: -w.m2; easing.type: Easing.OutSine; duration: w.d }
                NumberAnimation { from: -w.m2; to:     0; easing.type: Easing.InSine;  duration: w.d }
                NumberAnimation { from:     0; to:  w.m3; easing.type: Easing.OutSine; duration: w.d }
                NumberAnimation { from:  w.m3; to:     0; easing.type: Easing.InSine;  duration: w.d }
                NumberAnimation { from:     0; to: -w.m3; easing.type: Easing.OutSine; duration: w.d }
                NumberAnimation { from: -w.m3; to:     0; easing.type: Easing.InSine;  duration: w.d }

                PauseAnimation { duration: 500 }
            }
        }
    }

    Button
    {
        visible: root.waitingForOpen
        implicitWidth: downloadProgressBar.implicitWidth

        contentItem: Text
        {
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight

            text:
            {
                let filename = QmlUtils.baseFileNameForUrl(root.blockedUrl);
                return qsTr("Open ") + (filename.length > 0 ? filename : qsTr("Download"));
            }
        }


        onClicked: function(mouse)
        {
            root.openClicked(root.blockedUrl);
            root.blockedUrl = "";
        }
    }

    ProgressBar
    {
        id: downloadProgressBar
        visible: !root.waitingForOpen
        indeterminate: root.progress < 0
        value: !indeterminate ? root.progress / 100.0 : 0.0
    }

    FloatingButton
    {
        implicitHeight: progressBar.implicitHeight
        implicitWidth: implicitHeight

        iconName: "process-stop"
        text: qsTr("Cancel")

        onClicked: function(mouse)
        {
            root.blockedUrl = "";
            root.cancelClicked();
        }
    }

    signal openClicked(url url);
    signal cancelClicked();
}
