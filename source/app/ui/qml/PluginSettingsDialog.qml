import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.1

import "Constants.js" as Constants

Dialog
{
    id: pluginSettingsDialog

    title: qsTr("Plugin Settings")
    width: 500

    property string qmlPath

    onQmlPathChanged:
    {
        if(qmlPath.length > 0)
        {
            // Destroy anything already there
            while(pluginSettingsDialog.children.length > 0)
                pluginSettingsDialog.children[0].destroy();

            var pluginSettingsComponent = Qt.createComponent(qmlPath);

            if(pluginSettingsComponent.status !== Component.Ready)
            {
                console.log(pluginSettingsComponent.errorString());
                return;
            }

            var pluginSettingsObject = pluginSettingsComponent.createObject(contentItem);

            if(pluginSettingsObject === null)
            {
                console.log(settingsQmlPath + ": failed to create instance");
                return;
            }
        }
    }

    property string fileUrl
    property string fileType
    property string pluginName
    property var settings
    property bool inNewTab

    onVisibleChanged:
    {
        if(visible)
            settings = {};
    }

    standardButtons: StandardButton.Ok | StandardButton.Cancel
}
