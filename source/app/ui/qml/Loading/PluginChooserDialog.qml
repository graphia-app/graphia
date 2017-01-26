import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

import SortFilterProxyModel 0.2

import "../Constants.js" as Constants

Dialog
{
    id: pluginChooserDialog

    title: qsTr("Multiple Plugins Applicable")
    width: 500

    property var application
    property var model

    property string fileUrl
    property string fileType
    property var pluginNames: []
    property string pluginName
    property bool inNewTab

    GridLayout
    {
        columns: 2

        width: parent.width
        anchors.margins: Constants.margin

        Text
        {
            text: application.baseFileNameForUrl(fileUrl) +
                  qsTr(" may be loaded by two or more plugins. " +
                       "Please select how you wish to proceed below.")
            Layout.fillWidth: true
            Layout.columnSpan: 2
            wrapMode: Text.WordWrap
        }

        Text
        {
            text: qsTr("Open With Plugin:")
            Layout.alignment: Qt.AlignRight
        }

        ComboBox
        {
            id: pluginChoice
            Layout.alignment: Qt.AlignLeft
            implicitWidth: 200

            model: SortFilterProxyModel
            {
                id: proxyModel

                sourceModel: pluginChooserDialog.model
                filterRoleName: "name"
                filterPattern:
                {
                    var s = "";

                    for(var i = 0; i < pluginChooserDialog.pluginNames.length; i++)
                    {
                        if(i != 0) s += "|";
                        s += pluginChooserDialog.pluginNames[i];
                    }

                    return s;
                }

                onFilterPatternChanged:
                {
                    // Reset to first item
                    pluginChoice.currentIndex = -1;
                    pluginChoice.currentIndex = 0;
                }
            }

            property var selectedPlugin:
            {
                var index = proxyModel.index(currentIndex, 0);
                var mappedIndex = proxyModel.mapToSource(index);

                return pluginChooserDialog.model.nameAtIndex(mappedIndex);
            }

            textRole: "name"
        }
    }

    standardButtons: StandardButton.Ok | StandardButton.Cancel

    onAccepted:
    {
        pluginName = pluginChoice.selectedPlugin;
    }
}
