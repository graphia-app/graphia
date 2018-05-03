import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

import "../../../shared/ui/qml/Constants.js" as Constants

Rectangle
{
    id: root

    property var document

    readonly property bool showing: _visible
    property bool _visible: false

    width: row.width
    height: row.height

    border.color: document.contrastingColor
    border.width: 1
    radius: 4
    color: "white"

    Action
    {
        id: closeAction
        text: qsTr("Close")
        iconName: "emblem-unreadable"

        onTriggered:
        {
            _visible = false;
            hidden();
        }
    }

    RowLayout
    {
        id: row

        // The ColumnLayout in a RowLayout is just a hack to get some padding
        ColumnLayout
        {
            Layout.topMargin: Constants.padding
            Layout.bottomMargin: Constants.padding - root.parent.parent.anchors.bottomMargin
            Layout.leftMargin: Constants.padding + Constants.margin
            Layout.rightMargin: Constants.padding

            RowLayout
            {
                Label { font.bold: true; text: document.layoutDisplayName }
                Item { Layout.fillWidth: true }
                ToolButton { action: closeAction }
            }

            Repeater
            {
                model: document.layoutSettings

                RowLayout
                {
                    Label
                    {
                        id: label

                        MouseArea
                        {
                            anchors.fill: parent

                            onDoubleClicked:
                            {
                                document.resetLayoutSettingValue(modelData);
                                var setting = document.layoutSetting(modelData);
                                slider.value = setting.normalisedValue;
                            }
                        }
                    }

                    Item { Layout.fillWidth: true }

                    Slider
                    {
                        id: slider
                        minimumValue: 0.0
                        maximumValue: 1.0

                        onValueChanged:
                        {
                            if(pressed)
                                document.setLayoutSettingNormalisedValue(modelData, value);
                        }
                    }

                    Component.onCompleted:
                    {
                        var setting = document.layoutSetting(modelData);
                        label.text = setting.displayName;
                        slider.value = setting.normalisedValue;
                    }
                }
            }
        }
    }

    function show()
    {
        root._visible = true;
        shown();
    }

    function hide()
    {
        closeAction.trigger();
    }

    function toggle()
    {
        if(root._visible)
            hide();
        else
            show();
    }

    signal shown();
    signal hidden();
}
