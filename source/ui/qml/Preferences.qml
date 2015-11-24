import QtQuick 2.4

PreferencesForm {
   sldrTension.onClicked: messageDialog.show(qsTr("Button pressed"));
}

