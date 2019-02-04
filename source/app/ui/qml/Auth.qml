import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import QtQuick.Controls.Styles 1.4
import com.kajeka 1.0

import "../../../shared/ui/qml/Constants.js" as Constants

import "Controls"

Rectangle
{
    id: root

    property string email: emailField.text
    property string password: passwordField.text
    property bool rememberMe: rememberMeCheckBox.checked
    property string message
    property bool busy

    property bool _acceptableInput: emailField.acceptableInput && passwordField.acceptableInput

    Image
    {
        anchors.right: root.right
        anchors.top: root.top

        source: "auth-background.png"
    }

    color: "#575757"

    Preferences
    {
        id: preferences
        section: "auth"
        property alias emailAddress: emailField.text
        property alias rememberMe: rememberMeCheckBox.checked
    }

    ColumnLayout
    {
        visible: !root.busy
        enabled: !root.busy

        anchors.centerIn: parent

        spacing: Constants.spacing

        TextField
        {
            Layout.alignment: Qt.AlignCenter
            Layout.minimumWidth: 200

            id: emailField

            placeholderText: qsTr("Email Address")
            validator: RegExpValidator
            {
                // Check it's a valid email address
                regExp: /\w+([-+.']\w+)*@\w+([-.]\w+)*\.\w+([-.]\w+)*/
            }

            onAccepted:
            {
                passwordField.forceActiveFocus();
            }
        }

        TextField
        {
            Layout.alignment: Qt.AlignCenter
            Layout.minimumWidth: 200

            id: passwordField

            placeholderText: qsTr("Password")
            validator: RegExpValidator
            {
                regExp: /..*/ // At least 1 character
            }
            echoMode: TextInput.Password

            onActiveFocusChanged:
            {
                if(activeFocus)
                    selectAll();
            }
        }

        RowLayout
        {
            Layout.alignment: Qt.AlignCenter

            CheckBox
            {
                id: rememberMeCheckBox
                style: CheckBoxStyle
                {
                    label: Text
                    {
                        text: qsTr("Remember Me")
                        color: "white"
                    }
                }
            }

            Button
            {
                text: qsTr("Sign In")
                enabled: root._acceptableInput

                onClicked:
                {
                    if(root._acceptableInput)
                        root.signIn(emailField.text, passwordField.text);
                }
            }
        }

        Text
        {
            id: messageText

            Layout.alignment: Qt.AlignCenter
            Layout.topMargin: 16
            Layout.minimumHeight: 64
            Layout.maximumWidth: 512

            text:
            {
                if(!root.busy && root.message.length > 0)
                    return root.message;

                if(emailField.text.length === 0)
                {
                    return qsTr("Don't have an account? " +
                        "<a href=\"https://kajeka.com/register\">Register Now.</a>");
                }

                return "";
            }

            onTextChanged:
            {
                if(text.length > 0)
                {
                    // Changed text implies a problem, so refocus
                    root.refocus();
                }
            }

            color: "white"
            linkColor: "skyblue"
            textFormat: Text.StyledText
            wrapMode: Text.WordWrap

            PointingCursorOnHoverLink {}

            onLinkActivated: Qt.openUrlExternally(link);
        }

        Keys.onPressed:
        {
            if(!root._acceptableInput)
                return;

            switch(event.key)
            {
            case Qt.Key_Enter:
            case Qt.Key_Return:
                event.accepted = true;
                root.signIn(emailField.text, passwordField.text);
                break;

            default:
                event.accepted = false;
            }
        }
    }

    ColumnLayout
    {
        anchors.centerIn: parent
        visible: root.busy
        spacing: 32

        BusyIndicator
        {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 128
            Layout.preferredHeight: 128
        }

        Text
        {
            Layout.alignment: Qt.AlignHCenter

            text: qsTr("Signing In…")
            color: "white"
            font.pointSize: 22
        }
    }

    signal signIn(var email, var password)

    function refocus()
    {
        if(!visible)
            return;

        if(emailField.text.length === 0)
            emailField.forceActiveFocus();
        else
            passwordField.forceActiveFocus();
    }

    onVisibleChanged:
    {
        if(visible)
        {
            passwordField.text = "";
            refocus();
        }
        else
        {
            // If not visible, remove focus from any text fields
            root.forceActiveFocus();
        }
    }
}
