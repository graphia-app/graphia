import QtQuick 2.4
import QtQuick.Controls 1.3

Item {
    property alias paramList: paramList
    visible: false;
    height: 500
    Rectangle {
        color: "red"
        height: 500

        Component
        {

            id: paramSlider
            Item {
                height: 40
                Column {

                    Text {
                        text: name
                    }
                    Row {
                        Slider {
                            property bool loaded: false;
                            minimumValue: min
                            value: val
                            maximumValue: max
                            onValueChanged: {
                                if(loaded) val = this.value
                                    else
                                loaded = true;
                            }
                        }
                        Text {
                            text: val.toPrecision(3);
                        }
                    }
                }
            }
        }

        ListView {
            anchors.fill: parent
            id: paramList
            delegate: paramSlider
        }
    }
}

