import QtQuick 2.5
import QtQuick.Controls 1.4
import QtQml 2.2

Button
{
    property string defaultText: ""
    property string selectedValue: ""

    property alias model: instantiator.model

    id: button
    text: selectedValue != "" ? selectedValue : defaultText
    menu: Menu
    {
        id: menu

        Instantiator
        {
            id: instantiator
            delegate: MenuItem
            {
                text: index >= 0 ? instantiator.model[index] : ""

                onTriggered: { selectedValue = text }
            }

            onObjectAdded: menu.insertItem(index, object)
            onObjectRemoved: menu.removeItem(object)
        }
    }
}
