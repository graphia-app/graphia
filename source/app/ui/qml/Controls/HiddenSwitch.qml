import QtQuick 2.7

// If this is made the child of another control then the activated signal is
// emitted when the user clicks on the control in clockwise order starting at
// the top left, twice in succession

MouseArea
{
    anchors.fill: parent

    property int _clickState: 0

    onClicked:
    {
        _clickState++;

        if(_clickState == 10)
        {
            _clickState = 0;
            activated();
        }
    }

    signal activated()
}
