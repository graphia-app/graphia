import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4

Item
{
    property alias placeholderText: placeholder.text
    property alias text: textArea.text

    property bool __shouldShowPlaceholderText:
        !textArea.text.length && !textArea.activeFocus

    // This only exists to get at the default TextFieldStyle.placeholderTextColor
    // ...maybe there is a better way?
    TextField
    {
        visible: false
        style: TextFieldStyle
        {
            Component.onCompleted: placeholder.textColor = placeholderTextColor
        }
    }

    TextArea
    {
        id: placeholder
        anchors.fill: parent
        visible: __shouldShowPlaceholderText
        activeFocusOnTab: false
    }

    TextArea
    {
        id: textArea
        anchors.fill: parent
        backgroundVisible: !__shouldShowPlaceholderText
    }
}
