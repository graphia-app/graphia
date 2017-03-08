import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Layouts 1.3

Item
{
    width: row.width
    height: row.height

    property color textColor

    RowLayout
    {
        id: row

        Label
        {
            text: settingName
            color: textColor
        }

        Slider
        {
            id: slider

            minimumValue: settingMinimumValue
            maximumValue: settingMaximumValue

            onValueChanged:
            {
                if(pressed)
                    settingValue = value;
            }
        }

        Label
        {
            text: settingValue.toPrecision(3)
            color: textColor
        }
    }

    Component.onCompleted:
    {
        slider.value = settingValue;
    }
}
