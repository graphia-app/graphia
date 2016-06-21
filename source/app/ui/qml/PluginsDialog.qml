import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1
import "Constants.js" as Constants

Window
{
    id: pluginsWindow

    property var pluginDetails

    // These data members are undocumented, so this could break in future
    property string pluginDescription: pluginNamesList.contentItem.currentItem ?
        pluginNamesList.contentItem.currentItem.rowItem.itemModel.description : ""
    property string pluginImageSource: pluginNamesList.contentItem.currentItem ?
        pluginNamesList.contentItem.currentItem.rowItem.itemModel.imageSource : ""

    onVisibleChanged:
    {
        // Force selection of first row
        if(visible && pluginNamesList.rowCount > 0)
        {
            pluginNamesList.currentRow = 0;
            pluginNamesList.selection.select(0);
        }
    }

    title: qsTr("About Plugins")
    flags: Qt.Window|Qt.Dialog
    width: 550
    height: 300
    minimumWidth: 550
    minimumHeight: 300

    GridLayout
    {
        id: grid

        rows: 2
        columns: 2

        anchors.fill: parent
        anchors.margins: Constants.margin

        TableView
        {
            id: pluginNamesList

            Layout.rowSpan: pluginImageSource.length > 0 ? 3 : 2
            Layout.fillHeight: true

            TableViewColumn { role: "name" }
            headerDelegate: Item {}

            model: pluginDetails
        }

        Image
        {
            Layout.margins: Constants.margin
            visible: pluginImageSource.length > 0
            source: pluginImageSource
            sourceSize.width: 96
            sourceSize.height: 96
        }

        Text
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.margins: Constants.margin
            wrapMode: Text.WordWrap

            text: pluginDescription
        }

        Button
        {
            text: qsTr("Close")
            anchors.right: grid.right
            onClicked: pluginsWindow.close()
        }
    }
}

