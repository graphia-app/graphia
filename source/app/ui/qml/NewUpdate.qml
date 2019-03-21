import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import com.kajeka 1.0

import "../../../shared/ui/qml/Constants.js" as Constants

Item
{
    id: root

    width: row.width
    height: row.height

    Action
    {
        id: restartAction
        text: qsTr("Restart")
        onTriggered:
        {
            root.visible = false;
            root.restartClicked();
        }
    }

    Action
    {
        id: closeAction
        text: qsTr("Not Now")
        onTriggered:
        {
            root.visible = false;
        }
    }

    RowLayout
    {
        id: row

        Item
        {
            Layout.preferredWidth: icon.width + Math.abs(icon.endX - icon.startX)

            NamedIcon
            {
                id: icon
                anchors.verticalCenter: parent.verticalCenter

                property double startX: 16.0
                property double endX: 0.0

                iconName: "software-update-available"

                SequentialAnimation on x
                {
                    loops: Animation.Infinite

                    NumberAnimation
                    {
                        from: icon.startX; to: icon.endX
                        easing.type: Easing.OutExpo; duration: 500
                    }

                    NumberAnimation
                    {
                        from: icon.endX; to: icon.startX
                        easing.amplitude: 2.0
                        easing.type: Easing.OutBounce; duration: 500
                    }

                    PauseAnimation { duration: 500 }
                }
            }
        }

        Text { text: qsTr("An update is available!") }
        Button { action: restartAction }
        Button { action: closeAction }
    }

    signal restartClicked()
}

