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
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import app.graphia.Shared

BaseParameterDialog
{
    id: root
    default property list<Item> listPages
    property int currentIndex: 0
    property int enableFinishAtIndex: 0
    property bool nextEnabled: true
    property bool finishEnabled: true
    property alias animating: numberAnimation.running
    property Item currentItem:
    {
        return listPages[currentIndex];
    }

    //FIXME Set these based on the content pages
    //minimumWidth: 640
    //minimumHeight: 480

    onWidthChanged: { content.x = currentIndex * -root.width }
    onHeightChanged: { content.x = currentIndex * -root.width }

    onCurrentIndexChanged: pageIndicator.requestPaint();

    ColumnLayout
    {
        anchors.margins: Constants.margin
        anchors.fill: parent
        Item
        {
            id: content
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        RowLayout
        {
            Canvas
            {
                Layout.fillWidth: true
                id: pageIndicator
                property int _padding: 5
                property int pipSize: 5
                property int spacing: 20
                width: (listPages.length * spacing) + (_padding * 2)
                height: pipSize + (_padding  * 2)

                onPaint: function(rect)
                {
                    let ctx = getContext("2d");
                    let currentPipSize = pipSize + 2;
                    let topPip = ((height - pipSize) * 0.5);

                    ctx.save();
                    ctx.clearRect(0, 0, width, height);
                    ctx.strokeStyle = palette.dark;
                    ctx.fillStyle = palette.dark;

                    ctx.lineJoin = "round";
                    ctx.lineWidth = 1;

                    ctx.strokeStyle = palette.dark;

                    // Draw lines between pips
                    //let pipNum = 0;
                    for(let pipNum = 1; pipNum < listPages.length; pipNum++)
                    {
                        ctx.beginPath();
                        ctx.moveTo(_padding + ((pipNum - 1) * spacing) + pipSize, topPip + (pipSize * 0.5));
                        ctx.lineTo(_padding + (pipNum * spacing), topPip + (pipSize * 0.5));
                        ctx.stroke();
                    }

                    // Draw pips
                    for(let pipNum = 0; pipNum < listPages.length; pipNum++)
                    {
                        ctx.strokeStyle = palette.dark;
                        ctx.fillStyle = palette.dark;

                        let left = _padding + (pipNum * spacing);
                        let top = topPip;

                        // Current Pip
                        if(pipNum == currentIndex)
                        {
                            ctx.strokeStyle = palette.highlight;
                            ctx.fillStyle = palette.highlight;

                            // Current pips are slightly larger so we need to
                            // reposition them
                            left = left - (currentPipSize - pipSize) / 2;
                            top = topPip - (currentPipSize - pipSize) / 2;

                            context.strokeRect(left, top, currentPipSize, currentPipSize);
                            context.fillRect(left, top, currentPipSize, currentPipSize);
                        }
                        else
                        {
                            context.strokeRect(left, top, pipSize, pipSize);

                            // Fill previous pips
                            if(pipNum < currentIndex)
                                context.fillRect(left, top, pipSize, pipSize);
                        }
                    }

                    ctx.restore();
                }
            }

            Button
            {
                id: previousButton
                text: qsTr("Previous")
                onClicked: function(mouse) { root.previous(); }
                enabled: currentIndex > 0
            }

            Button
            {
                id: nextButton
                text: qsTr("Next")
                onClicked: function(mouse) { root.next(); }
                enabled: (currentIndex < listPages.length - 1) ? nextEnabled : false
            }

            Button
            {
                id: finishButton
                text: qsTr("Finish")
                onClicked: function(mouse) { root.accepted(); }
                enabled: (currentIndex >= enableFinishAtIndex) ? finishEnabled : false
            }

            Button
            {
                text: qsTr("Cancel")
                onClicked: function(mouse) { root.close(); }
            }
        }
    }

    function next()
    {
        if(currentIndex < listPages.length - 1)
        {
            currentIndex++;
            numberAnimation.running = false;
            numberAnimation.running = true;
        }
    }

    function previous()
    {
        if(currentIndex > 0)
        {
            currentIndex--;
            numberAnimation.running = false;
            numberAnimation.running = true;
        }
    }

    function goToPage(index)
    {
        if(index <= listPages.length - 1 && index >= 0)
        {
            currentIndex = index;
            numberAnimation.running = false;
            numberAnimation.running = true;
        }
    }

    NumberAnimation
    {
        id: numberAnimation
        target: content
        properties: "x"
        to: currentIndex * -root.width
        duration: 200
        easing.type: Easing.OutQuad
    }

    Component.onCompleted:
    {
        for(let i = 0; i < listPages.length; i++)
        {
            listPages[i].parent = content;
            listPages[i].x = Qt.binding(() => i * root.width);
            listPages[i].width = Qt.binding(() => root.width - (Constants.margin * 2));
            listPages[i].height = Qt.binding(() => content.height - (Constants.margin * 2));
        }
    }

    onAccepted:
    {
        close();
    }
}
