import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

import SortFilterProxyModel 0.1

import "Constants.js" as Constants

Dialog
{
    id: fileTypeChooserDialog

    title: qsTr("File Type Ambiguous")
    width: 500

    property var model

    property string fileUrl
    property var fileTypes: []
    property string fileType
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
            text: qsTr("The file you are opening may be interpreted as two or more possible formats. " +
                       "Please select how you wish to proceed below.")
            Layout.fillWidth: true
            Layout.columnSpan: 2
            wrapMode: Text.WordWrap
        }

        Text
        {
            text: qsTr("Open File As:")
            Layout.alignment: Qt.AlignRight
        }

        ComboBox
        {
            id: fileTypeChoice
            Layout.alignment: Qt.AlignLeft
            implicitWidth: 200

            model: SortFilterProxyModel
            {
                sourceModel: model
                filterExpression:
                {
                    var i = fileTypeChooserDialog.fileTypes.indexOf(model.name);

                    if(i > -1)
                    {
                        fileTypeChooserDialog.mapping[i] = index;
                        return true;
                    }

                    return false;
                }
            }

            textRole: "individualDescription"
        }
    }

    standardButtons: StandardButton.Ok | StandardButton.Cancel

    onAccepted:
    {
        var i = mapping[fileTypeChoice.currentIndex];
        fileType = model.nameAtIndex(i);
    }
}
