import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

import "Constants.js" as Constants

Item
{
    anchors.fill: parent
    anchors.margins: Constants.margin

    ColumnLayout
    {
        anchors.fill: parent

        Text
        {
            Layout.fillWidth: true
            width: parent.width
            elide: Text.ElideMiddle

            text: plugin.model.selectedNodeNames
        }

        Text
        {
            text: "Mean Node Degree: " + plugin.model.selectedNodeMeanDegree
        }
    }
}
