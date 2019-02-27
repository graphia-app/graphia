import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.3

import com.kajeka 1.0

import ".."

// This is basically a substitute for ToolButton,
// that looks consistent across platforms
Button
{
    id: root

    property string iconName

    implicitHeight: 32

    style: ButtonStyle
    {
        background: Rectangle
        {
            visible: control.hovered
            border.width: 1
            border.color: "#ababab"
            radius: 2
            gradient: Gradient
            {
                GradientStop { position: 0; color: control.pressed ? "#dcdcdc" : "#fefefe" }
                GradientStop { position: 1; color: control.pressed ? "#dcdcdc" : "#f8f8f8" }
            }
        }

        label: RowLayout
        {
            spacing: 4
            anchors.fill: parent

            property string _iconName: control.action !== null ?
                control.action.iconName : root.iconName
            property string _text: control.action !== null ?
                control.action.text : root.text

            NamedIcon
            {
                id: icon

                Layout.alignment: Qt.AlignVCenter
                visible: valid
                Layout.preferredWidth: height
                Layout.preferredHeight: root.height - (padding.top + padding.bottom)
                iconName: _iconName
            }

            Text
            {
                Layout.alignment: Qt.AlignVCenter
                visible: !icon.valid && _text.length > 0
                text: _text
            }

            Item
            {
                // Empty placeholder that's shown if there is no
                // valid icon or text available
                visible: !icon.valid && _text.length === 0
                Layout.preferredWidth: icon.width
                Layout.preferredHeight: icon.height
            }
        }
    }
}
