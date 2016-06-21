import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

import SortFilterProxyModel 0.1

import "Constants.js" as Constants

Dialog
{
    id: pluginChooserDialog

    title: qsTr("Multiple Plugins Applicable")
    width: 500

    property var model

    property string fileUrl
    property string fileType
    property var pluginNames: []
    property string pluginName
    property bool inNewTab

    // Mapping from the ComboBox currentIndex to the model
    property var mapping: []

    onVisibleChanged:
    {
        if(!visible)
            mapping.length = 0;
    }

    GridLayout
    {
        columns: 2

        width: parent.width
        anchors.margins: Constants.margin

        Text
        {
            text: qsTr("The file you are opening may be loaded by two or more plugins. " +
                       "Please select how you wish to proceed below.")
            Layout.fillWidth: true
            Layout.columnSpan: 2
            wrapMode: Text.WordWrap
        }

        Text
        {
            text: qsTr("Open File With Plugin:")
            Layout.alignment: Qt.AlignRight
        }

        ComboBox
        {
            id: pluginChoice
            Layout.alignment: Qt.AlignLeft
            implicitWidth: 200

            model: SortFilterProxyModel
            {
                sourceModel: model
                filterExpression:
                {
                    var i = pluginChooserDialog.pluginNames.indexOf(model.name);

                    if(i > -1)
                    {
                        pluginChooserDialog.mapping[i] = index;
                        return true;
                    }

                    return false;
                }
            }

            textRole: "name"
        }
    }

    standardButtons: StandardButton.Ok | StandardButton.Cancel

    onAccepted:
    {
        var i = mapping[pluginChoice.currentIndex];
        pluginName = model.nameAtIndex(i);
    }
}
