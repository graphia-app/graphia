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
            anchors.right: parent.right

            Label
            {
                id: label
                color: textColor
            }

            Slider
            {
                id: slider

                onValueChanged:
                {
                    if(pressed)
                        document.setLayoutSettingValue(modelData, value);
                }
            }

            Label
            {
                text: slider.value.toPrecision(3)
                color: textColor
            }

            Component.onCompleted:
            {
                var setting = document.layoutSetting(modelData);
                label.text = setting.displayName;

                slider.value = setting.value;
                slider.minimumValue = setting.minimumValue;
                slider.maximumValue = setting.maximumValue;
            }
        }
    }
}
