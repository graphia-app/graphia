import QtQuick 2.7

// If this is made the child of another control then the activated signal is
// emitted when the user clicks on the control in clockwise order starting at
// the top left, twice in succession

MouseArea
{
    anchors.fill: parent

    property int clickState: -1

    onClicked:
    {
        var left = mouseX < (width * 0.5);
        var top = mouseY < (height * 0.5);

        var value = -1;
        if(left && top)
            value = 0;
        else if(!left && top)
            value = 1;
        else if(!left && !top)
            value = 2;
        else if(left && !top)
            value = 3;

        if((clickState + 1) == value || (clickState - 3) == value)
            clickState++;
        else
            clickState = -1;

        if(clickState == 7)
            activated();
    }

    signal activated()
}
