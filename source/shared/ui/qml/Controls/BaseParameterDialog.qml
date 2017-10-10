import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import "../../shared/ui/qml/Constants.js" as Constants

Window
{
    property var parameters: ({})
    property string fileUrl: ""
    property string fileType: ""
    property string pluginName: ""
    property bool inNewTab: false

    title: pluginName + qsTr(" Plugin Parameters")
    signal accepted();
}
