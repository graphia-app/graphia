import QtQuick 2.7

// If this item is inserted as the child of something with a hoveredLink property,
// it will change the cursor to a pointing finger, whenever said property is non-empty
MouseArea
{
    id: root
    anchors.fill: parent

    cursorShape:
    {
        if(parent.hoveredLink === undefined)
        {
            console.log("Parent of " + root + " has no 'hoveredLink' property");
            return Qt.ArrowCursor;
        }

        return parent.hoveredLink.length > 0 ?
            Qt.PointingHandCursor : Qt.ArrowCursor;
    }

    // Ignore and pass through any events
    propagateComposedEvents: true
    onPressed: { mouse.accepted = false; }
}
