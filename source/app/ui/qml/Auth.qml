import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3
import com.kajeka 1.0

import "Constants.js" as Constants

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

        source: "bubbles.png"
    }

    color: "white"

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
                text: qsTr("Remember Me")
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

            text: !root.busy ? root.message : ""

            onTextChanged:
            {
                // Changed text implies a problem, so refocus
                root.refocus();
            }

            linkColor: "grey"
            textFormat: Text.StyledText
            wrapMode: Text.WordWrap

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

            text: qsTr("Signing In...")
            font.pointSize: 22
        }
    }

    signal signIn(var email, var password)

    function refocus()
    {
        if(emailField.text.length === 0)
            emailField.forceActiveFocus();
        else
            passwordField.forceActiveFocus();
    }

    onVisibleChanged:
    {
        passwordField.text = "";
        refocus();
    }

    Component.onCompleted:
    {
        refocus();
    }
}
