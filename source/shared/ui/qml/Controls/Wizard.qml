import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import "../Constants.js" as Constants

BaseParameterDialog
{
    id: view
    default property list<Item> listPages
    property int currentIndex: 0
    property int enableFinishAtIndex: 0
    property bool nextEnabled: true
    property bool finishEnabled: true

    modality: Qt.ApplicationModal

    minimumWidth: 450
    minimumHeight: 200

    onWidthChanged: { content.x = currentIndex * -view.width }
    onHeightChanged: { content.x = currentIndex * -view.width }

    onCurrentIndexChanged: pageIndicator.requestPaint()

    SystemPalette { id: systemPalette; }

    ColumnLayout
    {
        anchors.margins: Constants.margin
        anchors.fill: parent
        Rectangle
        {
            id: content
            Layout.fillWidth: true;
            Layout.fillHeight: true;
        }

        RowLayout
        {
            Canvas
            {
                Layout.fillWidth: true
                id: pageIndicator
                property int _padding: 5;
                property int pipSize: 5;
                property int spacing: 20;
                width: (listPages.length * spacing) + _padding * 2;
                height: (pipSize) + _padding  * 2
                onPaint:
                {
                    var ctx = getContext("2d");
                    var currentPipSize = pipSize + 2;
                    var topPip = ((height - pipSize) / 2);

                    ctx.save();
                    ctx.clearRect(0, 0, width, height);
                    ctx.strokeStyle = systemPalette.dark
                    ctx.fillStyle = systemPalette.dark

                    ctx.lineJoin = "round";
                    ctx.lineWidth = 1;

                    ctx.strokeStyle = systemPalette.dark

                    // Draw lines between pips
                    for(var pipNum=1; pipNum<listPages.length; pipNum++)
                    {
                        ctx.beginPath();
                        ctx.moveTo(_padding + (pipNum - 1) * spacing + pipSize, topPip + pipSize / 2);
                        ctx.lineTo(_padding + (pipNum) * spacing, topPip + pipSize / 2);
                        ctx.stroke();
                    }

                    // Draw pips
                    for(var pipNum=0; pipNum<listPages.length; pipNum++)
                    {
                        ctx.strokeStyle = systemPalette.dark
                        ctx.fillStyle = systemPalette.dark

                        var left = _padding + (pipNum) * spacing;
                        var top = topPip;

                        // Current Pip
                        if(pipNum == currentIndex)
                        {
                            ctx.strokeStyle = systemPalette.highlight;
                            ctx.fillStyle = systemPalette.highlight;

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
                onClicked: previous();
                enabled: currentIndex > 0
            }
            Button
            {
                id: nextButton
                text: qsTr("Next")
                onClicked: next();
                enabled: (currentIndex < listPages.length - 1) ? nextEnabled : false
            }
            Button
            {
                id: finishButton
                text: qsTr("Finish")
                onClicked: accepted()
                enabled: (currentIndex >= enableFinishAtIndex) ? finishEnabled : false
            }
            Button
            {
                text: qsTr("Cancel")
                onClicked: close();
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

    function indexOf(item)
    {
        for(var i=0; i < listPages.length; i++)
            if(listPages[i] === item)
                return i;
    }

    NumberAnimation
    {
        id: numberAnimation
        target: content
        properties: "x"
        to: currentIndex * -view.width
        duration: 200;
        easing.type: Easing.OutQuad
    }

    Component.onCompleted:
    {
        for(var i=0; i < listPages.length; i++)
        {
            listPages[i].parent = content;
            listPages[i].x = Qt.binding(function() { return indexOf(this) * view.width });
            listPages[i].width = Qt.binding(function() { return view.width - (Constants.margin * 2) });
            listPages[i].height = Qt.binding(function() { return content.height - (Constants.margin * 2) });
        }
    }
    onAccepted: close();
}
