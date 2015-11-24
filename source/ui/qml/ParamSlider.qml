import QtQuick 2.0
import QtQuick.Controls 1.3

Component
{
    id: paramSlider

    Text {
        text: name
    }

    Slider {
        minimumValue: min
        value: value
        maximumValue: max
    }
}


