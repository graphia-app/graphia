import QtQuick 2.7
import QtQuick.Controls 1.5
import QtQuick.Controls.Styles 1.4

Item
{
    id: root
    width: image.width
    height: image.height

    property string type
    property string text

    Image
    {
        id: image

        source:
        {
            switch(root.type)
            {
            case "error": return "error.png";
            default:
            case "warning": return "warning.png";
            }
        }

        ToolTip { text: root.text }
    }

}
