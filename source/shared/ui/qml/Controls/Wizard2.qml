import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import "../../shared/ui/qml/Constants.js" as Constants

BaseParameterDialog
{
    id: root
    default property list<Item> listPages
    property int currentIndex: 0
    property int enableFinishAtIndex: 0
    property bool nextEnabled: true
    property bool finishEnabled: true
    property bool summaryEnabled: true
    property alias animating: numberAnimation.running
    readonly property int _padding: 5
    property Item currentItem:
    {
        return listPages[currentIndex];
    }

    modality: Qt.ApplicationModal

    //FIXME Set these based on the content pages
    //minimumWidth: 640
    //minimumHeight: 480

    onWidthChanged: { content.x = currentIndex * -root.width }
    onHeightChanged: { content.x = currentIndex * -root.width }

    SystemPalette { id: systemPalette }

    ColumnLayout
    {
        id: containerLayout
        Layout.fillWidth: true
        Layout.fillHeight: true
        anchors.fill: parent
        anchors.margins: Constants.margin
        RowLayout
        {
            anchors.margins: Constants.margin
            Layout.fillHeight: true

            Rectangle
            {
                Layout.fillHeight: true
                width: 100
                border.width: 1
                border.color: systemPalette.mid
                z: -1

                ListView
                {
                    id: pageSelector
                    model: listPages.length
                    z: 100
                    boundsBehavior: Flickable.StopAtBounds
                    anchors.centerIn: parent
                    anchors.margins: 1
                    anchors.fill: parent

                    delegate: Item
                    {
                        width: parent.width
                        height: delegateRectangle.height
                        Rectangle
                        {
                            id: delegateRectangle
                            color: index == currentIndex ? "lightblue" : "#00000000"
                            anchors.centerIn: parent
                            width: parent.width
                            height: children[0].height + (_padding * 2.0)
                            Text
                            {
                                anchors.margins: _padding
                                anchors.centerIn: parent
                                text: listPages[index].title;
                                z: 5
                            }
                            MouseArea
                            {
                                anchors.fill: parent
                                onClicked: goToPage(index);
                            }
                        }
                    }
                }
            }

            Item
            {
                id: contentContainer
                Layout.fillWidth: true
                Layout.fillHeight: true
                clip: true
                Item
                {
                    id: content
                    height: parent.height
                }
            }
        }

        RowLayout
        {
            Rectangle
            {
                Layout.fillWidth: true
            }

            Button
            {
                id: previousButton
                text: qsTr("Previous")
                onClicked: { root.previous(); }
                enabled: currentIndex > 0
            }

            Button
            {
                id: nextButton
                text: qsTr("Next")
                onClicked: { root.next(); }
                enabled: (currentIndex < listPages.length - 1) ? nextEnabled : false
            }

            Button
            {
                id: finishButton
                text: qsTr("Finish")
                onClicked: {
                    if(summaryEnabled && currentIndex !== listPages.length - 1)
                        goToPage(listPages.length - 1);
                    else
                        root.accepted();
                }
                enabled: (currentIndex >= enableFinishAtIndex) ? finishEnabled : false
            }

            Button
            {
                text: qsTr("Cancel")
                onClicked: { root.close(); }
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

    function indexOf(item)
    {
        for(var i = 0; i < listPages.length; i++)
        {
            if(listPages[i] === item)
                return i;
        }

        return -1;
    }

    NumberAnimation
    {
        id: numberAnimation
        target: content
        properties: "x"
        to: currentIndex * -(contentContainer.width)
        easing.type: Easing.OutQuad
    }

    Component.onCompleted:
    {
        for(var i = 0; i < listPages.length; i++)
        {
            listPages[i].parent = content;
            listPages[i].x = Qt.binding(function() {
                return (indexOf(this) * ((contentContainer.width) + pageSelector.implicitWidth))  });
            listPages[i].width = Qt.binding(function() { return (contentContainer.width); });
            listPages[i].height = Qt.binding(function() { return content.height; });
        }
    }

    onAccepted:
    {
        close();
    }
}
