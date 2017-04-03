import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import ".."
import "../Constants.js" as Constants
import "../Utils.js" as Utils

Window
{
    id: root

    property string configuration

    // Hack: the window is shared between visualisations so
    // we need some way of knowing which one we're currently
    // changing
    property int visualisationIndex

    title: qsTr("Pick Gradient")
    modality: Qt.ApplicationModal
    flags: Qt.Window|Qt.Dialog

    width: 250
    minimumWidth: 250
    maximumWidth: 500

    height: layout.height + 2 * Constants.margin
    minimumHeight: height
    maximumHeight: height

    ColumnLayout
    {
        id: layout

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: implicitHeight
        anchors.margins: Constants.margin

        Repeater
        {
            model:
            [
                // Sunrise
                "{
                    \"0\":    \"Red\",
                    \"0.66\": \"Yellow\",
                    \"1\":    \"White\"
                }",
                // Heated Metal
                "{
                    \"0\":   \"Black\",
                    \"0.4\": \"Purple\",
                    \"0.6\": \"Red\",
                    \"0.8\": \"Yellow\",
                    \"1\":   \"White\"
                }",
                // Visible Spectrum
                "{
                    \"0.00\": \"#F0F\",
                    \"0.25\": \"#00F\",
                    \"0.50\": \"#0F0\",
                    \"0.75\": \"#FF0\",
                    \"1.00\": \"#F00\"
                }",
                // Stepped Colours
                "{
                    \"0\":    \"Navy\",
                    \"0.25\": \"Navy\",
                    \"0.26\": \"Green\",
                    \"0.5\":  \"Green\",
                    \"0.51\": \"Yellow\",
                    \"0.75\": \"Yellow\",
                    \"0.76\": \"Red\",
                    \"1\":    \"Red\"
                }",
                // Incandescent
                "{
                    \"0\":    \"Black\",
                    \"0.33\": \"DarkRed\",
                    \"0.66\": \"Yellow\",
                    \"1\":    \"White\"
                }",
                // Black Aqua White
                "{
                    \"0\":   \"Black\",
                    \"0.5\": \"Aqua\",
                    \"1\":   \"White\"
                }",
                // Blue Red
                "{
                    \"0.0\": \"blue\",
                    \"1\":   \"red\"
                }",
                // Deep Sea
                "{
                    \"0.0\":  \"#000000\",
                    \"0.6\":  \"#183567\",
                    \"0.75\": \"#2E649E\",
                    \"0.9\":  \"#17ADCB\",
                    \"1.0\":  \"#00FAFA\"
                }"
            ]

            GradientKey
            {
                Layout.fillWidth: true
                configuration: modelData
                showLabels: false

                textColor: "black"
                hoverColor: "grey"

                onClicked:
                {
                    root.configuration = configuration;
                }
            }
        }

        Button
        {
            Layout.alignment: Qt.AlignRight
            text: qsTr("Close")
            onClicked: root.close();
        }
    }
}
