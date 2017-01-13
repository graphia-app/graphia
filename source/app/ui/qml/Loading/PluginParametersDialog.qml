import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

import "../Constants.js" as Constants
import "../Utils.js" as Utils

Window
{
    id: root

    title: pluginName + qsTr(" Plugin Parameters")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog
    width: minimumWidth
    height: minimumHeight
    minimumWidth: layout.implicitWidth + (Constants.margin * 2)
    minimumHeight: layout.implicitHeight + (Constants.margin * 2)

    property string qmlPath

    onQmlPathChanged:
    {
        if(qmlPath.length > 0)
        {
            var component = Qt.createComponent(qmlPath);

            if(component.status !== Component.Ready)
            {
                console.log(component.errorString());
                return;
            }

            var contentObject = component.createObject(contentItem);

            if(contentObject === null)
            {
                console.log(parametersQmlPath + ": failed to create instance");
                return;
            }
        }
    }

    property string fileUrl
    property string fileType
    property string pluginName
    property var parameters
    property bool inNewTab

    // The component that's loaded from qmlPath
    property var content: contentItem.children && contentItem.children.length > 0 ?
                          contentItem.children[0] : null

    ColumnLayout
    {
        id: layout

        anchors.fill: parent
        anchors.margins: Constants.margin
        spacing: Constants.spacing

        // The loaded component gets parented to this
        Rectangle
        {
            id: contentItem
            Layout.minimumWidth: content.width
            Layout.minimumHeight: content.height
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        RowLayout
        {
            Rectangle { Layout.fillWidth: true }

            Button
            {
                text: qsTr("OK")
                onClicked: { root.accept(); }
            }

            Button
            {
                text: qsTr("Cancel")
                onClicked: { root.reject(); }
            }
        }

        Keys.onPressed:
        {
            event.accepted = true;
            switch(event.key)
            {
            case Qt.Key_Escape:
            case Qt.Key_Back:
                reject();
                break;

            case Qt.Key_Enter:
            case Qt.Key_Return:
                accept();
                break;

            default:
                event.accepted = false;
            }
        }
    }

    function open()
    {
        visible = true;

        if(content !== null && typeof content.initialise === 'function')
            content.initialise();
    }

    function accept()
    {
        accepted();
        root.close();
    }

    function reject()
    {
        rejected();
        root.close();
    }

    signal accepted()
    signal rejected()
}
