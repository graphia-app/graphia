pragma Singleton
import QtQuick 2.4

QtObject
{
    property var _defaultTextObj:
    {
        return  Qt.createQmlObject('import QtQuick 2.0; Text { text: "unused" }', this);
    }

    property real p: _defaultTextObj.font.pointSize;
    property real h2: _defaultTextObj.font.pointSize * 1.5;
    property real h1: _defaultTextObj.font.pointSize * 2.0;
}
