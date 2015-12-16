import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1

Item
{
    width: row.width
    height: row.height

    RowLayout
    {
        id: row

        Text
        {
            text: settingName
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

        Text
        {
            text: settingValue.toPrecision(3)
        }
    }

    Component.onCompleted:
    {
        slider.value = settingValue;
    }
}
