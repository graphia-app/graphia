import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

Column
{
    property var document
    property color textColor

    Repeater
    {
        model: document.layoutSettings

        RowLayout
        {
            anchors.left: parent.left

            Label
            {
                id: label
                color: textColor

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
